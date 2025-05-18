#include <stdio.h>

int main(void)
{
    // printf("\0337Hello,word!\0338123\n");             // ESC 7,8 输出123lo,word!
    // printf("\033[31;42mHello,word!\033[39;49m123\n"); // ESC [pn m, Hello,world红色，>其余绿色
    // printf("123\033[2DHello,word!\n");                // 光标左移2，1Hello,word!
    // printf("123\033[2CHello,word!\n");                // 光标右移2，123  Hello,word!

    // printf("\033[31m");            // ESC [pn m, Hello,world红色，其余绿色
    // printf("\033[10;10H test!\n"); // 定位到10, 10，test!
    // printf("\033[20;20H test!\n"); // 定位到20, 20，test!
    // printf("\033[32;25;39m123\n"); // ESC [pn m, Hello,world红色，其余绿色

    printf("\0337Hello, World1\0338123\n");
    return 0;
}