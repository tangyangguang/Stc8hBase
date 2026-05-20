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

#define NRF24_UART_DIAG_RATE_250KBPS 0u
#define NRF24_UART_DIAG_RATE_1MBPS 1u
#define NRF24_UART_DIAG_RATE_2MBPS 2u

#define NRF24_UART_DIAG_POWER_NEG18DBM 0u
#define NRF24_UART_DIAG_POWER_NEG12DBM 1u
#define NRF24_UART_DIAG_POWER_NEG6DBM 2u
#define NRF24_UART_DIAG_POWER_0DBM 3u

#ifndef NRF24_UART_DIAG_CHANNEL
#define NRF24_UART_DIAG_CHANNEL 76u
#endif

#ifndef NRF24_UART_DIAG_DATA_RATE
#define NRF24_UART_DIAG_DATA_RATE NRF24_UART_DIAG_RATE_1MBPS
#endif

#ifndef NRF24_UART_DIAG_RF_POWER
#define NRF24_UART_DIAG_RF_POWER NRF24_UART_DIAG_POWER_0DBM
#endif

#ifndef NRF24_UART_DIAG_PAYLOAD_SIZE
#define NRF24_UART_DIAG_PAYLOAD_SIZE 15u
#endif

#ifndef NRF24_UART_DIAG_ACK_PAYLOAD
#define NRF24_UART_DIAG_ACK_PAYLOAD 1
#endif

#ifndef NRF24_UART_DIAG_DYNAMIC_PAYLOAD
#define NRF24_UART_DIAG_DYNAMIC_PAYLOAD NRF24_UART_DIAG_ACK_PAYLOAD
#endif

#ifndef NRF24_UART_DIAG_RETRANSMIT_DELAY_CODE
#define NRF24_UART_DIAG_RETRANSMIT_DELAY_CODE 1u
#endif

#ifndef NRF24_UART_DIAG_RETRANSMIT_COUNT_CODE
#define NRF24_UART_DIAG_RETRANSMIT_COUNT_CODE 15u
#endif

#if (NRF24_UART_DIAG_CHANNEL > 125u)
#error "NRF24_UART_DIAG_CHANNEL must be 0..125."
#endif

#if (NRF24_UART_DIAG_PAYLOAD_SIZE == 0u) || (NRF24_UART_DIAG_PAYLOAD_SIZE > DRV_NRF24L01_PAYLOAD_MAX)
#error "NRF24_UART_DIAG_PAYLOAD_SIZE must be 1..32."
#endif

#if NRF24_UART_DIAG_ACK_PAYLOAD && !NRF24_UART_DIAG_DYNAMIC_PAYLOAD
#error "ACK payload requires dynamic payload on nRF24L01+."
#endif

#if (NRF24_UART_DIAG_RETRANSMIT_DELAY_CODE > 15u) || (NRF24_UART_DIAG_RETRANSMIT_COUNT_CODE > 15u)
#error "NRF24_UART_DIAG_RETRANSMIT_DELAY_CODE and NRF24_UART_DIAG_RETRANSMIT_COUNT_CODE must be 0..15."
#endif

#if NRF24_UART_DIAG_DATA_RATE == NRF24_UART_DIAG_RATE_250KBPS
#define NRF24_UART_DIAG_RATE_VALUE DRV_NRF24L01_RATE_250KBPS
#define NRF24_UART_DIAG_RATE_TEXT "250kbps"
#elif NRF24_UART_DIAG_DATA_RATE == NRF24_UART_DIAG_RATE_1MBPS
#define NRF24_UART_DIAG_RATE_VALUE DRV_NRF24L01_RATE_1MBPS
#define NRF24_UART_DIAG_RATE_TEXT "1Mbps"
#elif NRF24_UART_DIAG_DATA_RATE == NRF24_UART_DIAG_RATE_2MBPS
#define NRF24_UART_DIAG_RATE_VALUE DRV_NRF24L01_RATE_2MBPS
#define NRF24_UART_DIAG_RATE_TEXT "2Mbps"
#else
#error "NRF24_UART_DIAG_DATA_RATE must be 0, 1, or 2."
#endif

#if NRF24_UART_DIAG_RF_POWER == NRF24_UART_DIAG_POWER_NEG18DBM
#define NRF24_UART_DIAG_POWER_VALUE DRV_NRF24L01_POWER_NEG18DBM
#define NRF24_UART_DIAG_POWER_TEXT "-18dBm"
#elif NRF24_UART_DIAG_RF_POWER == NRF24_UART_DIAG_POWER_NEG12DBM
#define NRF24_UART_DIAG_POWER_VALUE DRV_NRF24L01_POWER_NEG12DBM
#define NRF24_UART_DIAG_POWER_TEXT "-12dBm"
#elif NRF24_UART_DIAG_RF_POWER == NRF24_UART_DIAG_POWER_NEG6DBM
#define NRF24_UART_DIAG_POWER_VALUE DRV_NRF24L01_POWER_NEG6DBM
#define NRF24_UART_DIAG_POWER_TEXT "-6dBm"
#elif NRF24_UART_DIAG_RF_POWER == NRF24_UART_DIAG_POWER_0DBM
#define NRF24_UART_DIAG_POWER_VALUE DRV_NRF24L01_POWER_0DBM
#define NRF24_UART_DIAG_POWER_TEXT "0dBm"
#else
#error "NRF24_UART_DIAG_RF_POWER must be 0..3."
#endif

static STC8H_CODE stc8h_u8 diag_address[5] = {'T', 'O', 'Y', 'R', '1'};

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

static void uart_put_dec16(stc8h_u16 value)
{
    stc8h_u16 divisor;
    stc8h_u8 started;
    stc8h_u8 digit;

    divisor = 10000u;
    started = 0u;
    while (divisor != 0u) {
        digit = (stc8h_u8)(value / divisor);
        if ((digit != 0u) || (started != 0u) || (divisor == 1u)) {
            stc8h_uart_putc(STC8H_UART1, (char)('0' + digit));
            started = 1u;
        }
        value = (stc8h_u16)(value % divisor);
        divisor = (stc8h_u16)(divisor / 10u);
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

static void print_compile_config(void)
{
    stc8h_uart_write_code(STC8H_UART1, "config channel=");
    uart_put_dec16(NRF24_UART_DIAG_CHANNEL);
    stc8h_uart_write_code(STC8H_UART1, " rate=");
    stc8h_uart_write_code(STC8H_UART1, NRF24_UART_DIAG_RATE_TEXT);
    stc8h_uart_write_code(STC8H_UART1, " power=");
    stc8h_uart_write_code(STC8H_UART1, NRF24_UART_DIAG_POWER_TEXT);
    stc8h_uart_write_code(STC8H_UART1, " payload=");
    uart_put_dec16(NRF24_UART_DIAG_PAYLOAD_SIZE);
    stc8h_uart_write_code(STC8H_UART1, " dpl=");
    uart_put_dec8(NRF24_UART_DIAG_DYNAMIC_PAYLOAD);
    stc8h_uart_write_code(STC8H_UART1, " ack_payload=");
    uart_put_dec8(NRF24_UART_DIAG_ACK_PAYLOAD);
    stc8h_uart_write_code(STC8H_UART1, " ard_us=");
    uart_put_dec16((stc8h_u16)((NRF24_UART_DIAG_RETRANSMIT_DELAY_CODE + 1u) * 250u));
    stc8h_uart_write_code(STC8H_UART1, " arc=");
    uart_put_dec8(NRF24_UART_DIAG_RETRANSMIT_COUNT_CODE);
    uart_crlf();
}

static void configure_test_registers(void)
{
    drv_nrf24l01_power_down();
    drv_nrf24l01_flush_tx();
    drv_nrf24l01_flush_rx();
    drv_nrf24l01_clear_irq(DRV_NRF24L01_IRQ_MASK);

    (void)drv_nrf24l01_set_channel(NRF24_UART_DIAG_CHANNEL);
    (void)drv_nrf24l01_set_address_width(5u);
    (void)drv_nrf24l01_set_tx_address(diag_address, 5u);
    (void)drv_nrf24l01_set_rx_address(0u, diag_address, 5u);
    (void)drv_nrf24l01_set_payload_size(0u, NRF24_UART_DIAG_PAYLOAD_SIZE);
    drv_nrf24l01_set_rx_pipes(DRV_NRF24L01_PIPE0);
    drv_nrf24l01_set_auto_ack(DRV_NRF24L01_PIPE0);
    (void)drv_nrf24l01_set_auto_retransmit(NRF24_UART_DIAG_RETRANSMIT_DELAY_CODE,
                                           NRF24_UART_DIAG_RETRANSMIT_COUNT_CODE);
    (void)drv_nrf24l01_set_rate_power(NRF24_UART_DIAG_RATE_VALUE, NRF24_UART_DIAG_POWER_VALUE);
#if NRF24_UART_DIAG_ACK_PAYLOAD
    stc8h_uart_write_code(STC8H_UART1, "ENABLE_ACK_PAYLOAD: ");
    if (drv_nrf24l01_enable_ack_payload(DRV_NRF24L01_PIPE0) == STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "OK");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "FAIL");
    }
    uart_crlf();
#elif NRF24_UART_DIAG_DYNAMIC_PAYLOAD
    stc8h_uart_write_code(STC8H_UART1, "ENABLE_DPL: ");
    if (drv_nrf24l01_enable_dynamic_payload(DRV_NRF24L01_PIPE0) == STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "OK");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "FAIL");
    }
    uart_crlf();
#else
    drv_nrf24l01_disable_dynamic_payload();
    drv_nrf24l01_disable_ack_payload();
#endif
    dump_registers();
}

void main(void)
{
    drv_nrf24l01_init_pins();
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_spi_init();

    stc8h_uart_write_code(STC8H_UART1, "\r\nSTC8H nRF24 UART diag\r\n");
    stc8h_uart_write_code(STC8H_UART1, "pins CE=P1.6 CSN=P1.2 SCK=P1.5 MOSI=P1.3 MISO=P1.4 IRQ=P3.2\r\n");
    print_compile_config();
    print_status_line();
    check_present_loop();
    dump_registers();
    check_features();
    configure_test_registers();

    while (1) {
        stc8h_uart_write_code(STC8H_UART1, "heartbeat ");
        print_status_line();
        stc8h_delay_ms(1000u);
    }
}
