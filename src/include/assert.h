#pragma once

//#define ASSERT_NO_PANIC

void __assert_fail(const char* __assertion, const char* __file, unsigned int __line);

#define assert(expr) if(!(expr)) __assert_fail(#expr, __FILE__, __LINE__);