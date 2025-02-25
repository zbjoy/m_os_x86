#include "kernel/include/tools/klib.h"

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

int strncmp(const char *s1, const char *s2, int size)
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