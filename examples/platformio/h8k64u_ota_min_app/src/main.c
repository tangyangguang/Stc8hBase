#include "stc8h_boot_jump.h"
#include "stc8h_boot_control_iap.h"
#include "stc8h_interrupt.h"
#include "stc8h_uart.h"
#include "stc8h_wdt.h"

static void safe_outputs_init(void)
{
    /* First validation app has no controlled outputs. */
}

#if H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP
static void lab_poll_explicit_update_request(void)
{
    static const stc8h_u8 prefix[4] = {0x4Fu, 0x54u, 0x41u, 0x21u};
    static stc8h_u8 matched;
    static stc8h_u8 session_bytes;
    static stc8h_u32 session_id;
    stc8h_u8 value;

    if (stc8h_uart_readable(STC8H_UART1) == 0u) {
        return;
    }
    value = (stc8h_u8)stc8h_uart_getc(STC8H_UART1);
    if (matched < 4u) {
        matched = value == prefix[matched] ? (stc8h_u8)(matched + 1u) : 0u;
        session_bytes = 0u;
        session_id = 0UL;
        return;
    }
    session_id |= ((stc8h_u32)value << (session_bytes * 8u));
    ++session_bytes;
    if (session_bytes == 4u) {
        matched = 0u;
        session_bytes = 0u;
        if ((session_id != 0UL) &&
            (stc8h_boot_request_update(session_id) == STC8H_OK)) {
            stc8h_uart_write_code(STC8H_UART1, "OTA ACK\r\n");
            stc8h_boot_controlled_reset();
        }
        session_id = 0UL;
    }
}
#endif

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
#if H8K64U_OTA_MIN_APP_ENABLE_MARK_VALID_IAP
        lab_poll_explicit_update_request();
#endif
    }
}
