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
#include "tty.h"
#include "utf8.h"

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

int main(int argc, char* argv[]) {
    assert(tty_enable_raw_mode() == 0);
    editor e {};
    timer t {};
    e.filename = argv[1];
    lua_State* L = lua_initialize();
    lua_load_file(L, e.filename);
    lua_state_to_editor(L, e);
    while (true) {
        editor_draw(e);
        e.status[0] = '\0';
        t.report("render:    ");
        e.cmd += getchar_utf8();
        lua_set_cmd(L, e.cmd.c_str());
        lua_handle_cmd(L);
        t.start();
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

