#pragma once

#include <stddef.h>

char* strcpy(char* dest, const char* src);
int strlen(char* src);
size_t strnlen(const char *s, size_t maxlen);
char* strcat(char* dest, const char* src);
int strcmp(char* str1, char* str2);
const char* strstr(const char* X, const char* Y);
char* strchr(const char* s, int c);
char* strrchr (const char* s, int c);
int strncmp(const char* s1, const char* s2, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
int memcmp(const void * _s1, const void* _s2, size_t n);
char* strncpy(char* _dst, const char* _src, size_t count);