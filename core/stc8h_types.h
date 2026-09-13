#ifndef STC8H_TYPES_H
#define STC8H_TYPES_H

/* Match the MCU wire/arithmetic widths when running C services on a host.
 * SDCC and Keil keep their native 8051 types and require no stdint library. */
#if defined(__SDCC) || defined(__C51__) || defined(__CX51__)
typedef unsigned char stc8h_u8;
typedef unsigned int stc8h_u16;
typedef unsigned long stc8h_u32;

typedef signed char stc8h_s8;
typedef signed int stc8h_s16;
typedef signed long stc8h_s32;

#else
#include <stdint.h>
typedef uint8_t stc8h_u8;
typedef uint16_t stc8h_u16;
typedef uint32_t stc8h_u32;
typedef int8_t stc8h_s8;
typedef int16_t stc8h_s16;
typedef int32_t stc8h_s32;
#endif

typedef enum {
    STC8H_OK = 0,
    STC8H_ERROR = 1,
    STC8H_TIMEOUT = 2,
    STC8H_BUSY = 3
} stc8h_status_t;

#ifndef STC8H_TRUE
#define STC8H_TRUE 1u
#endif

#ifndef STC8H_FALSE
#define STC8H_FALSE 0u
#endif

#endif
