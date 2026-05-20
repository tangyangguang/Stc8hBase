#include "drv_nrf24l01.h"
#include "stc8h_delay.h"
#include "stc8h_spi.h"
#include "stc8h_uart.h"

#if defined(NRF24_PAIR_DIAG_PTX) && defined(NRF24_PAIR_DIAG_PRX)
#error "Select only one nRF24 pair diagnostic role."
#endif

#if !defined(NRF24_PAIR_DIAG_PTX) && !defined(NRF24_PAIR_DIAG_PRX)
#define NRF24_PAIR_DIAG_PTX 1
#endif

#define NRF24_PAIR_CHANNEL 76u
#define NRF24_PAIR_PAYLOAD_SIZE 32u
#define NRF24_PAIR_TX_WAIT_LIMIT 9000u
#define NRF24_PAIR_RETRANSMIT_DELAY_CODE 5u
#define NRF24_PAIR_RETRANSMIT_COUNT_CODE 3u

static const STC8H_CODE char hex_chars[] = "0123456789ABCDEF";
static const stc8h_u8 pair_address[5] = {'T', 'O', 'Y', 'R', '1'};

static STC8H_XDATA stc8h_u8 payload[NRF24_PAIR_PAYLOAD_SIZE];
static STC8H_XDATA stc8h_u8 ack_payload[NRF24_PAIR_PAYLOAD_SIZE];

static void uart_crlf(void)
{
    stc8h_uart_write_code(STC8H_UART1, "\r\n");
}

static void uart_put_hex8(stc8h_u8 value)
{
    stc8h_uart_putc(STC8H_UART1, hex_chars[(value >> 4) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex_chars[value & 0x0Fu]);
}

static void uart_put_hex16(stc8h_u16 value)
{
    uart_put_hex8((stc8h_u8)(value >> 8));
    uart_put_hex8((stc8h_u8)value);
}

static void print_hex_field(const STC8H_CODE char *name, stc8h_u8 value)
{
    stc8h_uart_write_code(STC8H_UART1, " ");
    stc8h_uart_write_code(STC8H_UART1, name);
    stc8h_uart_write_code(STC8H_UART1, "=0x");
    uart_put_hex8(value);
}

static void print_hex16_field(const STC8H_CODE char *name, stc8h_u16 value)
{
    stc8h_uart_write_code(STC8H_UART1, " ");
    stc8h_uart_write_code(STC8H_UART1, name);
    stc8h_uart_write_code(STC8H_UART1, "=0x");
    uart_put_hex16(value);
}

static void print_payload_head(const STC8H_XDATA stc8h_u8 *data, stc8h_u8 len)
{
    stc8h_u8 i;
    stc8h_u8 count;

    count = len;
    if (count > 8u) {
        count = 8u;
    }
    stc8h_uart_write_code(STC8H_UART1, " head=");
    for (i = 0u; i < count; ++i) {
        if (i != 0u) {
            stc8h_uart_putc(STC8H_UART1, ',');
        }
        uart_put_hex8(data[i]);
    }
}

static void fill_payload(stc8h_u8 tag0, stc8h_u8 tag1, stc8h_u8 seq, stc8h_u16 count)
{
    stc8h_u8 i;

    payload[0] = tag0;
    payload[1] = tag1;
    payload[2] = seq;
    payload[3] = (stc8h_u8)count;
    payload[4] = (stc8h_u8)(count >> 8);
    for (i = 5u; i < NRF24_PAIR_PAYLOAD_SIZE; ++i) {
        payload[i] = (stc8h_u8)(seq + i);
    }
}

static void fill_ack_payload(stc8h_u8 last_seq, stc8h_u16 rx_count)
{
    stc8h_u8 i;

    ack_payload[0] = 'A';
    ack_payload[1] = 'C';
    ack_payload[2] = 'K';
    ack_payload[3] = last_seq;
    ack_payload[4] = (stc8h_u8)rx_count;
    ack_payload[5] = (stc8h_u8)(rx_count >> 8);
    for (i = 6u; i < NRF24_PAIR_PAYLOAD_SIZE; ++i) {
        ack_payload[i] = (stc8h_u8)(0xA0u + i);
    }
}

static void print_common_banner(void)
{
    stc8h_uart_write_code(STC8H_UART1, "\r\nSTC8H nRF24 pair diag ");
#if NRF24_PAIR_DIAG_PTX
    stc8h_uart_write_code(STC8H_UART1, "PTX\r\n");
#else
    stc8h_uart_write_code(STC8H_UART1, "PRX\r\n");
#endif
    stc8h_uart_write_code(STC8H_UART1, "pins CE=P1.6 CSN=P1.2 SCK=P1.5 MOSI=P1.3 MISO=P1.4 IRQ=P3.2\r\n");
    stc8h_uart_write_code(STC8H_UART1, "channel=76 addr=TOYR1 rate=250kbps power=0dBm retr=1500usx3\r\n");
}

static stc8h_status_t print_config_fail(const STC8H_CODE char *step)
{
    stc8h_uart_write_code(STC8H_UART1, "CONFIG: FAIL ");
    stc8h_uart_write_code(STC8H_UART1, step);
    print_hex_field("STATUS", drv_nrf24l01_read_status());
    uart_crlf();
    return STC8H_ERROR;
}

static stc8h_status_t configure_radio_common(void)
{
    drv_nrf24l01_power_down();
    drv_nrf24l01_flush_tx();
    drv_nrf24l01_flush_rx();
    drv_nrf24l01_clear_irq(0x70u);

    if (drv_nrf24l01_check_present() != STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "CHECK: FAIL");
        print_hex_field("STATUS", drv_nrf24l01_read_status());
        uart_crlf();
        return STC8H_ERROR;
    }
    stc8h_uart_write_code(STC8H_UART1, "CHECK: PASS");
    print_hex_field("STATUS", drv_nrf24l01_read_status());
    uart_crlf();

    if (drv_nrf24l01_set_channel(NRF24_PAIR_CHANNEL) != STC8H_OK) {
        return print_config_fail("RF_CH");
    }
    if (drv_nrf24l01_set_address_width(5u) != STC8H_OK) {
        return print_config_fail("SETUP_AW");
    }
    if (drv_nrf24l01_set_tx_address(pair_address, 5u) != STC8H_OK) {
        return print_config_fail("TX_ADDR");
    }
    if (drv_nrf24l01_set_rx_address(0u, pair_address, 5u) != STC8H_OK) {
        return print_config_fail("RX_ADDR_P0");
    }
    if (drv_nrf24l01_set_payload_size(0u, NRF24_PAIR_PAYLOAD_SIZE) != STC8H_OK) {
        return print_config_fail("RX_PW_P0");
    }
    if (drv_nrf24l01_enable_ack_payload(DRV_NRF24L01_PIPE0) != STC8H_OK) {
        return print_config_fail("FEATURE");
    }
    drv_nrf24l01_set_auto_ack(DRV_NRF24L01_PIPE0);
    if (drv_nrf24l01_set_auto_retransmit(NRF24_PAIR_RETRANSMIT_DELAY_CODE,
                                         NRF24_PAIR_RETRANSMIT_COUNT_CODE) != STC8H_OK) {
        return print_config_fail("SETUP_RETR");
    }
    if (drv_nrf24l01_set_rate_power(DRV_NRF24L01_RATE_250KBPS,
                                    DRV_NRF24L01_POWER_0DBM) != STC8H_OK) {
        return print_config_fail("RF_SETUP");
    }
    stc8h_uart_write_code(STC8H_UART1, "CONFIG: PASS");
    print_hex_field("STATUS", drv_nrf24l01_read_status());
    uart_crlf();
    return STC8H_OK;
}

static void init_platform(void)
{
    drv_nrf24l01_init_pins();
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_spi_init();
    print_common_banner();
}

#if NRF24_PAIR_DIAG_PTX
static void run_ptx(void)
{
    stc8h_u8 seq;
    stc8h_u8 status;
    stc8h_u8 width;
    stc8h_u16 tx_count;
    stc8h_u16 wait;

    if (configure_radio_common() != STC8H_OK) {
        while (1) {
            stc8h_delay_ms(1000u);
        }
    }
    drv_nrf24l01_enter_tx();
    seq = 0u;
    tx_count = 0u;

    while (1) {
        ++seq;
        ++tx_count;
        fill_payload('P', 'T', seq, tx_count);
        drv_nrf24l01_flush_tx();
        drv_nrf24l01_flush_rx();
        drv_nrf24l01_clear_irq(0x70u);
        (void)drv_nrf24l01_write_payload(payload, NRF24_PAIR_PAYLOAD_SIZE);
        drv_nrf24l01_pulse_ce();

        status = 0u;
        for (wait = 0u; wait < NRF24_PAIR_TX_WAIT_LIMIT; ++wait) {
            status = drv_nrf24l01_read_status();
            if ((status & (DRV_NRF24L01_STATUS_TX_DONE | DRV_NRF24L01_STATUS_MAX_RETRY)) != 0u) {
                break;
            }
        }

        stc8h_uart_write_code(STC8H_UART1, "PTX");
        print_hex16_field("cnt", tx_count);
        print_hex_field("seq", seq);
        print_hex_field("STATUS", status);
        print_hex_field("OBS", drv_nrf24l01_read_observe_tx());
        print_hex_field("FIFO", drv_nrf24l01_read_fifo_status());

        if ((status & DRV_NRF24L01_STATUS_TX_DONE) != 0u) {
            stc8h_uart_write_code(STC8H_UART1, " TX_DONE");
            if ((status & DRV_NRF24L01_STATUS_RX_READY) != 0u) {
                width = drv_nrf24l01_read_dynamic_payload_size();
                print_hex_field("ACK_LEN", width);
                if ((width > 0u) && (width <= NRF24_PAIR_PAYLOAD_SIZE)) {
                    (void)drv_nrf24l01_read_payload(ack_payload, width);
                    print_payload_head(ack_payload, width);
                } else {
                    drv_nrf24l01_flush_rx();
                }
            } else {
                stc8h_uart_write_code(STC8H_UART1, " ACK_EMPTY");
            }
        } else if ((status & DRV_NRF24L01_STATUS_MAX_RETRY) != 0u) {
            stc8h_uart_write_code(STC8H_UART1, " MAX_RETRY");
            drv_nrf24l01_flush_tx();
        } else {
            stc8h_uart_write_code(STC8H_UART1, " TIMEOUT");
            drv_nrf24l01_flush_tx();
        }

        drv_nrf24l01_clear_irq(status);
        uart_crlf();
        stc8h_delay_ms(250u);
    }
}
#endif

#if NRF24_PAIR_DIAG_PRX
static void load_ack_payload(stc8h_u8 last_seq, stc8h_u16 rx_count)
{
    fill_ack_payload(last_seq, rx_count);
    drv_nrf24l01_flush_tx();
    (void)drv_nrf24l01_write_ack_payload(0u, ack_payload, NRF24_PAIR_PAYLOAD_SIZE);
}

static void run_prx(void)
{
    stc8h_u8 status;
    stc8h_u8 width;
    stc8h_u8 last_seq;
    stc8h_u8 idle_ticks;
    stc8h_u16 rx_count;

    if (configure_radio_common() != STC8H_OK) {
        while (1) {
            stc8h_delay_ms(1000u);
        }
    }

    rx_count = 0u;
    last_seq = 0u;
    idle_ticks = 0u;
    drv_nrf24l01_enter_rx();
    load_ack_payload(last_seq, rx_count);

    while (1) {
        status = drv_nrf24l01_read_status();
        if ((status & DRV_NRF24L01_STATUS_RX_READY) != 0u) {
            width = drv_nrf24l01_read_dynamic_payload_size();
            if ((width > 0u) && (width <= NRF24_PAIR_PAYLOAD_SIZE)) {
                (void)drv_nrf24l01_read_payload(payload, width);
                ++rx_count;
                last_seq = payload[2];
                stc8h_uart_write_code(STC8H_UART1, "PRX RX");
                print_hex16_field("cnt", rx_count);
                print_hex_field("seq", last_seq);
                print_hex_field("LEN", width);
                print_hex_field("STATUS", status);
                print_hex_field("FIFO", drv_nrf24l01_read_fifo_status());
                print_payload_head(payload, width);
                uart_crlf();
                load_ack_payload(last_seq, rx_count);
            } else {
                stc8h_uart_write_code(STC8H_UART1, "PRX BAD_WIDTH");
                print_hex_field("LEN", width);
                print_hex_field("STATUS", status);
                uart_crlf();
                drv_nrf24l01_flush_rx();
            }
            drv_nrf24l01_clear_irq(status);
            idle_ticks = 0u;
        } else {
            ++idle_ticks;
            if (idle_ticks >= 100u) {
                idle_ticks = 0u;
                stc8h_uart_write_code(STC8H_UART1, "PRX idle");
                print_hex16_field("cnt", rx_count);
                print_hex_field("STATUS", status);
                print_hex_field("FIFO", drv_nrf24l01_read_fifo_status());
                uart_crlf();
            }
            stc8h_delay_ms(10u);
        }
    }
}
#endif

void main(void)
{
    init_platform();
#if NRF24_PAIR_DIAG_PTX
    run_ptx();
#else
    run_prx();
#endif
}
