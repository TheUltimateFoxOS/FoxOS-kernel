#pragma once

#include <stdint.h>
#include <apic/madt.h>
#include <string.h>
#include <scheduling/pit/pit.h>
#include <paging/page_table_manager.h>

void start_smp();