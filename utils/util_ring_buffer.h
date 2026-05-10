#ifndef UTIL_RING_BUFFER_H
#define UTIL_RING_BUFFER_H

#include "stc8h_config.h"

typedef struct {
    STC8H_DATA stc8h_u8 *buffer;
    stc8h_u8 size;
    stc8h_u8 head;
    stc8h_u8 tail;
} util_ring_buffer_t;

void util_ring_buffer_init(util_ring_buffer_t *rb, STC8H_DATA stc8h_u8 *buffer, stc8h_u8 size);
stc8h_u8 util_ring_buffer_push(util_ring_buffer_t *rb, stc8h_u8 value);
stc8h_u8 util_ring_buffer_pop(util_ring_buffer_t *rb, stc8h_u8 *value);
stc8h_u8 util_ring_buffer_available(const util_ring_buffer_t *rb);

#endif
