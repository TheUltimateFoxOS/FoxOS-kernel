#pragma once

#include <stdint.h>
#include <string.h>

extern uint8_t lapic_ids[256];
extern uint8_t numcore;
extern uint64_t lapic_ptr;
extern uint64_t ioapic_ptr;

void parse_madt(uint8_t *ptr);