#include "lib_syscall.h"
#include <stdio.h>

int main(int argc, char** argv) {
    sbrk(0);
    sbrk(100);
    sbrk(200);
    sbrk(4096 * 2 + 200);
    sbrk(4096 * 5 + 1234);

    printf("abef\b\b\b\bcd\n"); // cdef
    printf("abcd\x7f;fg\n"); // abc;fg
    
    // printf("\0337Hello,World!\0338123\n"); // ESC 7,8 输出123lo,word!
    // printf("\033[31;42mHello, World!!\033[39;49m123\n"); // ESC [pn m, Hello,world红色，其余绿色

    printf("123\033[2DHello,word!\n");                // 光标左移2，1Hello,word!
    printf("123\033[2CHello,word!\n");                // 光标右移2，123  Hello,word!

    printf("\033[31m");            // ESC [pn m, Hello,world红色，其余绿色
    printf("\033[10;10H test!\n"); // 定位到10, 10，test!
    printf("\033[20;20H test!\n"); // 定位到20, 20，test!
    printf("\033[32;25;39m123\n"); // ESC [pn m, Hello,world红色，其余绿色

    printf("\033[2J"); // 清屏

    printf("Hello World!!\n");
    printf("Hello From Shell!!\n");
    printf("os version: %s\n", "1.0.0");
    printf("%d %d %d\n", 1, 2, 3);

    for (int i = 0; i < argc; i++) {
        printf("arg = %s\n", (int)argv[i]);
    }

    fork();
    yield();

    for (;;) {
        printf("shell pid = %d\n", getpid());
        ms_sleep(1000);
    }
}
