#ifndef STC8H_TYPES_H
#define STC8H_TYPES_H

typedef unsigned char stc8h_u8;
typedef unsigned int stc8h_u16;
typedef unsigned long stc8h_u32;

typedef signed char stc8h_s8;
typedef signed int stc8h_s16;
typedef signed long stc8h_s32;

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
