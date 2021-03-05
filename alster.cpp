#include <cassert>
#include <chrono>
#include <iostream>
#include <set>

extern "C" {
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
}
#include "lua.h"

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


int main(int argc, char* argv[]) {
    assert(tty_enable_raw_mode() == 0);
    editor e {};
    timer t {};
    window win {};

    e.filename = argv[1];

    e.pos.x = 1;
    e.pos.y = 1;

    lua_State* L = lua_initialize();

    lua_load_file(L, e.filename);
    lua_state_to_editor(L, e);

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
                lua_push_state(L);
                lua_call(L, 1, 1);
                lua_update_state(L);
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
                lua_push_state(L);
                lua_call(L, 1, 1);
                lua_update_state(L);
                e.cmd = {};
                lua_pop(L, 1); // pop insert
                lua_pop(L, 1); // pop bindings
            }
            // check if e.cmd is a prefix of some command, otherwise insert cmd
            if (bindings.upper_bound(e.cmd) == bindings.end() ||
                bindings.upper_bound(e.cmd)->find(e.cmd) != 0) {
                lua_event_insert(L, utf8_encode(e.cmd).c_str());
                e.cmd = {};
            }
        }
        lua_state_to_editor(L, e);
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

