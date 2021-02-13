```c
#include <stdio.h>

#define YYPEEK()  (c = getchar())
#define YYSKIP()  (c)

int main()
{
    char c;
    system ("/bin/stty raw");
    system ("/bin/stty -echo");
    for (;;) {
        /*!re2c
        re2c:flags:input = custom;
        re2c:define:YYCTYPE = char;
        re2c:yyfill:enable = 0;

        *      { printf("no such command\r\n"); continue; }
        "gg"   { printf("go to top\r\n"); continue; }
        "G"    { printf("go to bottom\r\n"); continue; }
        "q"    { return 0; }

        */
    }
    return 0;
}
```
