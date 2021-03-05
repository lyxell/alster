#ifndef TTY_H
#define TTY_H

#ifdef __cplusplus
extern "C" {
#endif
int tty_enable_raw_mode();
void tty_disable_raw_mode();
#ifdef __cplusplus
}
#endif

#endif
