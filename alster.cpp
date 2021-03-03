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

int main(int argc, char* argv[]) {
    editor e {};
    timer t {};
    window win {};

    // <-- load functions
    auto L = luaL_newstate();
    luaL_openlibs(L);
    // end load functions -->

    if (argc > 1) {
        e.buf = file_load(argv[1]);
        e.filename = argv[1];
    } else {
        e.buf = {{std::make_shared<buffer_line>()}, {0, 0}};
        e.filename = "/tmp/alster.tmp";
    }
    std::vector<buffer_lines> reference_holder = {};
    assert(tty_enable_raw_mode() == 0);
    
    // create buffer api
    lua_newtable(L);
    lua_pushstring(L, "sub");
    lua_pushcfunction(L, lua_buffer_sub);
    lua_settable(L, -3);
    lua_setglobal(L, "buffer");

    // load init
    assert(luaL_dofile(L, "lua/piecetable.lua") == 0);
    assert(luaL_dofile(L, "config.lua") == 0);
    e.bindings_normal = get_bindings(L, "normal");
//    e.bindings_insert = get_bindings(L, "insert");

    while (true) {
        win = editor_draw(e, win);
        e.status[0] = '\0';
        t.report("render:    ");
        e.cmd.push_back(utf8_getchar());
        t.start();
        e = editor_handle_command(std::move(e));
        if (e.lua_function) {
            reference_holder.push_back(e.buf.lines);
            // find function to call
            lua_getglobal(L, "bindings");
            lua_getfield(L, -1, e.mode == MODE_NORMAL ? "normal" : "insert");
            lua_getfield(L, -1, utf8_encode(*e.lua_function).c_str());
            assert(lua_isfunction(L, -1));
            // set up argument to function
            lua_newtable(L);
                // buffer
                lua_pushstring(L, "buffer");
                    buf* b = (buf*) lua_newuserdata(L, sizeof(buf)+e.buf.lines.size()*sizeof(char32_t*));
                    b->num_lines = e.buf.lines.size();
                    for (size_t i = 0; i < b->num_lines; i++) {
                        b->lines[i] = e.buf.lines[i]->c_str();
                    }
                    luaL_newmetatable(L, "buffer");
                    lua_pushstring(L, "__concat");
                    lua_pushcfunction(L, lua_buffer_concat);
                    lua_settable(L, -3);
                    lua_pushstring(L, "__len");
                    lua_pushcfunction(L, lua_buffer_len);
                    lua_settable(L, -3);
                    lua_pushstring(L, "__index");
                    lua_getglobal(L, "buffer");
                    lua_settable(L, -3);
                    lua_setmetatable(L, -2);
                lua_settable(L, -3);
                // position
                lua_pushstring(L, "x");
                lua_pushinteger(L, e.buf.pos.x + 1);
                lua_settable(L, -3);
                lua_pushstring(L, "y");
                lua_pushinteger(L, e.buf.pos.y + 1);
                lua_settable(L, -3);
                // mode
                lua_pushstring(L, "mode");
                lua_pushinteger(L, e.mode);
                lua_settable(L, -3);
            // call function
            lua_call(L, 1, 1);
            // read output
            assert(lua_istable(L, -1));
                lua_getfield(L, -1, "buffer");
                    if (!lua_isnil(L, -1)) {
                        buffer_lines ls = {};
                        auto bf = (buf*) lua_touserdata(L, -1);
                        for (size_t i = 0; i < bf->num_lines; i++) {
                            ls.push_back(std::make_shared<buffer_line>(bf->lines[i]));
                        }
                        e.buf.lines = ls;
                    }
                    lua_pop(L, 1);
                lua_getfield(L, -1, "x");
                    if (!lua_isnil(L, -1)) {
                        e.buf.pos.x = lua_tointeger(L, -1) - 1;
                    }
                    lua_pop(L, 1);
                lua_getfield(L, -1, "y");
                    if (!lua_isnil(L, -1)) {
                        e.buf.pos.y = lua_tointeger(L, -1) - 1;
                    }
                    lua_pop(L, 1);
                lua_getfield(L, -1, "exiting");
                    if (!lua_isnil(L, -1)) {
                        e.exiting = true;
                    }
                    lua_pop(L, 1);
            lua_pop(L, 1);
            //e.mode = (int) lua_tointeger(L, -1);
            while (lua_gettop(L))
                lua_pop(L, 1);
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

