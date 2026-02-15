// kernel/string.h
// Basic string and memory functions

#ifndef STRING_H
#define STRING_H

#include "types.h"

static inline void* memset(void* ptr, int value, size_t num) {
    u8* p = (u8*)ptr;
    for (size_t i = 0; i < num; i++) {
        p[i] = (u8)value;
    }
    return ptr;
}

static inline void* memcpy(void* dest, const void* src, size_t num) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    for (size_t i = 0; i < num; i++) {
        d[i] = s[i];
    }
    return dest;
}

static inline size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

static inline int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const u8*)s1 - *(const u8*)s2;
}

static inline char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

#endif // STRING_H
