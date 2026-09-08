#include "stc8h_boot_jump.h"
#include "stc8h_interrupt.h"
#include "stc8h_uart.h"
#include "stc8h_wdt.h"

static void safe_outputs_init(void)
{
    /* First validation app has no controlled outputs. */
}

STC8H_INTERRUPT(ota_app_uart2_vector_probe, STC8H_VECTOR_UART2)
{
    /* Compile-only proof that the protected vector table reaches 0x6C43. */
}

void main(void)
{
    safe_outputs_init();
    (void)stc8h_uart_init(STC8H_UART1);
    (void)stc8h_boot_mark_app_valid();
    stc8h_uart_write_code(STC8H_UART1, "H8K64U OTA app v1.0.0\r\n");

    while (1) {
        /* Bootloader leaves the trial watchdog running; the application owns it
         * after health confirmation and must continue feeding it. */
        stc8h_wdt_feed();
    }
}
