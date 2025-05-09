#include "lib_syscall.h"

int main(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        print_msg("arg = %s\n", (int)argv[i]);
    }
    for (;;) {
        ms_sleep(1000);
    }
}
