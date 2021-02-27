#include <cassert>
#include <chrono>
#include <iostream>
#include <set>

#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#include "lua.h"
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
    auto L = luaL_newstate();
    luaL_openlibs(L);
    assert(luaL_dofile(L, "init.lua") == 0);
    e.bindings = get_bindings(L);
    // end load functions -->

    if (argc > 1) {
        e.buf = file_load(argv[1]);
        e.filename = argv[1];
    } else {
        e.buf = {{std::make_shared<buffer_line>()}, {0, 0}};
        e.filename = "/tmp/alster.tmp";
    }
    assert(tty_enable_raw_mode() == 0);
    
    lua_getglobal(L, "buffer");
    lua_pushstring(L, "lines");
    buffer_lines** mem = (buffer_lines**) lua_newuserdata(L, sizeof(buffer_lines*));
    *mem = &(e.buf.lines);
    lua_newtable(L);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_lines_index);
    lua_settable(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, lua_lines_newindex);
    lua_settable(L, -3);
    lua_pushstring(L, "__len");
    lua_pushcfunction(L, lua_lines_len);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    lua_settable(L, -3);
    lua_pop(L, 1); // pop buffer

    // create line api
    lua_newtable(L);
    lua_pushstring(L, "sub");
    lua_pushcfunction(L, lua_line_sub);
    lua_settable(L, -3);
    lua_pushstring(L, "create");
    lua_pushcfunction(L, lua_line_create);
    lua_settable(L, -3);
    lua_setglobal(L, "line");

    // create lines api
    lua_newtable(L);
    lua_pushstring(L, "insert");
    lua_pushcfunction(L, lua_lines_insert);
    lua_settable(L, -3);
    lua_setglobal(L, "lines");

    while (true) {
        win = editor_draw(e, win);
        e.status[0] = '\0';
        t.report("render:    ");
        e.cmd.push_back(utf8_getchar());
        t.start();
        e = editor_handle_command(std::move(e));
        if (e.lua_function) {
            // write buffer
            lua_getglobal(L, "buffer");
            // push x
            lua_pushstring(L, "x");
            lua_pushinteger(L, e.buf.pos.x + 1);
            lua_settable(L, -3);
            // push mode
            lua_pushstring(L, "mode");
            lua_pushinteger(L, e.mode);
            lua_settable(L, -3);
            // push y
            lua_pushstring(L, "y");
            lua_pushinteger(L, e.buf.pos.y + 1);
            lua_settable(L, -3);
            lua_pop(L, 1); // pop buffer
            // done with buffer
            lua_getglobal(L, "config");
            lua_getfield(L, -1, "bindings");
            lua_getfield(L, -1, utf8_encode(*e.lua_function).c_str());
            assert(lua_isfunction(L, -1));
            lua_call(L, 0, 0);
            lua_pop(L, 1); // pop bindings
            lua_pop(L, 1); // pop config
            // read buffer
            lua_getglobal(L, "buffer");
            lua_getfield(L, -1, "mode");
            e.mode = (int) lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, -1, "x");
            e.buf.pos.x = lua_tointeger(L, -1) - 1;
            lua_pop(L, 1);
            lua_getfield(L, -1, "y");
            e.buf.pos.y = lua_tointeger(L, -1) - 1;
            lua_pop(L, 1);
            lua_getfield(L, -1, "exiting");
            e.exiting = lua_toboolean(L, -1);
            lua_pop(L, 1);
            lua_pop(L, 1); // pop buffer
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

