#include <chrono>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <string>
#include <termios.h>

#include "window.h"
#include "buffer.h"
#include "tokenize.h"
#include "file.h"

const char* output = "test";
const char* MESSAGE_COMMAND_NOT_FOUND = "No such command.";

static struct termios orig_termios;

void disable_raw_mode(int fd) {
    tcsetattr(fd,TCSAFLUSH,&orig_termios);
}

void editor_at_exit(void) {
    disable_raw_mode(STDIN_FILENO);
}

int enable_raw_mode() {
    struct termios raw;
    if (!isatty(STDIN_FILENO)) goto fatal;
    atexit(editor_at_exit);
    if (tcgetattr(STDIN_FILENO,&orig_termios) == -1) goto fatal;
    raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw) < 0) goto fatal;
    return 0;
fatal:
    errno = ENOTTY;
    return -1;
}

enum {
    MODE_NORMAL,
    MODE_INSERT
};

struct state {
    int mode;
    bool exiting;
    bool undo;
    bool checkpoint;
    bool redo;
    const char* status;
};

template <typename S, typename T>
std::pair<buffer, state> handle_input(buffer b, state s, S YYPEEK, T YYSKIP) {
    /*!re2c
    re2c:flags:input = custom;
    re2c:define:YYCTYPE = int;
    re2c:yyfill:enable = 0;
    nul = "\x00";
    esc = "\x1b";
    ret = "\x0d";
    del = "\x7f";
    tab = "\x09";
    */
    if (s.mode == MODE_NORMAL) {
        /*!re2c
        "$"  {return {buffer_move_end_of_line(std::move(b)), s};}
        "0"  {return {buffer_move_start_of_line(std::move(b)), s};}
        "A"  {s.mode = MODE_INSERT;
              return {buffer_move_end_of_line(std::move(b)), s};}
        "G"  {return {buffer_move_end(std::move(b)), s};}
        "dd" {s.checkpoint = true;
              return {buffer_erase_current_line(std::move(b)), s};}
        "gg" {return {buffer_move_start(std::move(b)), s};}
        "h"  {return {buffer_move_left(std::move(b), 1), s};}
        "i"  {s.mode = MODE_INSERT;
              return {std::move(b), s};}
        "j"  {return {buffer_move_down(std::move(b), 1), s};}
        "k"  {return {buffer_move_up(std::move(b), 1), s};}
        "l"  {return {buffer_move_right(std::move(b), 1), s};}
        "u"  {s.undo = true;
              return {std::move(b), s};}
        "r"  {s.redo = true;
              return {std::move(b), s};}
        "s"  {file_save(output, b); return {std::move(b), s};}
        *    {s.status = MESSAGE_COMMAND_NOT_FOUND; return {std::move(b), s};}
        */
    } else if (s.mode == MODE_INSERT) {
        /*!re2c
        del  {return {buffer_erase(std::move(b)), s};}
        esc  {s.checkpoint = true;
              s.mode = MODE_NORMAL;
              return {buffer_move_left(std::move(b), 1), s};}
        ret  {return {buffer_break_line(std::move(b)), s};}
        tab  {return {buffer_insert(std::move(b), ' ', 4), s};}
        nul  {return {std::move(b), s};}
        *    {return {buffer_insert(std::move(b), yych, 1), s};}
        */
    }
    return {b, s};
}

std::pair<buffer, state> handle_input_stdin(buffer b, state s) {
    int c;
    bool skip = true;
    return handle_input(
        std::move(b),
        s,
        [&](){
            if (skip) {
                c = getchar();
                skip = false;
            }
            return c;
        },
        [&](){
            skip = true;
        }
    );
}

int main(int argc, char* argv[]) {

    std::chrono::time_point<std::chrono::high_resolution_clock> timer_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> timer_end;
    std::vector<buffer> history;
    std::vector<buffer> future;
    window w;
    std::pair<buffer,state> state = {
        {{std::make_shared<buffer_line>()},{0, 0}},
        {}
    };
    auto& [b, s] = state;
    if (argc > 1) {
        b = file_load(argv[1]);
    }

    history.push_back(b);

    assert(enable_raw_mode() == 0);

    while (true) {

        /* render main window */
        w = window_update_size(w);
        w = window_update_scroll(state.first, w);
        window_render(b, w);

        /* print statusline, if any */
        if (s.status) {
            printf("\033[%ld;%ldH%s\033[K", w.height, 0ul, s.status);
            s.status = NULL;
        }

        /* stop timer, print time elapsed */
        timer_end = std::chrono::high_resolution_clock::now();
        auto timer_elapsed_ms = std::chrono::duration_cast<
                std::chrono::microseconds>(timer_end - timer_start).count();
        printf("\033[%ld;%ldH%6ld", w.height-1, w.width-9, timer_elapsed_ms);

        window_render_cursor(b, w);

        /* handle user input */
        state = handle_input_stdin(std::move(b), s);

        /* handle save history */
        if (s.checkpoint) {
            history.push_back(b);
            s.checkpoint = false;
        }

        /* handle undo */
        if (s.undo) {
            if (history.empty()) {
                s.status = "History empty!";
            } else {
                b = history.back();
                history.pop_back();
            }
            s.undo = false;
        }

        /* start timer */
        timer_start = std::chrono::high_resolution_clock::now();

    }
    return 0;
}

