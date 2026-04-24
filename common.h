#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UINT8_COUNT (UINT8_MAX + 1)
#define MAX_LOCALS  65536   // raised local variable cap (uses 2-byte slot operand)

// Uncomment to enable bytecode disassembly trace
// #define DEBUG_PRINT_CODE
// #define DEBUG_TRACE_EXECUTION

#endif