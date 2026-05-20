#include "drv_nrf24l01.h"
#include "stc8h_delay.h"
#include "stc8h_spi.h"
#include "stc8h_uart.h"

#define NRF24_REG_CONFIG 0x00u
#define NRF24_REG_EN_AA 0x01u
#define NRF24_REG_EN_RXADDR 0x02u
#define NRF24_REG_SETUP_AW 0x03u
#define NRF24_REG_SETUP_RETR 0x04u
#define NRF24_REG_RF_CH 0x05u
#define NRF24_REG_RF_SETUP 0x06u
#define NRF24_REG_STATUS 0x07u
#define NRF24_REG_OBSERVE_TX 0x08u
#define NRF24_REG_RX_ADDR_P0 0x0Au
#define NRF24_REG_RX_PW_P0 0x11u
#define NRF24_REG_FIFO_STATUS 0x17u
#define NRF24_REG_DYNPD 0x1Cu
#define NRF24_REG_FEATURE 0x1Du

#define NRF24_DIAG_CHECK_COUNT 8u

static const STC8H_CODE char hex_chars[] = "0123456789ABCDEF";

static void uart_put_hex8(stc8h_u8 value)
{
    stc8h_uart_putc(STC8H_UART1, hex_chars[(value >> 4) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex_chars[value & 0x0Fu]);
}

static void uart_put_dec8(stc8h_u8 value)
{
    if (value >= 100u) {
        stc8h_uart_putc(STC8H_UART1, (char)('0' + (value / 100u)));
        value %= 100u;
        stc8h_uart_putc(STC8H_UART1, (char)('0' + (value / 10u)));
        stc8h_uart_putc(STC8H_UART1, (char)('0' + (value % 10u)));
    } else if (value >= 10u) {
        stc8h_uart_putc(STC8H_UART1, (char)('0' + (value / 10u)));
        stc8h_uart_putc(STC8H_UART1, (char)('0' + (value % 10u)));
    } else {
        stc8h_uart_putc(STC8H_UART1, (char)('0' + value));
    }
}

static void uart_crlf(void)
{
    stc8h_uart_write_code(STC8H_UART1, "\r\n");
}

static void print_reg(const STC8H_CODE char *name, stc8h_u8 reg)
{
    stc8h_uart_write_code(STC8H_UART1, name);
    stc8h_uart_write_code(STC8H_UART1, "=0x");
    uart_put_hex8(drv_nrf24l01_read_reg(reg));
    uart_crlf();
}

static void print_status_line(void)
{
    stc8h_u8 status;

    status = drv_nrf24l01_read_status();
    stc8h_uart_write_code(STC8H_UART1, "STATUS=0x");
    uart_put_hex8(status);
    if (status == 0x00u) {
        stc8h_uart_write_code(STC8H_UART1, " suspicious-low");
    } else if (status == 0xFFu) {
        stc8h_uart_write_code(STC8H_UART1, " suspicious-high");
    }
    uart_crlf();
}

static void check_present_loop(void)
{
    stc8h_u8 i;
    stc8h_u8 pass_count;

    pass_count = 0u;
    for (i = 0u; i < NRF24_DIAG_CHECK_COUNT; ++i) {
        stc8h_uart_write_code(STC8H_UART1, "CHECK ");
        uart_put_dec8((stc8h_u8)(i + 1u));
        stc8h_uart_write_code(STC8H_UART1, ": ");
        if (drv_nrf24l01_check_present() == STC8H_OK) {
            ++pass_count;
            stc8h_uart_write_code(STC8H_UART1, "PASS");
        } else {
            stc8h_uart_write_code(STC8H_UART1, "FAIL");
        }
        stc8h_uart_write_code(STC8H_UART1, " ");
        print_status_line();
    }

    stc8h_uart_write_code(STC8H_UART1, "CHECK_PASS=");
    uart_put_dec8(pass_count);
    stc8h_uart_write_code(STC8H_UART1, "/");
    uart_put_dec8(NRF24_DIAG_CHECK_COUNT);
    uart_crlf();
}

static void dump_registers(void)
{
    print_reg("CONFIG", NRF24_REG_CONFIG);
    print_reg("EN_AA", NRF24_REG_EN_AA);
    print_reg("EN_RXADDR", NRF24_REG_EN_RXADDR);
    print_reg("SETUP_AW", NRF24_REG_SETUP_AW);
    print_reg("SETUP_RETR", NRF24_REG_SETUP_RETR);
    print_reg("RF_CH", NRF24_REG_RF_CH);
    print_reg("RF_SETUP", NRF24_REG_RF_SETUP);
    print_reg("STATUS", NRF24_REG_STATUS);
    print_reg("OBSERVE_TX", NRF24_REG_OBSERVE_TX);
    print_reg("RX_ADDR_P0_0", NRF24_REG_RX_ADDR_P0);
    print_reg("RX_PW_P0", NRF24_REG_RX_PW_P0);
    print_reg("FIFO_STATUS", NRF24_REG_FIFO_STATUS);
    print_reg("DYNPD", NRF24_REG_DYNPD);
    print_reg("FEATURE", NRF24_REG_FEATURE);
}

static void check_features(void)
{
    drv_nrf24l01_power_down();
    drv_nrf24l01_flush_tx();
    drv_nrf24l01_flush_rx();
    drv_nrf24l01_clear_irq(0x70u);

    stc8h_uart_write_code(STC8H_UART1, "DPL: ");
    if (drv_nrf24l01_enable_dynamic_payload(DRV_NRF24L01_PIPE0) == STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "OK");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "FAIL");
    }
    uart_crlf();
    print_reg("FEATURE", NRF24_REG_FEATURE);
    print_reg("DYNPD", NRF24_REG_DYNPD);

    drv_nrf24l01_disable_dynamic_payload();
    stc8h_uart_write_code(STC8H_UART1, "ACK_PAYLOAD: ");
    if (drv_nrf24l01_enable_ack_payload(DRV_NRF24L01_PIPE0) == STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "OK");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "FAIL");
    }
    uart_crlf();
    print_reg("FEATURE", NRF24_REG_FEATURE);
    print_reg("DYNPD", NRF24_REG_DYNPD);
}

void main(void)
{
    drv_nrf24l01_init_pins();
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_spi_init();

    stc8h_uart_write_code(STC8H_UART1, "\r\nSTC8H nRF24 UART diag\r\n");
    stc8h_uart_write_code(STC8H_UART1, "pins CE=P1.6 CSN=P1.2 SCK=P1.5 MOSI=P1.3 MISO=P1.4 IRQ=P3.2\r\n");
    print_status_line();
    check_present_loop();
    dump_registers();
    check_features();

    while (1) {
        stc8h_uart_write_code(STC8H_UART1, "heartbeat ");
        print_status_line();
        stc8h_delay_ms(1000u);
    }
}
