#include <cassert>
#include <chrono>

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

int main(int argc, char* argv[]) {
    editor e {};
    timer t {};
    window win {};
    if (argc > 1) {
        e.buf = file_load(argv[1]);
        e.filename = argv[1];
    } else {
        e.buf = {{std::make_shared<buffer_line>()}, {0, 0}};
        e.filename = "/tmp/alster.tmp";
    }
    assert(tty_enable_raw_mode() == 0);
    while (true) {
        win = editor_draw(e, win);
        e.status[0] = '\0';
        t.report("render:    ");
        e.cmd.push_back(utf8_getchar());
        t.start();
        e = editor_handle_command(std::move(e));
        if (e.saving) {
            file_save(e.filename, e.buf);
            sprintf(e.status, "Saving %s", e.filename);
        }
        t.report("handle cmd:");
        t.start();
        if (e.exiting) break;
    }
    return 0;
}

