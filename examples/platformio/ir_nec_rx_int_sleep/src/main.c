#include "drv_ir_rx.h"
#include "stc8h_exti.h"
#include "stc8h_gpio.h"
#include "stc8h_interrupt.h"
#include "stc8h_power.h"
#include "stc8h_sfr.h"
#include "stc8h_timer.h"
#include "stc8h_uart.h"

#define IR_RX_MASK 0x04u
#define IR_RX_ACTIVE_TIMEOUT_TICKS 27648u
#define IR_RX_ACTIVE_TIMEOUT_CHUNKS 100u

static drv_ir_rx_t ir_rx;
static volatile stc8h_u8 last_edge_level;
static volatile stc8h_u16 last_edge_ticks;
static volatile stc8h_u8 active_chunks;
static volatile stc8h_u8 active_mode;
static volatile stc8h_u8 sleeping_mode;
static volatile stc8h_u8 woke_from_ir;
static volatile stc8h_u8 last_feed_level;
static volatile stc8h_u16 last_feed_width_us;

static stc8h_u8 ir_rx_level(void)
{
    return ((P3 & IR_RX_MASK) != 0u) ? 1u : 0u;
}

static void int0_init_both_edges(void)
{
    (void)stc8h_exti_configure(STC8H_EXTI_INT0, STC8H_EXTI_MODE_BOTH_EDGES);
    stc8h_exti_enable(STC8H_EXTI_INT0);
}

static void ir_rx_board_init(void)
{
    P_SW2 |= 0x80u;
    P3IE |= IR_RX_MASK;
    P3PU |= IR_RX_MASK;
    stc8h_gpio_set_mode(3u, 2u, STC8H_GPIO_MODE_INPUT_ONLY);
    (void)stc8h_timer0_init_free_run_12t();
    stc8h_timer_start(STC8H_TIMER0);
    last_edge_level = ir_rx_level();
    last_edge_ticks = stc8h_timer0_read();
    int0_init_both_edges();
}

static void uart_write_hex8(stc8h_u8 value)
{
    static const STC8H_CODE char hex[] = "0123456789ABCDEF";
    stc8h_uart_putc(STC8H_UART1, hex[value >> 4]);
    stc8h_uart_putc(STC8H_UART1, hex[value & 0x0Fu]);
}

static void uart_write_u16(stc8h_u16 value)
{
    static const STC8H_CODE stc8h_u16 divisors[5] = {
        10000u, 1000u, 100u, 10u, 1u
    };
    stc8h_u8 i;
    stc8h_u8 started;
    stc8h_u8 digit;

    started = 0u;
    for (i = 0u; i < 5u; ++i) {
        digit = 0u;
        while (value >= divisors[i]) {
            value = (stc8h_u16)(value - divisors[i]);
            ++digit;
        }
        if ((digit != 0u) || (started != 0u) || (i == 4u)) {
            started = 1u;
            stc8h_uart_putc(STC8H_UART1, (char)('0' + digit));
        }
    }
}

static void print_frame(const drv_ir_rx_frame_t *frame)
{
    stc8h_uart_write_code(STC8H_UART1, "frame addr=0x");
    uart_write_hex8(frame->address);
    stc8h_uart_write_code(STC8H_UART1, " cmd=0x");
    uart_write_hex8(frame->command);
    stc8h_uart_write_code(STC8H_UART1, "\r\n");
}

static void handle_event(void)
{
    drv_ir_rx_frame_t frame;
    drv_ir_rx_event_t event;

    EA = 0;
    event = drv_ir_rx_get_event(&ir_rx, &frame);
    EA = 1;

    if (event == DRV_IR_RX_EVENT_FRAME) {
        print_frame(&frame);
    } else if (event == DRV_IR_RX_EVENT_REPEAT) {
        stc8h_uart_write_code(STC8H_UART1, "repeat\r\n");
    } else if (event == DRV_IR_RX_EVENT_ERROR) {
        stc8h_uart_write_code(STC8H_UART1, "error level=");
        stc8h_uart_putc(STC8H_UART1, last_feed_level ? '1' : '0');
        stc8h_uart_write_code(STC8H_UART1, " width=");
        uart_write_u16(last_feed_width_us);
        stc8h_uart_write_code(STC8H_UART1, "us\r\n");
    }
}

static void enter_power_down_until_ir(void)
{
    drv_ir_rx_reset(&ir_rx);
    active_mode = 0u;
    active_chunks = 0u;
    sleeping_mode = 1u;
    woke_from_ir = 0u;

    stc8h_uart_write_code(STC8H_UART1, "sleep\r\n");
    while (ir_rx_level() == 0u) {
    }
    stc8h_exti_clear_flag(STC8H_EXTI_INT0);
    EA = 1;
    stc8h_power_down();
}

static void poll_active_timeout(void)
{
    stc8h_u16 now_ticks;
    stc8h_u16 width_ticks;

    if (active_mode == 0u) {
        return;
    }

    now_ticks = stc8h_timer0_read();
    width_ticks = (stc8h_u16)(now_ticks - last_edge_ticks);
    if (width_ticks > IR_RX_ACTIVE_TIMEOUT_TICKS) {
        last_edge_ticks = now_ticks;
        ++active_chunks;
        if (active_chunks >= IR_RX_ACTIVE_TIMEOUT_CHUNKS) {
            enter_power_down_until_ir();
        }
    }
}

STC8H_INTERRUPT(int0_isr, STC8H_VECTOR_INT0)
{
    stc8h_u8 level;
    stc8h_u16 now_ticks;
    stc8h_u16 width_ticks;
    stc8h_u16 width_us;

    level = ir_rx_level();
    now_ticks = stc8h_timer0_read();

    if (sleeping_mode != 0u) {
        sleeping_mode = 0u;
        woke_from_ir = 1u;
        active_mode = 1u;
        active_chunks = 0u;
        drv_ir_rx_reset(&ir_rx);
        stc8h_timer0_reset();
        stc8h_timer_start(STC8H_TIMER0);
        last_edge_ticks = stc8h_timer0_read();
        last_edge_level = level;
        return;
    }

    width_ticks = (stc8h_u16)(now_ticks - last_edge_ticks);
    width_us = stc8h_timer0_12t_ticks_to_us(width_ticks);
    last_feed_level = last_edge_level;
    last_feed_width_us = width_us;
    drv_ir_rx_feed_pulse(&ir_rx, last_edge_level, width_us);
    last_edge_level = level;
    last_edge_ticks = now_ticks;
    active_mode = 1u;
    active_chunks = 0u;
}

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    drv_ir_rx_init(&ir_rx);
    ir_rx_board_init();
    active_mode = 1u;
    active_chunks = 0u;
    sleeping_mode = 0u;
    woke_from_ir = 0u;
    EA = 1;

    stc8h_uart_write_code(STC8H_UART1, "ir nec rx int sleep P32\r\n");

    while (1) {
        if (woke_from_ir != 0u) {
            woke_from_ir = 0u;
            stc8h_uart_write_code(STC8H_UART1, "wake\r\n");
        }
        handle_event();
        poll_active_timeout();
    }
}
