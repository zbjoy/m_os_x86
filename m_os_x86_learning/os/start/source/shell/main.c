#include "lib_syscall.h"
#include <stdio.h>

int main(int argc, char** argv) {
    sbrk(0);
    sbrk(100);
    sbrk(200);
    sbrk(4096 * 2 + 200);
    sbrk(4096 * 5 + 1234);

    printf("Hello World!!\n");

    for (int i = 0; i < argc; i++) {
        print_msg("arg = %s\n", (int)argv[i]);
    }

    fork();
    yield();

    for (;;) {
        print_msg("shell pid = %d\n", getpid());
        ms_sleep(1000);
    }
}
