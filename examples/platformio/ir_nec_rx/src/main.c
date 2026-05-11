#include "drv_ir_rx.h"
#include "stc8h_gpio.h"
#include "stc8h_sfr.h"
#include "stc8h_timer.h"
#include "stc8h_uart.h"

#define IR_RX_MASK 0x04u
#define IR_RX_IDLE_TIMEOUT_TICKS 27648u
#define IR_RX_HEARTBEAT_CHUNKS 32u

static drv_ir_rx_t ir_rx;
static stc8h_u16 last_feed_width_us;

static stc8h_u8 ir_rx_level(void)
{
    return ((P3 & IR_RX_MASK) != 0u) ? 1u : 0u;
}

static void ir_rx_board_init(void)
{
    P_SW2 |= 0x80u;
    P3IE |= IR_RX_MASK;
    P3PU |= IR_RX_MASK;
    stc8h_gpio_set_mode(3u, 2u, STC8H_GPIO_MODE_INPUT_ONLY);
    (void)stc8h_timer0_init_free_run_12t();
    stc8h_timer_start(STC8H_TIMER0);
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

    event = drv_ir_rx_get_event(&ir_rx, &frame);
    if (event == DRV_IR_RX_EVENT_FRAME) {
        print_frame(&frame);
    } else if (event == DRV_IR_RX_EVENT_REPEAT) {
        stc8h_uart_write_code(STC8H_UART1, "repeat\r\n");
    } else if (event == DRV_IR_RX_EVENT_ERROR) {
        stc8h_uart_write_code(STC8H_UART1, "error interval=");
        uart_write_u16(last_feed_width_us);
        stc8h_uart_write_code(STC8H_UART1, "us\r\n");
    }
}

static void feed_falling_interval(stc8h_u16 width_us)
{
    last_feed_width_us = width_us;
    drv_ir_rx_feed_nec_falling_interval(&ir_rx, width_us);
}

void main(void)
{
    stc8h_u8 last_level;
    stc8h_u8 level;
    stc8h_u8 idle_chunks;
    stc8h_u8 falling_seen;
    stc8h_u16 last_falling_ticks;
    stc8h_u16 now_ticks;
    stc8h_u16 width_ticks;

    (void)stc8h_uart_init(STC8H_UART1);
    ir_rx_board_init();
    drv_ir_rx_init(&ir_rx);

    last_level = ir_rx_level();
    idle_chunks = 0u;
    falling_seen = 0u;
    last_falling_ticks = stc8h_timer0_read();
    stc8h_uart_write_code(STC8H_UART1, "ir nec rx P32\r\n");

    while (1) {
        level = ir_rx_level();
        now_ticks = stc8h_timer0_read();
        width_ticks = (stc8h_u16)(now_ticks - last_falling_ticks);

        if ((last_level != 0u) && (level == 0u)) {
            if (falling_seen != 0u) {
                feed_falling_interval(stc8h_timer0_12t_ticks_to_us(width_ticks));
                handle_event();
            }
            falling_seen = 1u;
            last_falling_ticks = now_ticks;
            last_level = level;
            idle_chunks = 0u;
        } else if (level != last_level) {
            last_level = level;
        } else if (width_ticks > IR_RX_IDLE_TIMEOUT_TICKS) {
            drv_ir_rx_finish_nec_falling_interval(&ir_rx);
            handle_event();
            falling_seen = 0u;
            last_falling_ticks = now_ticks;
            ++idle_chunks;
            if (idle_chunks >= IR_RX_HEARTBEAT_CHUNKS) {
                idle_chunks = 0u;
                stc8h_uart_write_code(STC8H_UART1, "level=");
                stc8h_uart_putc(STC8H_UART1, last_level ? '1' : '0');
                stc8h_uart_write_code(STC8H_UART1, "\r\n");
            }
        }
    }
}
