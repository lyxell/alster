#include <cassert>
#include <chrono>
#include <iostream>
#include <set>

extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
}

#include "buffer.h"
#include "editor.h"
#include "tty.h"
#include "utf8.h"
#include "window.h"

struct timer {
    std::chrono::time_point<std::chrono::high_resolution_clock> s;
    void start() {
        s = std::chrono::high_resolution_clock::now();
    }
    void report(const char* str) {
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::microseconds;
        /*
        auto e = high_resolution_clock::now();
        auto diff = duration_cast<microseconds>(e - s).count();
        fprintf(stderr, "%s %ld μs\n", str, diff);
        */
    }
};

template <typename T>
std::set<std::u32string> get_bindings(T lua_state, const char* mode) {
    std::set<std::u32string> bindings;
    // push config
    lua_getglobal(lua_state, "bindings");
    assert(lua_istable(lua_state, -1));
    // push bindings
    lua_getfield(lua_state, -1, mode);
    // traverse bindings
    lua_pushnil(lua_state);
    while (lua_next(lua_state, -2) != 0) {
        // pops 'value', keeps 'key' for next iteration
        std::u32string str;
        for (auto c : std::string(lua_tolstring(lua_state, -2, NULL)))
            str.push_back(c);
        bindings.insert(str);
        lua_pop(lua_state, 1);
    }
    // pop bindings
    assert(lua_istable(lua_state, -1));
    lua_pop(lua_state, 1);
    // pop config
    assert(lua_istable(lua_state, -1));
    lua_pop(lua_state, 1);
    return bindings;
}

static void push_state(lua_State *L, const editor& e, int BUFFER_REFERENCE) {
    lua_newtable(L);
    // buffer
    lua_pushstring(L, "buffer");
    lua_rawgeti(L, LUA_REGISTRYINDEX, BUFFER_REFERENCE);
    lua_settable(L, -3);
    // position
    lua_pushstring(L, "x");
    lua_pushinteger(L, e.pos.x);
    lua_settable(L, -3);
    lua_pushstring(L, "y");
    lua_pushinteger(L, e.pos.y);
    lua_settable(L, -3);
    // mode
    lua_pushstring(L, "mode");
    lua_pushinteger(L, e.mode);
    lua_settable(L, -3);
}

static void read_state(lua_State *L, editor& e, int BUFFER_REFERENCE) {
    assert(lua_istable(L, -1));
    {
        lua_getfield(L, -1, "buffer");
        if (!lua_isnil(L, -1)) {
            lua_getglobal(L, "flattenpiecetable");
            lua_pushvalue(L, -2);
            lua_call(L, 1, 1);
            // now there is a table of strings on the stack
            assert(lua_istable(L, -1));
            // traverse table of strings
            std::vector<buffer_line> ls = {};
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                ls.push_back(utf8_decode(std::string(lua_tolstring(L, -1, NULL))));
                lua_pop(L, 1);
            }
            e.lines = ls;
            lua_pop(L, 1); // pop table of strings
            lua_rawseti(L, LUA_REGISTRYINDEX, BUFFER_REFERENCE); // pops buffer
        } else {
            lua_pop(L, 1); // pops buffer
        }
    }
    {
        lua_getfield(L, -1, "x");
        if (!lua_isnil(L, -1)) {
            e.pos.x = lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
    }
    {
        lua_getfield(L, -1, "y");
        if (!lua_isnil(L, -1)) {
            e.pos.y = lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
    }
    {
        lua_getfield(L, -1, "exiting");
        if (!lua_isnil(L, -1)) {
            e.exiting = true;
        }
        lua_pop(L, 1);
    }
    {
        lua_getfield(L, -1, "mode");
        if (!lua_isnil(L, -1)) {
            e.mode = lua_tointeger(L, -1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

int main(int argc, char* argv[]) {
    editor e {};
    timer t {};
    window win {};

    e.filename = argv[1];

    auto L = luaL_newstate();
    luaL_openlibs(L);

    e.pos.x = 1;
    e.pos.y = 1;
    assert(tty_enable_raw_mode() == 0);

    // load file api
    assert(luaL_dofile(L, "lua/file.lua") == 0);
    lua_getfield(L, -1, "read");
    const int API_READFILE = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_getfield(L, -1, "write");
    const int API_WRITEFILE = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1); // pop file api

    // load piecetable libs FIXME
    assert(luaL_dofile(L, "lua/piecetable.lua") == 0);
    lua_getfield(L, -1, "topiecetable");
    lua_setglobal(L, "topiecetable");
    lua_getfield(L, -1, "flattenpiecetable");
    lua_setglobal(L, "flattenpiecetable");
    lua_pop(L, 1);

    // create buffer
    lua_getglobal(L, "topiecetable");
    if (e.filename) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, API_READFILE);
        lua_pushstring(L, e.filename);
        lua_call(L, 1, 1);
    } else {
        lua_newtable(L);
    }
    lua_call(L, 1, 1);
    const int BUFFER_REFERENCE = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // load config
    assert(luaL_dofile(L, "config.lua") == 0);

    push_state(L, e, BUFFER_REFERENCE);
    read_state(L, e, BUFFER_REFERENCE);

    while (true) {
        win = editor_draw(e, win);
        e.status[0] = '\0';
        t.report("render:    ");
        e.cmd.push_back(utf8_getchar());
        t.start();
        // handle command if match
        if (e.mode == MODE_NORMAL) {
            auto bindings = get_bindings(L, "normal");
            if (bindings.find(e.cmd) != bindings.end()) {
                lua_getglobal(L, "bindings");
                lua_getfield(L, -1, "normal");
                lua_getfield(L, -1, utf8_encode(e.cmd).c_str());
                assert(lua_isfunction(L, -1));
                push_state(L, e, BUFFER_REFERENCE);
                lua_call(L, 1, 1);
                read_state(L, e, BUFFER_REFERENCE);
                e.cmd = {};
                lua_pop(L, 1);
                lua_pop(L, 1);
            }
            // check if e.cmd is a prefix of some command, otherwise clear cmd
            if (bindings.upper_bound(e.cmd) == bindings.end() || bindings.upper_bound(e.cmd)->find(e.cmd) != 0) {
                e.cmd = {};
            }
        } else if (e.mode == MODE_INSERT) {
            auto bindings = get_bindings(L, "insert");
            if (bindings.find(e.cmd) != bindings.end()) {
                lua_getglobal(L, "bindings");
                lua_getfield(L, -1, "insert");
                lua_getfield(L, -1, utf8_encode(e.cmd).c_str());
                assert(lua_isfunction(L, -1));
                push_state(L, e, BUFFER_REFERENCE);
                lua_call(L, 1, 1);
                read_state(L, e, BUFFER_REFERENCE);
                e.cmd = {};
                lua_pop(L, 1);
                lua_pop(L, 1);
            }
            // check if e.cmd is a prefix of some command, otherwise insert cmd
            if (bindings.upper_bound(e.cmd) == bindings.end() || bindings.upper_bound(e.cmd)->find(e.cmd) != 0) {
                lua_getglobal(L, "events");
                lua_getfield(L, -1, "insert");
                push_state(L, e, BUFFER_REFERENCE);
                lua_pushstring(L, utf8_encode(e.cmd).c_str());
                lua_call(L, 2, 1);
                read_state(L, e, BUFFER_REFERENCE);
                e.cmd = {};
                lua_pop(L, 1);
            }
        }
        if (e.saving) {
            //file_save(e.filename, e.lines);
            //sprintf(e.status, "Saving %s", e.filename);
        }
        t.report("handle cmd:");
        t.start();
        if (e.exiting) break;
    }
    tty_disable_raw_mode();
    return 0;
}

