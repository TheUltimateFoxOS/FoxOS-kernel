#pragma once

//#define ASSERT_NO_PANIC

void __assert_fail(const char* __assertion, const char* __file, unsigned int __line);
void __assert_fail_nopanic(const char* __assertion, const char* __file, unsigned int __line);

#ifndef ASSERT_NO_PANIC
#define assert(expr) if(!(expr)) __assert_fail(#expr, __FILE__, __LINE__);
#else
#define assert(expr) if(!(expr)) { __assert_fail_nopanic(#expr, __FILE__, __LINE__); goto assert_fail; }
#endif