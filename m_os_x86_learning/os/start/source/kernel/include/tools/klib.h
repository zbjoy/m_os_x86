#ifndef KLIB_H
#define KLIB_H

#include "comm/types.h"
#include <stdarg.h>

// down2: 向下对齐到指定的边界
// 例如: down2(0x1234, 0x100) = 0x1200
// 例如: down2(0x1234, 0x10) = 0x1230
static inline uint32_t down2(uint32_t size, uint32_t bound) {
    return size & ~(bound - 1);
}

// up2: 向上对齐到指定的边界
// 例如: up2(0x1234, 0x100) = 0x1300
static inline uint32_t up2(uint32_t size, uint32_t bound) {
    return (size + bound - 1) & ~(bound - 1);
}

void kernel_strcpy(char *dest, const char *src);
void kernel_strncpy(char *dest, const char *src, int size);
int kernel_strncmp(const char *s1, const char *s2, int size);
int kernel_strlen(const char* str);

void kernel_memcpy(void* dest, void* src, int size);
void kernel_memset(void* dest, uint8_t v, int size);
int kernel_memcmp(void* d1, void* d2, int size);

void kernel_sprintf(char* buf, const char* fmt,...);
void kernel_vsprintf(char* buf, const char* fmt, va_list args);

#ifndef RELEASE
#define ASSERT(expr) \
    if (!(expr)) pannic(__FILE__, __LINE__, __func__, #expr)

void pannic(const char* file, int line, const char* func, const char* cond);

#else
#define ASSERT(expr) \
    ((void)0)

#endif

char* get_file_name(const char* name);
int strings_count(char** start);

#endif /* KLIB_H */
