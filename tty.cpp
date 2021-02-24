#include <cstdlib>
#include <termios.h>
#include <unistd.h>

static struct termios orig_termios;

void tty_disable_raw_mode(int fd) {
    tcsetattr(fd,TCSAFLUSH,&orig_termios);
}

void tty_editor_at_exit(void) {
    tty_disable_raw_mode(STDIN_FILENO);
}

int tty_enable_raw_mode() {
    struct termios raw;
    if (!isatty(STDIN_FILENO)) return -1;
    atexit(tty_editor_at_exit);
    if (tcgetattr(STDIN_FILENO,&orig_termios) == -1) return -1;
    raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw) < 0) return -1;
    return 0;
}
