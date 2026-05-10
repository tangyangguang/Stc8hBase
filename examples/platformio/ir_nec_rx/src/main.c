#include "drv_ir_rx.h"
#include "stc8h_gpio.h"
#include "stc8h_sfr.h"
#include "stc8h_uart.h"

#define IR_RX_MASK 0x04u
#define IR_RX_IDLE_TIMEOUT_TICKS 27648u
#define IR_RX_HEARTBEAT_CHUNKS 32u

static drv_ir_rx_t ir_rx;
static stc8h_u8 last_feed_level;
static stc8h_u16 last_feed_width_us;

static stc8h_u16 timer0_read(void)
{
    stc8h_u8 high1;
    stc8h_u8 low;
    stc8h_u8 high2;

    do {
        high1 = TH0;
        low = TL0;
        high2 = TH0;
    } while (high1 != high2);

    return (stc8h_u16)(((stc8h_u16)high1 << 8) | low);
}

static stc8h_u16 timer0_ticks_to_us(stc8h_u16 ticks)
{
    return (stc8h_u16)(ticks + (ticks >> 4) + (ticks >> 6) + (ticks >> 7));
}

static stc8h_u8 ir_rx_level(void)
{
    return ((P3 & IR_RX_MASK) != 0u) ? 1u : 0u;
}

static void timer0_init_free_run(void)
{
    TR0 = 0;
    ET0 = 0;
    TMOD = (stc8h_u8)((TMOD & (stc8h_u8)~0x0Fu) | 0x01u);
    AUXR &= (stc8h_u8)~0x80u;
    INTCLKO &= (stc8h_u8)~0x01u;
    TH0 = 0u;
    TL0 = 0u;
    TF0 = 0;
    TR0 = 1;
}

static void ir_rx_board_init(void)
{
    P_SW2 |= 0x80u;
    P3IE |= IR_RX_MASK;
    P3PU |= IR_RX_MASK;
    stc8h_gpio_set_mode(3u, 2u, STC8H_GPIO_MODE_INPUT_ONLY);
    timer0_init_free_run();
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
        stc8h_uart_write_code(STC8H_UART1, "error level=");
        stc8h_uart_putc(STC8H_UART1, last_feed_level ? '1' : '0');
        stc8h_uart_write_code(STC8H_UART1, " width=");
        uart_write_u16(last_feed_width_us);
        stc8h_uart_write_code(STC8H_UART1, "us\r\n");
    }
}

static void feed_pulse(stc8h_u8 level, stc8h_u16 width_us)
{
    last_feed_level = level;
    last_feed_width_us = width_us;
    drv_ir_rx_feed_pulse(&ir_rx, level, width_us);
}

void main(void)
{
    stc8h_u8 last_level;
    stc8h_u8 level;
    stc8h_u8 idle_chunks;
    stc8h_u16 last_ticks;
    stc8h_u16 now_ticks;
    stc8h_u16 width_ticks;

    (void)stc8h_uart_init(STC8H_UART1);
    ir_rx_board_init();
    drv_ir_rx_init(&ir_rx);

    last_level = ir_rx_level();
    idle_chunks = 0u;
    last_ticks = timer0_read();
    stc8h_uart_write_code(STC8H_UART1, "ir nec rx P32\r\n");

    while (1) {
        level = ir_rx_level();
        now_ticks = timer0_read();
        width_ticks = (stc8h_u16)(now_ticks - last_ticks);

        if (level != last_level) {
            feed_pulse(last_level, timer0_ticks_to_us(width_ticks));
            last_level = level;
            last_ticks = now_ticks;
            idle_chunks = 0u;
            handle_event();
        } else if (width_ticks > IR_RX_IDLE_TIMEOUT_TICKS) {
            feed_pulse(last_level, timer0_ticks_to_us(width_ticks));
            last_ticks = now_ticks;
            ++idle_chunks;
            if (idle_chunks >= IR_RX_HEARTBEAT_CHUNKS) {
                idle_chunks = 0u;
                stc8h_uart_write_code(STC8H_UART1, "level=");
                stc8h_uart_putc(STC8H_UART1, last_level ? '1' : '0');
                stc8h_uart_write_code(STC8H_UART1, "\r\n");
            }
            handle_event();
        }
    }
}
