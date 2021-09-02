#pragma once

#include <stdint.h>

bool handle_signal(int signum);
void register_signal_handler(int signum, uint64_t handler);