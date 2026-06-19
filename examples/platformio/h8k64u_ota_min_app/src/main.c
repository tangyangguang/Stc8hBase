#include "stc8h_boot_jump.h"
#include "stc8h_uart.h"

static void safe_outputs_init(void)
{
    /* First validation app has no controlled outputs. */
}

void main(void)
{
#if H8K64U_OTA_MIN_APP_EARLY_MARKER
    stc8h_uart_write_code(STC8H_UART1, "APP-ENTRY\r\n");
#endif
    safe_outputs_init();
    (void)stc8h_uart_init(STC8H_UART1);
    (void)stc8h_boot_mark_app_valid();
    stc8h_uart_write_code(STC8H_UART1, "H8K64U OTA app v1.0.0\r\n");

    while (1) {
    }
}
