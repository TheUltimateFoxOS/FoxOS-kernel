#pragma once
#include <stdint.h>

extern "C" bool atomic_lock(uint64_t* mutex, uint64_t bit);
extern "C" bool atomic_unlock(uint64_t* mutex, uint64_t bit);
extern "C" void atomic_spinlock(uint64_t* mutex, uint64_t bit);