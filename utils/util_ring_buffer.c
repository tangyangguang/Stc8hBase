#include "util_ring_buffer.h"

static stc8h_u8 util_ring_buffer_next(const util_ring_buffer_t *rb, stc8h_u8 index)
{
    ++index;
    if (index >= rb->size) {
        index = 0u;
    }
    return index;
}

void util_ring_buffer_init(util_ring_buffer_t *rb, STC8H_DATA stc8h_u8 *buffer, stc8h_u8 size)
{
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0u;
    rb->tail = 0u;
}

stc8h_u8 util_ring_buffer_push(util_ring_buffer_t *rb, stc8h_u8 value)
{
    stc8h_u8 next;

    next = util_ring_buffer_next(rb, rb->head);
    if (next == rb->tail) {
        return STC8H_FALSE;
    }

    rb->buffer[rb->head] = value;
    rb->head = next;
    return STC8H_TRUE;
}

stc8h_u8 util_ring_buffer_pop(util_ring_buffer_t *rb, stc8h_u8 *value)
{
    if (rb->head == rb->tail) {
        return STC8H_FALSE;
    }

    *value = rb->buffer[rb->tail];
    rb->tail = util_ring_buffer_next(rb, rb->tail);
    return STC8H_TRUE;
}

stc8h_u8 util_ring_buffer_available(const util_ring_buffer_t *rb)
{
    if (rb->head >= rb->tail) {
        return (stc8h_u8)(rb->head - rb->tail);
    }

    return (stc8h_u8)(rb->size - rb->tail + rb->head);
}
