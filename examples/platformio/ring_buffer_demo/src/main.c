#include "stc8h_uart.h"
#include "util_ring_buffer.h"

static STC8H_DATA stc8h_u8 rb_mem[4];

static void print_result(stc8h_u8 ok)
{
    if (ok != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "ring buffer ok\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "ring buffer error\r\n");
    }
}

void main(void)
{
    util_ring_buffer_t rb;
    stc8h_u8 value;
    stc8h_u8 ok;

    (void)stc8h_uart_init(STC8H_UART1);

    util_ring_buffer_init(&rb, rb_mem, sizeof(rb_mem));
    ok = 1u;

    if (util_ring_buffer_available(&rb) != 0u) ok = 0u;
    if (util_ring_buffer_pop(&rb, &value) != STC8H_FALSE) ok = 0u;

    if (util_ring_buffer_push(&rb, 1u) != STC8H_TRUE) ok = 0u;
    if (util_ring_buffer_push(&rb, 2u) != STC8H_TRUE) ok = 0u;
    if (util_ring_buffer_push(&rb, 3u) != STC8H_TRUE) ok = 0u;
    if (util_ring_buffer_push(&rb, 4u) != STC8H_FALSE) ok = 0u;
    if (util_ring_buffer_available(&rb) != 3u) ok = 0u;

    if ((util_ring_buffer_pop(&rb, &value) != STC8H_TRUE) || (value != 1u)) ok = 0u;
    if ((util_ring_buffer_pop(&rb, &value) != STC8H_TRUE) || (value != 2u)) ok = 0u;
    if (util_ring_buffer_push(&rb, 4u) != STC8H_TRUE) ok = 0u;
    if (util_ring_buffer_push(&rb, 5u) != STC8H_TRUE) ok = 0u;
    if (util_ring_buffer_available(&rb) != 3u) ok = 0u;

    if ((util_ring_buffer_pop(&rb, &value) != STC8H_TRUE) || (value != 3u)) ok = 0u;
    if ((util_ring_buffer_pop(&rb, &value) != STC8H_TRUE) || (value != 4u)) ok = 0u;
    if ((util_ring_buffer_pop(&rb, &value) != STC8H_TRUE) || (value != 5u)) ok = 0u;
    if (util_ring_buffer_pop(&rb, &value) != STC8H_FALSE) ok = 0u;

    while (1) {
        print_result(ok);
    }
}
