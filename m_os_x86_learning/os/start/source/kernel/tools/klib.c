#include "kernel/include/tools/klib.h"
#include "kernel/include/tools/log.h"
#include "comm/cpu_instr.h"


char* get_file_name(const char* name) {
    char* s = (char*)name;

    while (*s != '\0') {
        s++;
    }

    while ((*s != '/') && (*s != '\\') && (s >= name)) {
        s--;
    }

    // 此时 s 指向最后一个 '/' 或者 '\\'，或者指向 name 的开头, 所以 s+1 指向文件名的第一个字符
    return s + 1;
}

int strings_count(char** start) {
    int count = 0;

    if (start) {
        while (*start++) {
            count++;
        }
    }

    return count;
}

void kernel_strcpy(char *dest, const char *src)
{
    if (!dest ||!src) {
        return;
    }
    while (*dest && *src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}
void kernel_strncpy(char *dest, const char *src, int size)
{
    if (!dest ||!src || !size) {
        return;
    }
    char* d = dest;
    const char* s = src;
    while ((size-- > 0) && (*s)) {
        *d++ = *s++;
    }
    if (size == 0) {
        *(d-1) = '\0';
    } else {
        *d = '\0';
    }
}

int kernel_strncmp(const char *s1, const char *s2, int size)
{
    if (!s1 ||!s2) {
        return -1;
    }
    while (*s1 && *s2 && (*s1 == *s2) && (size-- > 0)) {
        s1++;
        s2++;
    }
    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}


int kernel_strlen(const char *str)
{
    if (!str) {
        return 0;
    }
    const char* c = str;
    int len = 0;
    while (*c++) {
        len++;
    }
    
    return len;
}

void kernel_memcpy(void *dest, void *src, int size)
{
    if (!dest ||!src || !size) {
        return;
    }
    uint8_t* d = (uint8_t*)dest;
    uint8_t* s = (uint8_t*)src;
    while (size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void *dest, uint8_t v, int size)
{
    if (!dest || !size) {
        return;
    }
    uint8_t* d = (uint8_t*)dest;
    while (size--) {
        *d++ = v;
    }
}

int kernel_memcmp(void *d1, void *d2, int size)
{
    if (!d1 ||!d2 || !size) {
        return 1;
    }
    uint8_t* p_d1 = (uint8_t*)d1;
    uint8_t* p_d2 = (uint8_t*)d2;
    while (size--) {
        if (*p_d1++ != *p_d2++) {
            return 1;
        }
    }
    return 0;
}

void kernel_sprintf(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kernel_vsprintf(buf, fmt, args);
    va_end(args);
}

void kernel_itoa(char* buf, int num, int base) {
    static const char* num2ch ={"FEDCBA9876543210123456789ABCDEF"};
    char* p = buf;
    int old_num = num;
    if ((base != 2) && (base != 8) && (base != 10) && (base != 16)) {
        *p = '\0';
        return;
    }

    if ((num < 0) && (base == 10)) {
        *p++ = '-';
    }

    do {
        char ch = num2ch[num % base + 15];
        *p++ = ch;
        num /= base;
    } while (num);
    *p-- = '\0';

    // char* start = p;
    char* start = (old_num > 0) ? buf : (buf + 1);
    while (start < p)
    {
        char ch = *start;
        *start = *p;
        *p = ch;
        start++;
        p--;
    }
}

/**
 * @brief kernel_vsprintf
 * @param buf
 * @param fmt
 * @param args
 * @return
 * 
 * 
 * %[d] 10进制整数
 * %[x] 16进制整数
 * %[c] 单个字符
 * %[s] 字符串  
 * 
 * 注意：
 * 1. 目前只支持%[d], %[x], %[c], %[s]四种格式化符号
 * 2. 目前只支持整数格式化符号，不支持浮点数格式化符号  
 * 3. 目前只支持十进制和十六进制两种进制格式化符号
 * 4. 目前只支持字符串格式化符号，不支持宽字符格式化符号
 * 5. 目前只支持格式化字符串，不支持格式化数组
 * 6. 目前只支持格式化字符串，不支持格式化指针
 * 7. 目前只支持格式化字符串，不支持格式化结构体
 * 8. 目前只支持格式化字符串，不支持格式化联合体
 * 9. 目前只支持格式化字符串，不支持格式化枚举类型
 * 10. 目前只支持格式化字符串，不支持格式化函数指针
 */
void kernel_vsprintf(char* buf, const char* fmt, va_list args) {
    enum
    {
        NORMAL,
        READ_FMT
    } state = NORMAL;

    char* curr = buf;
    char ch;

    // while ((ch = *fmt++)) {
        // *curr++ = ch;
    // }

    while ((ch = *fmt++))
    {
        switch (state) {
        case NORMAL:
            if (ch == '%') {
                state = READ_FMT;
            } else {
                *curr++ = ch;
            }
            break;
        case READ_FMT:
            if (ch == 'd') {
                int num = va_arg(args, int);
                kernel_itoa(curr, num, 10);
                curr += kernel_strlen(curr);
            } else if (ch == 'x') {
                int num = va_arg(args, int);
                kernel_itoa(curr, num, 16);
                curr += kernel_strlen(curr);
            } else if (ch == 'c') {
                // char c = (char)va_arg(args, char);
                char c = va_arg(args, int);
                *curr++ = c;
            } else if (ch == 's') {
                const char* str = va_arg(args, char*);
                int len = kernel_strlen(str);
                while (len--) {
                    *curr++ = *str++;
                }
            }

            state = NORMAL;
            break;
        }
    }
}


void pannic(const char* file, int line, const char* func, const char* cond) {
    log_printf("assert failed! %s", cond);
    log_printf("file:%s, line:%d, func:%s", file, line, func);
    for (;;) {
        hlt();
    }
}
