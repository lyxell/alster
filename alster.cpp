#include <cassert>
#include <chrono>
#include <iostream>
#include <set>

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#include "buffer.h"
#include "editor.h"
#include "file.h"
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

static int lines_len(lua_State *L) {
    // TODO: Why is this value 2? Shouldn't it be 1?
    assert(lua_gettop(L) == 2); /* number of arguments */
    assert(lua_isuserdata(L, -2));
    buffer_lines** lines = (buffer_lines**) lua_touserdata(L, -2);
    lua_pushnumber(L, (int) (*lines)->size());
    return 1;
}

static int lines_index(lua_State *L) {
    assert(lua_gettop(L) == 2); /* number of arguments */
    assert(lua_isnumber(L, -1));
    assert(lua_isuserdata(L, -2));
    long int idx = lua_tointeger(L, -1) - 1; // subtract 1, lua is 1-indexed
    buffer_lines** lines = (buffer_lines**) lua_touserdata(L, -2);
    auto& line = (*lines)->at(idx);
    lua_pushnumber(L, (int) line->size());
    return 1; /* number of results */
}

template <typename T>
std::set<std::u32string> get_bindings(T lua_state) {
    std::set<std::u32string> bindings;
    // push config
    lua_getglobal(lua_state, "config");
    assert(lua_istable(lua_state, -1));
    // push bindings
    lua_getfield(lua_state, -1, "bindings");
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

int main(int argc, char* argv[]) {
    editor e {};
    timer t {};
    window win {};

    // <-- load functions
    auto lua_state = luaL_newstate();
    luaL_openlibs(lua_state);
    assert(luaL_dofile(lua_state, "init.lua") == 0);
    e.bindings = get_bindings(lua_state);
    // end load functions -->

    if (argc > 1) {
        e.buf = file_load(argv[1]);
        e.filename = argv[1];
    } else {
        e.buf = {{std::make_shared<buffer_line>()}, {0, 0}};
        e.filename = "/tmp/alster.tmp";
    }
    assert(tty_enable_raw_mode() == 0);
    
    buffer_lines** mem = (buffer_lines**) lua_newuserdata(lua_state, sizeof(buffer_lines*));
    *mem = &(e.buf.lines);
    lua_newtable(lua_state);
    lua_pushstring(lua_state, "__index");
    lua_pushcfunction(lua_state, lines_index);
    lua_settable(lua_state, -3);
    lua_pushstring(lua_state, "__len");
    lua_pushcfunction(lua_state, lines_len);
    lua_settable(lua_state, -3);
    lua_setmetatable(lua_state, -2);
    lua_setglobal(lua_state, "lines");

    while (true) {
        win = editor_draw(e, win);
        e.status[0] = '\0';
        t.report("render:    ");
        e.cmd.push_back(utf8_getchar());
        t.start();
        e = editor_handle_command(std::move(e));
        if (e.lua_function) {
            // write buffer
            lua_getglobal(lua_state, "buffer");
            // push x
            lua_pushstring(lua_state, "x");
            lua_pushinteger(lua_state, e.buf.pos.x);
            lua_settable(lua_state, -3);
            // push mode
            lua_pushstring(lua_state, "mode");
            lua_pushinteger(lua_state, e.mode);
            lua_settable(lua_state, -3);
            // push y
            lua_pushstring(lua_state, "y");
            lua_pushinteger(lua_state, e.buf.pos.y);
            lua_settable(lua_state, -3);
            lua_pop(lua_state, 1); // pop buffer
            // done with buffer
            lua_getglobal(lua_state, "config");
            lua_getfield(lua_state, -1, "bindings");
            lua_getfield(lua_state, -1, utf8_encode(*e.lua_function).c_str());
            assert(lua_isfunction(lua_state, -1));
            lua_call(lua_state, 0, 0);
            lua_pop(lua_state, 1); // pop bindings
            lua_pop(lua_state, 1); // pop config
            // read buffer
            lua_getglobal(lua_state, "buffer");
            lua_getfield(lua_state, -1, "mode");
            e.mode = (int) lua_tointeger(lua_state, -1);
            lua_pop(lua_state, 1);
            lua_getfield(lua_state, -1, "x");
            e.buf.pos.x = lua_tointeger(lua_state, -1);
            lua_pop(lua_state, 1);
            lua_getfield(lua_state, -1, "y");
            e.buf.pos.y = lua_tointeger(lua_state, -1);
            lua_pop(lua_state, 1);
            lua_pop(lua_state, 1); // pop buffer
            e.lua_function = {};
        }
        if (e.saving) {
            file_save(e.filename, e.buf);
            sprintf(e.status, "Saving %s", e.filename);
        }
        t.report("handle cmd:");
        t.start();
        if (e.exiting) break;
    }
    tty_disable_raw_mode();
    return 0;
}

