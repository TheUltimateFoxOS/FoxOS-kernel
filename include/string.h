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