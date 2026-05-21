#include "drv_nrf24l01.h"
#include "stc8h_delay.h"
#include "stc8h_spi.h"
#include "stc8h_uart.h"
#include "nrf24_pair_diag_logic.h"

#if defined(NRF24_PAIR_DIAG_PTX) && defined(NRF24_PAIR_DIAG_PRX)
#error "Select only one nRF24 pair diagnostic role."
#endif

#if !defined(NRF24_PAIR_DIAG_PTX) && !defined(NRF24_PAIR_DIAG_PRX)
#define NRF24_PAIR_DIAG_PTX 1
#endif

#ifndef NRF24_PAIR_MATRIX_DIAG
#define NRF24_PAIR_MATRIX_DIAG 0
#endif

#define NRF24_PAIR_RATE_250KBPS NRF24_PAIR_DIAG_RATE_250KBPS
#define NRF24_PAIR_RATE_1MBPS NRF24_PAIR_DIAG_RATE_1MBPS
#define NRF24_PAIR_RATE_2MBPS NRF24_PAIR_DIAG_RATE_2MBPS

#define NRF24_PAIR_POWER_NEG18DBM 0u
#define NRF24_PAIR_POWER_NEG12DBM 1u
#define NRF24_PAIR_POWER_NEG6DBM 2u
#define NRF24_PAIR_POWER_0DBM 3u

#ifndef NRF24_PAIR_CHANNEL
#define NRF24_PAIR_CHANNEL 76u
#endif

#ifndef NRF24_PAIR_DATA_RATE
#define NRF24_PAIR_DATA_RATE NRF24_PAIR_RATE_1MBPS
#endif

#ifndef NRF24_PAIR_RF_POWER
#define NRF24_PAIR_RF_POWER NRF24_PAIR_POWER_0DBM
#endif

#ifndef NRF24_PAIR_PAYLOAD_SIZE
#define NRF24_PAIR_PAYLOAD_SIZE 15u
#endif

#ifndef NRF24_PAIR_ACK_PAYLOAD
#define NRF24_PAIR_ACK_PAYLOAD 1
#endif

#ifndef NRF24_PAIR_DYNAMIC_PAYLOAD
#define NRF24_PAIR_DYNAMIC_PAYLOAD NRF24_PAIR_ACK_PAYLOAD
#endif

#ifndef NRF24_PAIR_AUTO_ACK
#define NRF24_PAIR_AUTO_ACK 1
#endif

#ifndef NRF24_PAIR_SUMMARY_INTERVAL
#if NRF24_PAIR_MATRIX_DIAG
#define NRF24_PAIR_SUMMARY_INTERVAL 1000u
#else
#define NRF24_PAIR_SUMMARY_INTERVAL 100u
#endif
#endif

#ifndef NRF24_PAIR_LOG_EACH_PACKET
#define NRF24_PAIR_LOG_EACH_PACKET 0
#endif

#ifndef NRF24_PAIR_SEND_PERIOD_MS
#if NRF24_PAIR_MATRIX_DIAG
#define NRF24_PAIR_SEND_PERIOD_MS 5u
#else
#define NRF24_PAIR_SEND_PERIOD_MS 50u
#endif
#endif

#ifndef NRF24_PAIR_TX_WAIT_LIMIT
#if NRF24_PAIR_MATRIX_DIAG
#define NRF24_PAIR_TX_WAIT_LIMIT 30000u
#else
#define NRF24_PAIR_TX_WAIT_LIMIT 12000u
#endif
#endif

#if (NRF24_PAIR_CHANNEL > 125u)
#error "NRF24_PAIR_CHANNEL must be 0..125."
#endif

#if (NRF24_PAIR_PAYLOAD_SIZE < 8u) || (NRF24_PAIR_PAYLOAD_SIZE > DRV_NRF24L01_PAYLOAD_MAX)
#error "NRF24_PAIR_PAYLOAD_SIZE must be 8..32 for this diagnostic payload format."
#endif

#if NRF24_PAIR_ACK_PAYLOAD && !NRF24_PAIR_DYNAMIC_PAYLOAD
#error "ACK payload requires dynamic payload on nRF24L01+."
#endif

#if NRF24_PAIR_ACK_PAYLOAD && !NRF24_PAIR_AUTO_ACK
#error "ACK payload requires auto-ack."
#endif

#if NRF24_PAIR_MATRIX_DIAG && !NRF24_PAIR_ACK_PAYLOAD
#error "NRF24_PAIR_MATRIX_DIAG requires compile-time ACK payload support; no-ACK stages disable it at runtime."
#endif

#if NRF24_PAIR_SUMMARY_INTERVAL == 0u
#error "NRF24_PAIR_SUMMARY_INTERVAL must be non-zero."
#endif

#if NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_250KBPS
#define NRF24_PAIR_RATE_VALUE DRV_NRF24L01_RATE_250KBPS
#define NRF24_PAIR_RATE_TEXT "250kbps"
#elif NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_1MBPS
#define NRF24_PAIR_RATE_VALUE DRV_NRF24L01_RATE_1MBPS
#define NRF24_PAIR_RATE_TEXT "1Mbps"
#elif NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_2MBPS
#define NRF24_PAIR_RATE_VALUE DRV_NRF24L01_RATE_2MBPS
#define NRF24_PAIR_RATE_TEXT "2Mbps"
#else
#error "NRF24_PAIR_DATA_RATE must be NRF24_PAIR_RATE_250KBPS, NRF24_PAIR_RATE_1MBPS, or NRF24_PAIR_RATE_2MBPS."
#endif

#if NRF24_PAIR_RF_POWER == NRF24_PAIR_POWER_NEG18DBM
#define NRF24_PAIR_POWER_VALUE DRV_NRF24L01_POWER_NEG18DBM
#define NRF24_PAIR_POWER_TEXT "-18dBm"
#elif NRF24_PAIR_RF_POWER == NRF24_PAIR_POWER_NEG12DBM
#define NRF24_PAIR_POWER_VALUE DRV_NRF24L01_POWER_NEG12DBM
#define NRF24_PAIR_POWER_TEXT "-12dBm"
#elif NRF24_PAIR_RF_POWER == NRF24_PAIR_POWER_NEG6DBM
#define NRF24_PAIR_POWER_VALUE DRV_NRF24L01_POWER_NEG6DBM
#define NRF24_PAIR_POWER_TEXT "-6dBm"
#elif NRF24_PAIR_RF_POWER == NRF24_PAIR_POWER_0DBM
#define NRF24_PAIR_POWER_VALUE DRV_NRF24L01_POWER_0DBM
#define NRF24_PAIR_POWER_TEXT "0dBm"
#else
#error "NRF24_PAIR_RF_POWER must be 0..3."
#endif

#ifndef NRF24_PAIR_RETRANSMIT_DELAY_CODE
#if (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_250KBPS) && NRF24_PAIR_ACK_PAYLOAD && (NRF24_PAIR_PAYLOAD_SIZE >= 24u)
#define NRF24_PAIR_RETRANSMIT_DELAY_CODE 5u
#elif (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_250KBPS) && NRF24_PAIR_ACK_PAYLOAD && (NRF24_PAIR_PAYLOAD_SIZE >= 16u)
#define NRF24_PAIR_RETRANSMIT_DELAY_CODE 4u
#elif (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_250KBPS) && NRF24_PAIR_ACK_PAYLOAD
#define NRF24_PAIR_RETRANSMIT_DELAY_CODE 3u
#else
#define NRF24_PAIR_RETRANSMIT_DELAY_CODE 1u
#endif
#endif

#ifndef NRF24_PAIR_RETRANSMIT_COUNT_CODE
#define NRF24_PAIR_RETRANSMIT_COUNT_CODE 15u
#endif

#if (NRF24_PAIR_RETRANSMIT_DELAY_CODE > 15u) || (NRF24_PAIR_RETRANSMIT_COUNT_CODE > 15u)
#error "NRF24_PAIR_RETRANSMIT_DELAY_CODE and NRF24_PAIR_RETRANSMIT_COUNT_CODE must be 0..15."
#endif

#if (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_250KBPS) && (NRF24_PAIR_RETRANSMIT_DELAY_CODE < 1u)
#error "Nordic nRF24L01+ requires ARD >= 500us in 250kbps mode."
#endif

#if NRF24_PAIR_ACK_PAYLOAD && (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_250KBPS) && (NRF24_PAIR_PAYLOAD_SIZE >= 24u) && (NRF24_PAIR_RETRANSMIT_DELAY_CODE < 5u)
#error "Nordic nRF24L01+ requires ARD >= 1500us for 250kbps ACK payloads up to 32 bytes."
#endif

#if NRF24_PAIR_ACK_PAYLOAD && (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_250KBPS) && (NRF24_PAIR_PAYLOAD_SIZE >= 16u) && (NRF24_PAIR_PAYLOAD_SIZE < 24u) && (NRF24_PAIR_RETRANSMIT_DELAY_CODE < 4u)
#error "Nordic nRF24L01+ requires ARD >= 1250us for 250kbps ACK payloads below 24 bytes."
#endif

#if NRF24_PAIR_ACK_PAYLOAD && (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_250KBPS) && (NRF24_PAIR_PAYLOAD_SIZE < 16u) && (NRF24_PAIR_RETRANSMIT_DELAY_CODE < 3u)
#error "Nordic nRF24L01+ requires ARD >= 1000us for 250kbps ACK payloads below 16 bytes."
#endif

#if NRF24_PAIR_ACK_PAYLOAD && (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_1MBPS) && (NRF24_PAIR_PAYLOAD_SIZE > 5u) && (NRF24_PAIR_RETRANSMIT_DELAY_CODE < 1u)
#error "Nordic nRF24L01+ requires ARD >= 500us for 1Mbps ACK payloads above 5 bytes."
#endif

#if NRF24_PAIR_ACK_PAYLOAD && (NRF24_PAIR_DATA_RATE == NRF24_PAIR_RATE_2MBPS) && (NRF24_PAIR_PAYLOAD_SIZE > 15u) && (NRF24_PAIR_RETRANSMIT_DELAY_CODE < 1u)
#error "Nordic nRF24L01+ requires ARD >= 500us for 2Mbps ACK payloads above 15 bytes."
#endif

static const STC8H_CODE char hex_chars[] = "0123456789ABCDEF";
static STC8H_CODE stc8h_u8 pair_address[5] = {'T', 'O', 'Y', 'R', '1'};

static STC8H_XDATA stc8h_u8 payload[DRV_NRF24L01_PAYLOAD_MAX];
static STC8H_XDATA stc8h_u8 ack_payload[DRV_NRF24L01_PAYLOAD_MAX];

static void uart_crlf(void)
{
    stc8h_uart_write_code(STC8H_UART1, "\r\n");
}

static void uart_put_hex8(stc8h_u8 value)
{
    stc8h_uart_putc(STC8H_UART1, hex_chars[(value >> 4) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex_chars[value & 0x0Fu]);
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

static void print_u16_field(const STC8H_CODE char *name, stc8h_u16 value)
{
    stc8h_uart_write_code(STC8H_UART1, " ");
    stc8h_uart_write_code(STC8H_UART1, name);
    stc8h_uart_write_code(STC8H_UART1, "=");
    uart_put_dec16(value);
}

static void print_hex_field(const STC8H_CODE char *name, stc8h_u8 value)
{
    stc8h_uart_write_code(STC8H_UART1, " ");
    stc8h_uart_write_code(STC8H_UART1, name);
    stc8h_uart_write_code(STC8H_UART1, "=0x");
    uart_put_hex8(value);
}

#if NRF24_PAIR_LOG_EACH_PACKET
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
#endif

static void fill_payload_len(stc8h_u8 seq, stc8h_u16 count, stc8h_u8 len)
{
    stc8h_u8 i;

    payload[0] = 'P';
    payload[1] = 'T';
    payload[2] = seq;
    payload[3] = (stc8h_u8)count;
    payload[4] = (stc8h_u8)(count >> 8);
    for (i = 5u; i < len; ++i) {
        payload[i] = (stc8h_u8)(seq + i);
    }
}

static void fill_payload(stc8h_u8 seq, stc8h_u16 count)
{
    fill_payload_len(seq, count, NRF24_PAIR_PAYLOAD_SIZE);
}

#if NRF24_PAIR_ACK_PAYLOAD
static void fill_ack_payload_len(stc8h_u8 last_seq, stc8h_u16 rx_count, stc8h_u8 len)
{
    stc8h_u8 i;

    ack_payload[0] = 'A';
    ack_payload[1] = 'C';
    ack_payload[2] = 'K';
    ack_payload[3] = last_seq;
    ack_payload[4] = (stc8h_u8)rx_count;
    ack_payload[5] = (stc8h_u8)(rx_count >> 8);
    for (i = 6u; i < len; ++i) {
        ack_payload[i] = (stc8h_u8)(0xA0u + i);
    }
}

static void fill_ack_payload(stc8h_u8 last_seq, stc8h_u16 rx_count)
{
    fill_ack_payload_len(last_seq, rx_count, NRF24_PAIR_PAYLOAD_SIZE);
}
#endif

static void print_common_banner(void)
{
    stc8h_uart_write_code(STC8H_UART1, "\r\nSTC8H nRF24 pair diag ");
#if NRF24_PAIR_DIAG_PTX
    stc8h_uart_write_code(STC8H_UART1, "PTX\r\n");
#else
    stc8h_uart_write_code(STC8H_UART1, "PRX\r\n");
#endif
    stc8h_uart_write_code(STC8H_UART1, "pins CE=P1.6 CSN=P1.2 SCK=P1.5 MOSI=P1.3 MISO=P1.4 IRQ=P3.2\r\n");
#if NRF24_PAIR_MATRIX_DIAG
    stc8h_uart_write_code(STC8H_UART1, "matrix=fast stages=");
    uart_put_dec16(nrf24_pair_diag_matrix_stage_count());
    stc8h_uart_write_code(STC8H_UART1, " period_ms=");
    uart_put_dec16(NRF24_PAIR_SEND_PERIOD_MS);
    stc8h_uart_write_code(STC8H_UART1, " summary=");
    uart_put_dec16(NRF24_PAIR_SUMMARY_INTERVAL);
    uart_crlf();
#else
    stc8h_uart_write_code(STC8H_UART1, "channel=");
    uart_put_dec16(NRF24_PAIR_CHANNEL);
    stc8h_uart_write_code(STC8H_UART1, " addr=TOYR1 rate=");
    stc8h_uart_write_code(STC8H_UART1, NRF24_PAIR_RATE_TEXT);
    stc8h_uart_write_code(STC8H_UART1, " power=");
    stc8h_uart_write_code(STC8H_UART1, NRF24_PAIR_POWER_TEXT);
    stc8h_uart_write_code(STC8H_UART1, " payload=");
    uart_put_dec16(NRF24_PAIR_PAYLOAD_SIZE);
    stc8h_uart_write_code(STC8H_UART1, " auto_ack=");
    uart_put_dec16(NRF24_PAIR_AUTO_ACK);
    stc8h_uart_write_code(STC8H_UART1, " dpl=");
    uart_put_dec16(NRF24_PAIR_DYNAMIC_PAYLOAD);
    stc8h_uart_write_code(STC8H_UART1, " ack_payload=");
    uart_put_dec16(NRF24_PAIR_ACK_PAYLOAD);
    stc8h_uart_write_code(STC8H_UART1, " ard_us=");
    uart_put_dec16((stc8h_u16)((NRF24_PAIR_RETRANSMIT_DELAY_CODE + 1u) * 250u));
    stc8h_uart_write_code(STC8H_UART1, " arc=");
    uart_put_dec16(NRF24_PAIR_RETRANSMIT_COUNT_CODE);
    uart_crlf();
#endif
}

static stc8h_status_t print_config_fail(const STC8H_CODE char *step)
{
    stc8h_uart_write_code(STC8H_UART1, "CONFIG FAIL ");
    stc8h_uart_write_code(STC8H_UART1, step);
    print_hex_field("STATUS", drv_nrf24l01_read_status());
    print_hex_field("FIFO", drv_nrf24l01_read_fifo_status());
    uart_crlf();
    return STC8H_ERROR;
}

static stc8h_status_t configure_radio_common(void)
{
    drv_nrf24l01_power_down();
    drv_nrf24l01_flush_tx();
    drv_nrf24l01_flush_rx();
    drv_nrf24l01_clear_irq(DRV_NRF24L01_IRQ_MASK);

    if (drv_nrf24l01_check_present() != STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "CHECK FAIL");
        print_hex_field("STATUS", drv_nrf24l01_read_status());
        uart_crlf();
        return STC8H_ERROR;
    }
    stc8h_uart_write_code(STC8H_UART1, "CHECK PASS");
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

    drv_nrf24l01_set_rx_pipes(DRV_NRF24L01_PIPE0);
#if NRF24_PAIR_AUTO_ACK
    drv_nrf24l01_set_auto_ack(DRV_NRF24L01_PIPE0);
#else
    drv_nrf24l01_set_auto_ack(0u);
#endif

#if NRF24_PAIR_ACK_PAYLOAD
    if (drv_nrf24l01_enable_ack_payload(DRV_NRF24L01_PIPE0) != STC8H_OK) {
        return print_config_fail("FEATURE_ACK");
    }
#elif NRF24_PAIR_DYNAMIC_PAYLOAD
    if (drv_nrf24l01_enable_dynamic_payload(DRV_NRF24L01_PIPE0) != STC8H_OK) {
        return print_config_fail("FEATURE_DPL");
    }
#else
    drv_nrf24l01_disable_dynamic_payload();
    drv_nrf24l01_disable_ack_payload();
#endif

    if (drv_nrf24l01_set_auto_retransmit(NRF24_PAIR_RETRANSMIT_DELAY_CODE,
                                         NRF24_PAIR_RETRANSMIT_COUNT_CODE) != STC8H_OK) {
        return print_config_fail("SETUP_RETR");
    }
    if (drv_nrf24l01_set_rate_power(NRF24_PAIR_RATE_VALUE, NRF24_PAIR_POWER_VALUE) != STC8H_OK) {
        return print_config_fail("RF_SETUP");
    }

    stc8h_uart_write_code(STC8H_UART1, "CONFIG PASS");
    print_hex_field("STATUS", drv_nrf24l01_read_status());
    print_hex_field("FIFO", drv_nrf24l01_read_fifo_status());
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

#if NRF24_PAIR_MATRIX_DIAG

#define NRF24_PAIR_MATRIX_SWITCH_RETRIES 20u
#define NRF24_PAIR_MATRIX_DONE_STAGE 0xFFu

#if NRF24_PAIR_DIAG_PTX
typedef struct {
    stc8h_u16 tx_count;
    stc8h_u16 tx_ok;
    stc8h_u16 max_rt;
    stc8h_u16 ack_ok;
    stc8h_u16 ack_empty;
    stc8h_u16 ack_bad;
    stc8h_u8 status;
} nrf24_pair_diag_ptx_stage_stats_t;
#endif

#if NRF24_PAIR_DIAG_PRX
typedef struct {
    stc8h_u16 rx_count;
    stc8h_u16 bad_width;
    stc8h_u16 ack_load;
    stc8h_u16 ack_busy;
    stc8h_u16 ack_fail;
    nrf24_pair_diag_rx_stats_t stats;
} nrf24_pair_diag_prx_stage_stats_t;
#endif

static drv_nrf24l01_rate_t matrix_rate_value(stc8h_u8 rate)
{
    if (rate == NRF24_PAIR_DIAG_RATE_250KBPS) {
        return DRV_NRF24L01_RATE_250KBPS;
    }
    if (rate == NRF24_PAIR_DIAG_RATE_2MBPS) {
        return DRV_NRF24L01_RATE_2MBPS;
    }
    return DRV_NRF24L01_RATE_1MBPS;
}

static void matrix_print_rate(stc8h_u8 rate)
{
    if (rate == NRF24_PAIR_DIAG_RATE_250KBPS) {
        stc8h_uart_write_code(STC8H_UART1, "250kbps");
    } else if (rate == NRF24_PAIR_DIAG_RATE_2MBPS) {
        stc8h_uart_write_code(STC8H_UART1, "2Mbps");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "1Mbps");
    }
}

static void matrix_print_stage_begin(const STC8H_CODE char *role, stc8h_u8 stage_index,
                                     const nrf24_pair_diag_matrix_stage_t *stage)
{
    stc8h_uart_write_code(STC8H_UART1, "MATRIX_STAGE_BEGIN role=");
    stc8h_uart_write_code(STC8H_UART1, role);
    print_u16_field("stage", (stc8h_u16)(stage_index + 1u));
    print_u16_field("total", nrf24_pair_diag_matrix_stage_count());
    stc8h_uart_write_code(STC8H_UART1, " rate=");
    matrix_print_rate(stage->rate);
    print_u16_field("payload", stage->payload_size);
    stc8h_uart_write_code(STC8H_UART1, " auto_ack=1");
    print_u16_field("dpl", stage->dynamic_payload);
    print_u16_field("ack_payload", stage->ack_payload);
    print_u16_field("ard_us", (stc8h_u16)((stage->ard_code + 1u) * 250u));
    print_u16_field("arc", stage->arc_count);
    print_u16_field("packets", stage->packet_count);
    uart_crlf();
}

static stc8h_status_t matrix_configure_radio(stc8h_u8 stage_index,
                                             const nrf24_pair_diag_matrix_stage_t *stage)
{
    drv_nrf24l01_power_down();
    drv_nrf24l01_flush_tx();
    drv_nrf24l01_flush_rx();
    drv_nrf24l01_clear_irq(DRV_NRF24L01_IRQ_MASK);

    if (drv_nrf24l01_check_present() != STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "MATRIX_CHECK_FAIL");
        print_u16_field("stage", (stc8h_u16)(stage_index + 1u));
        print_hex_field("STATUS", drv_nrf24l01_read_status());
        uart_crlf();
        return STC8H_ERROR;
    }

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
    if (drv_nrf24l01_set_payload_size(0u, stage->payload_size) != STC8H_OK) {
        return print_config_fail("RX_PW_P0");
    }

    drv_nrf24l01_set_rx_pipes(DRV_NRF24L01_PIPE0);
    drv_nrf24l01_set_auto_ack(DRV_NRF24L01_PIPE0);

    if (stage->ack_payload != 0u) {
        if (drv_nrf24l01_enable_ack_payload(DRV_NRF24L01_PIPE0) != STC8H_OK) {
            return print_config_fail("FEATURE_ACK");
        }
    } else if (stage->dynamic_payload != 0u) {
        drv_nrf24l01_disable_ack_payload();
        if (drv_nrf24l01_enable_dynamic_payload(DRV_NRF24L01_PIPE0) != STC8H_OK) {
            return print_config_fail("FEATURE_DPL");
        }
    } else {
        drv_nrf24l01_disable_ack_payload();
        drv_nrf24l01_disable_dynamic_payload();
    }

    if (drv_nrf24l01_set_auto_retransmit(stage->ard_code, stage->arc_count) != STC8H_OK) {
        return print_config_fail("SETUP_RETR");
    }
    if (drv_nrf24l01_set_rate_power(matrix_rate_value(stage->rate), NRF24_PAIR_POWER_VALUE) != STC8H_OK) {
        return print_config_fail("RF_SETUP");
    }

    stc8h_uart_write_code(STC8H_UART1, "MATRIX_CONFIG_PASS");
    print_u16_field("stage", (stc8h_u16)(stage_index + 1u));
    print_hex_field("STATUS", drv_nrf24l01_read_status());
    print_hex_field("FIFO", drv_nrf24l01_read_fifo_status());
    uart_crlf();
    return STC8H_OK;
}

#if NRF24_PAIR_DIAG_PRX
static stc8h_status_t matrix_load_ack_payload(const nrf24_pair_diag_matrix_stage_t *stage,
                                              stc8h_u8 last_seq,
                                              stc8h_u16 rx_count,
                                              stc8h_u8 replace_pending)
{
    if (stage->ack_payload == 0u) {
        return STC8H_OK;
    }
    fill_ack_payload_len(last_seq, rx_count, stage->payload_size);
    return drv_nrf24l01_preload_ack_payload(0u, ack_payload, stage->payload_size, replace_pending);
}

static void matrix_record_ack_load(nrf24_pair_diag_prx_stage_stats_t *stats, stc8h_status_t status)
{
    if (status == STC8H_OK) {
        ++stats->ack_load;
    } else if (status == STC8H_BUSY) {
        ++stats->ack_busy;
    } else {
        ++stats->ack_fail;
    }
}

static stc8h_u16 matrix_payload_tx_count(void)
{
    return (stc8h_u16)(payload[3] | ((stc8h_u16)payload[4] << 8));
}
#endif

#if NRF24_PAIR_DIAG_PTX
static void matrix_fill_switch_payload(stc8h_u8 next_stage,
                                       const nrf24_pair_diag_matrix_stage_t *stage)
{
    stc8h_u8 i;

    payload[0] = 'S';
    payload[1] = 'W';
    payload[2] = next_stage;
    payload[3] = 0xA5u;
    payload[4] = 0x5Au;
    for (i = 5u; i < stage->payload_size; ++i) {
        payload[i] = (stc8h_u8)(0x50u + i);
    }
}
#endif

#if NRF24_PAIR_DIAG_PRX
static stc8h_u8 matrix_is_switch_payload(stc8h_u8 len)
{
    if (len < 5u) {
        return 0u;
    }
    if ((payload[0] != 'S') || (payload[1] != 'W') ||
        (payload[3] != 0xA5u) || (payload[4] != 0x5Au)) {
        return 0u;
    }
    return 1u;
}
#endif

#if NRF24_PAIR_DIAG_PTX
static drv_nrf24l01_tx_result_t matrix_send_packet(const nrf24_pair_diag_matrix_stage_t *stage,
                                                   stc8h_u8 is_switch,
                                                   stc8h_u8 seq,
                                                   stc8h_u16 count,
                                                   stc8h_u8 next_stage,
                                                   stc8h_u8 *status_out,
                                                   stc8h_u8 *ack_len_out)
{
    stc8h_u16 wait;
    stc8h_u8 status;

    if (is_switch != 0u) {
        matrix_fill_switch_payload(next_stage, stage);
    } else {
        fill_payload_len(seq, count, stage->payload_size);
    }

    drv_nrf24l01_flush_tx();
    if (stage->ack_payload != 0u) {
        drv_nrf24l01_flush_rx();
    }
    drv_nrf24l01_clear_irq(DRV_NRF24L01_IRQ_MASK);
    (void)drv_nrf24l01_write_payload(payload, stage->payload_size);
    drv_nrf24l01_pulse_ce();

    status = 0u;
    for (wait = 0u; wait < NRF24_PAIR_TX_WAIT_LIMIT; ++wait) {
        status = drv_nrf24l01_read_status();
        if ((status & (DRV_NRF24L01_STATUS_TX_DONE | DRV_NRF24L01_STATUS_MAX_RETRY)) != 0u) {
            break;
        }
    }

    *status_out = status;
    *ack_len_out = 0u;
    if (wait >= NRF24_PAIR_TX_WAIT_LIMIT) {
        drv_nrf24l01_recover(DRV_NRF24L01_RECOVER_PTX);
        return DRV_NRF24L01_TX_ACK_PAYLOAD_INVALID;
    }

    if (stage->ack_payload != 0u) {
        return drv_nrf24l01_complete_tx(status, ack_payload, ack_len_out, stage->payload_size);
    }
    return drv_nrf24l01_complete_tx(status, 0, 0, 0u);
}

static void matrix_count_ptx_result(nrf24_pair_diag_ptx_stage_stats_t *stats,
                                    const nrf24_pair_diag_matrix_stage_t *stage,
                                    drv_nrf24l01_tx_result_t result,
                                    stc8h_u8 ack_len)
{
    if (result == DRV_NRF24L01_TX_MAX_RT) {
        ++stats->max_rt;
    } else if (result == DRV_NRF24L01_TX_ACK_PAYLOAD_OK) {
        ++stats->tx_ok;
        ++stats->ack_ok;
    } else if (result == DRV_NRF24L01_TX_ACK_EMPTY) {
        ++stats->tx_ok;
        if (stage->ack_payload != 0u) {
            ++stats->ack_empty;
        }
    } else if (result == DRV_NRF24L01_TX_DONE) {
        ++stats->tx_ok;
    } else {
        ++stats->ack_bad;
    }
    (void)ack_len;
}

static void matrix_print_ptx_summary(const STC8H_CODE char *tag, stc8h_u8 stage_index,
                                     const nrf24_pair_diag_ptx_stage_stats_t *stats)
{
    stc8h_uart_write_code(STC8H_UART1, tag);
    print_u16_field("stage", (stc8h_u16)(stage_index + 1u));
    print_u16_field("tx_count", stats->tx_count);
    print_u16_field("tx_ok", stats->tx_ok);
    print_u16_field("max_rt", stats->max_rt);
    print_u16_field("ack_ok", stats->ack_ok);
    print_u16_field("ack_empty", stats->ack_empty);
    print_u16_field("ack_bad", stats->ack_bad);
    print_hex_field("OBSERVE_TX", drv_nrf24l01_read_observe_tx());
    print_hex_field("STATUS", stats->status);
    print_hex_field("FIFO_STATUS", drv_nrf24l01_read_fifo_status());
    uart_crlf();
}
#endif

#if NRF24_PAIR_DIAG_PRX
static void matrix_print_prx_summary(const STC8H_CODE char *tag, stc8h_u8 stage_index,
                                     const nrf24_pair_diag_prx_stage_stats_t *stage_stats)
{
    stc8h_uart_write_code(STC8H_UART1, tag);
    print_u16_field("stage", (stc8h_u16)(stage_index + 1u));
    print_u16_field("rx_count", stage_stats->rx_count);
    print_hex_field("seq", stage_stats->stats.last_seq);
    print_u16_field("lost", stage_stats->stats.lost_count);
    print_u16_field("dup", stage_stats->stats.dup_count);
    print_u16_field("ptx_reset", stage_stats->stats.ptx_reset_count);
    print_u16_field("bad_width", stage_stats->bad_width);
    print_hex_field("STATUS", drv_nrf24l01_read_status());
    print_hex_field("FIFO_STATUS", drv_nrf24l01_read_fifo_status());
    print_u16_field("ack_load", stage_stats->ack_load);
    print_u16_field("ack_busy", stage_stats->ack_busy);
    print_u16_field("ack_fail", stage_stats->ack_fail);
    uart_crlf();
}

static void matrix_prx_stats_init(nrf24_pair_diag_prx_stage_stats_t *stage_stats)
{
    stage_stats->rx_count = 0u;
    stage_stats->bad_width = 0u;
    stage_stats->ack_load = 0u;
    stage_stats->ack_busy = 0u;
    stage_stats->ack_fail = 0u;
    nrf24_pair_diag_rx_stats_init(&stage_stats->stats);
}
#endif

#if NRF24_PAIR_DIAG_PTX
static void run_matrix_ptx(void)
{
    stc8h_u8 stage_index;
    stc8h_u8 seq;
    stc8h_u8 ack_len;
    stc8h_u8 switch_ok;
    stc8h_u8 attempt;
    stc8h_u8 next_stage;
    nrf24_pair_diag_matrix_stage_t stage;
    nrf24_pair_diag_ptx_stage_stats_t stats;
    drv_nrf24l01_tx_result_t result;

    for (stage_index = 0u; stage_index < nrf24_pair_diag_matrix_stage_count(); ++stage_index) {
        (void)nrf24_pair_diag_matrix_stage_init(stage_index, &stage);
        matrix_print_stage_begin("PTX", stage_index, &stage);
        if (matrix_configure_radio(stage_index, &stage) != STC8H_OK) {
            while (1) {
                stc8h_delay_ms(1000u);
            }
        }
        drv_nrf24l01_enter_tx();

        seq = 0u;
        stats.tx_count = 0u;
        stats.tx_ok = 0u;
        stats.max_rt = 0u;
        stats.ack_ok = 0u;
        stats.ack_empty = 0u;
        stats.ack_bad = 0u;
        stats.status = 0u;

        while (stats.tx_count < stage.packet_count) {
            ++seq;
            ++stats.tx_count;
            result = matrix_send_packet(&stage, 0u, seq, stats.tx_count, 0u, &stats.status, &ack_len);
            matrix_count_ptx_result(&stats, &stage, result, ack_len);
            if ((stats.tx_count % NRF24_PAIR_SUMMARY_INTERVAL) == 0u) {
                matrix_print_ptx_summary("MATRIX_PTX_SUM", stage_index, &stats);
            }
            stc8h_delay_ms(NRF24_PAIR_SEND_PERIOD_MS);
        }
        matrix_print_ptx_summary("MATRIX_STAGE_END role=PTX", stage_index, &stats);

        if ((stc8h_u8)(stage_index + 1u) < nrf24_pair_diag_matrix_stage_count()) {
            next_stage = (stc8h_u8)(stage_index + 1u);
        } else {
            next_stage = NRF24_PAIR_MATRIX_DONE_STAGE;
        }

        switch_ok = 0u;
        for (attempt = 0u; attempt < NRF24_PAIR_MATRIX_SWITCH_RETRIES; ++attempt) {
            result = matrix_send_packet(&stage, 1u, 0u, 0u, next_stage, &stats.status, &ack_len);
            if ((result != DRV_NRF24L01_TX_MAX_RT) &&
                (result != DRV_NRF24L01_TX_ACK_PAYLOAD_INVALID)) {
                switch_ok = 1u;
                break;
            }
            stc8h_delay_ms(20u);
        }

        if (switch_ok == 0u) {
            attempt = (stc8h_u8)(NRF24_PAIR_MATRIX_SWITCH_RETRIES - 1u);
        }
        stc8h_uart_write_code(STC8H_UART1, "MATRIX_SWITCH role=PTX");
        print_u16_field("from", (stc8h_u16)(stage_index + 1u));
        if (next_stage == NRF24_PAIR_MATRIX_DONE_STAGE) {
            stc8h_uart_write_code(STC8H_UART1, " to=DONE");
        } else {
            print_u16_field("to", (stc8h_u16)(next_stage + 1u));
        }
        print_u16_field("ok", switch_ok);
        print_u16_field("attempts", (stc8h_u16)(attempt + 1u));
        uart_crlf();

        if (switch_ok == 0u) {
            while (1) {
                stc8h_delay_ms(1000u);
            }
        }

        if (next_stage == NRF24_PAIR_MATRIX_DONE_STAGE) {
            stc8h_uart_write_code(STC8H_UART1, "MATRIX_DONE role=PTX\r\n");
            while (1) {
                stc8h_delay_ms(1000u);
            }
        }
        stc8h_delay_ms(100u);
    }
}
#endif

#if NRF24_PAIR_DIAG_PRX
static void run_matrix_prx(void)
{
    stc8h_u8 stage_index;
    stc8h_u8 status;
    stc8h_u8 len;
    stc8h_u8 next_stage;
    nrf24_pair_diag_matrix_stage_t stage;
    nrf24_pair_diag_prx_stage_stats_t stage_stats;
    stc8h_status_t rx_status;
    stc8h_status_t ack_status;

    stage_index = 0u;
    while (1) {
        (void)nrf24_pair_diag_matrix_stage_init(stage_index, &stage);
        matrix_print_stage_begin("PRX", stage_index, &stage);
        if (matrix_configure_radio(stage_index, &stage) != STC8H_OK) {
            while (1) {
                stc8h_delay_ms(1000u);
            }
        }
        matrix_prx_stats_init(&stage_stats);
        drv_nrf24l01_enter_rx();
        if (stage.ack_payload != 0u) {
            ack_status = matrix_load_ack_payload(&stage, stage_stats.stats.last_seq,
                                                 stage_stats.rx_count,
                                                 nrf24_pair_diag_ack_replace_on_recover());
            matrix_record_ack_load(&stage_stats, ack_status);
        }

        while (1) {
            if (stage.dynamic_payload != 0u) {
                rx_status = drv_nrf24l01_read_rx_packet(payload, &len, stage.payload_size);
                status = drv_nrf24l01_read_status();
            } else {
                status = drv_nrf24l01_read_status();
                if ((status & DRV_NRF24L01_STATUS_RX_READY) != 0u) {
                    (void)drv_nrf24l01_read_payload(payload, stage.payload_size);
                    drv_nrf24l01_clear_irq(status);
                    len = stage.payload_size;
                    rx_status = STC8H_OK;
                } else {
                    len = 0u;
                    rx_status = STC8H_BUSY;
                }
            }

            if (rx_status == STC8H_OK) {
                if (matrix_is_switch_payload(len) != 0u) {
                    matrix_print_prx_summary("MATRIX_STAGE_END role=PRX", stage_index, &stage_stats);
                    next_stage = payload[2];
                    stc8h_uart_write_code(STC8H_UART1, "MATRIX_SWITCH role=PRX");
                    print_u16_field("from", (stc8h_u16)(stage_index + 1u));
                    if (next_stage == NRF24_PAIR_MATRIX_DONE_STAGE) {
                        stc8h_uart_write_code(STC8H_UART1, " to=DONE");
                    } else {
                        print_u16_field("to", (stc8h_u16)(next_stage + 1u));
                    }
                    print_hex_field("STATUS", status);
                    uart_crlf();
                    if (next_stage == NRF24_PAIR_MATRIX_DONE_STAGE) {
                        stc8h_uart_write_code(STC8H_UART1, "MATRIX_DONE role=PRX\r\n");
                        while (1) {
                            stc8h_delay_ms(1000u);
                        }
                    }
                    if (next_stage < nrf24_pair_diag_matrix_stage_count()) {
                        stage_index = next_stage;
                        stc8h_delay_ms(100u);
                        break;
                    }
                } else {
                    ++stage_stats.rx_count;
                    nrf24_pair_diag_rx_stats_update(&stage_stats.stats, payload[2],
                                                    matrix_payload_tx_count());
                    if (stage.ack_payload != 0u) {
                        ack_status = matrix_load_ack_payload(&stage, stage_stats.stats.last_seq,
                                                             stage_stats.rx_count,
                                                             nrf24_pair_diag_ack_replace_after_rx());
                        matrix_record_ack_load(&stage_stats, ack_status);
                    }
                    if ((stage_stats.rx_count % NRF24_PAIR_SUMMARY_INTERVAL) == 0u) {
                        matrix_print_prx_summary("MATRIX_PRX_SUM", stage_index, &stage_stats);
                    }
                }
            } else if (rx_status == STC8H_ERROR) {
                ++stage_stats.bad_width;
                if ((stage_stats.bad_width % NRF24_PAIR_SUMMARY_INTERVAL) == 0u) {
                    matrix_print_prx_summary("MATRIX_PRX_SUM", stage_index, &stage_stats);
                }
            } else {
                stc8h_delay_ms(2u);
            }
        }
    }
}
#endif
#endif

#if NRF24_PAIR_DIAG_PTX && !NRF24_PAIR_MATRIX_DIAG
static void print_ptx_summary(stc8h_u16 tx_count, stc8h_u16 tx_ok, stc8h_u16 max_rt,
                              stc8h_u16 ack_ok, stc8h_u16 ack_empty, stc8h_u16 ack_bad,
                              stc8h_u8 status)
{
    stc8h_uart_write_code(STC8H_UART1, "PTX_SUM");
    print_u16_field("tx_count", tx_count);
    print_u16_field("tx_ok", tx_ok);
    print_u16_field("max_rt", max_rt);
    print_u16_field("ack_ok", ack_ok);
    print_u16_field("ack_empty", ack_empty);
    print_u16_field("ack_bad", ack_bad);
    print_hex_field("OBSERVE_TX", drv_nrf24l01_read_observe_tx());
    print_hex_field("STATUS", status);
    print_hex_field("FIFO_STATUS", drv_nrf24l01_read_fifo_status());
    uart_crlf();
}

#if NRF24_PAIR_ACK_PAYLOAD
static stc8h_u8 ack_payload_valid(stc8h_u8 len)
{
    if (len != NRF24_PAIR_PAYLOAD_SIZE) {
        return 0u;
    }
    if ((ack_payload[0] != 'A') || (ack_payload[1] != 'C') || (ack_payload[2] != 'K')) {
        return 0u;
    }
    return 1u;
}
#endif

static void run_ptx(void)
{
    stc8h_u8 seq;
    stc8h_u8 status;
    stc8h_u8 ack_len;
    stc8h_u16 wait;
    stc8h_u16 tx_count;
    stc8h_u16 tx_ok;
    stc8h_u16 max_rt;
    stc8h_u16 ack_ok;
    stc8h_u16 ack_empty;
    stc8h_u16 ack_bad;
    drv_nrf24l01_tx_result_t result;

    if (configure_radio_common() != STC8H_OK) {
        while (1) {
            stc8h_delay_ms(1000u);
        }
    }
    drv_nrf24l01_enter_tx();

    seq = 0u;
    tx_count = 0u;
    tx_ok = 0u;
    max_rt = 0u;
    ack_ok = 0u;
    ack_empty = 0u;
    ack_bad = 0u;
    status = 0u;

    while (1) {
        ++seq;
        ++tx_count;
        fill_payload(seq, tx_count);

        drv_nrf24l01_flush_tx();
#if NRF24_PAIR_ACK_PAYLOAD
        drv_nrf24l01_flush_rx();
#endif
        drv_nrf24l01_clear_irq(DRV_NRF24L01_IRQ_MASK);
        (void)drv_nrf24l01_write_payload(payload, NRF24_PAIR_PAYLOAD_SIZE);
        drv_nrf24l01_pulse_ce();

        status = 0u;
        for (wait = 0u; wait < NRF24_PAIR_TX_WAIT_LIMIT; ++wait) {
            status = drv_nrf24l01_read_status();
            if ((status & (DRV_NRF24L01_STATUS_TX_DONE | DRV_NRF24L01_STATUS_MAX_RETRY)) != 0u) {
                break;
            }
        }

        if (wait >= NRF24_PAIR_TX_WAIT_LIMIT) {
            ++ack_bad;
            drv_nrf24l01_recover(DRV_NRF24L01_RECOVER_PTX);
        } else {
#if NRF24_PAIR_ACK_PAYLOAD
            result = drv_nrf24l01_complete_tx(status, ack_payload, &ack_len, NRF24_PAIR_PAYLOAD_SIZE);
#else
            ack_len = 0u;
            result = drv_nrf24l01_complete_tx(status, 0, 0, 0u);
#endif
            if (result == DRV_NRF24L01_TX_MAX_RT) {
                ++max_rt;
#if NRF24_PAIR_ACK_PAYLOAD
            } else if (result == DRV_NRF24L01_TX_ACK_PAYLOAD_OK) {
                ++tx_ok;
                if (ack_payload_valid(ack_len) != 0u) {
                    ++ack_ok;
                } else {
                    ++ack_bad;
                }
            } else if (result == DRV_NRF24L01_TX_ACK_EMPTY) {
                ++tx_ok;
                ++ack_empty;
#endif
            } else if (result == DRV_NRF24L01_TX_DONE) {
                ++tx_ok;
            } else if (result == DRV_NRF24L01_TX_ACK_PAYLOAD_INVALID) {
                ++tx_ok;
                ++ack_bad;
            } else {
                ++ack_bad;
            }
        }

#if NRF24_PAIR_LOG_EACH_PACKET
        stc8h_uart_write_code(STC8H_UART1, "PTX_PACKET");
        print_u16_field("tx_count", tx_count);
        print_hex_field("seq", seq);
        print_hex_field("STATUS", status);
        print_hex_field("FIFO", drv_nrf24l01_read_fifo_status());
#if NRF24_PAIR_ACK_PAYLOAD
        print_hex_field("ACK_LEN", ack_len);
        if (ack_len != 0u) {
            print_payload_head(ack_payload, ack_len);
        }
#endif
        uart_crlf();
#endif

        if ((tx_count % NRF24_PAIR_SUMMARY_INTERVAL) == 0u) {
            print_ptx_summary(tx_count, tx_ok, max_rt, ack_ok, ack_empty, ack_bad, status);
        }
        stc8h_delay_ms(NRF24_PAIR_SEND_PERIOD_MS);
    }
}
#endif

#if NRF24_PAIR_DIAG_PRX && !NRF24_PAIR_MATRIX_DIAG
static stc8h_status_t load_ack_payload(stc8h_u8 last_seq, stc8h_u16 rx_count, stc8h_u8 replace_pending)
{
#if NRF24_PAIR_ACK_PAYLOAD
    fill_ack_payload(last_seq, rx_count);
    return drv_nrf24l01_preload_ack_payload(0u, ack_payload, NRF24_PAIR_PAYLOAD_SIZE, replace_pending);
#else
    (void)last_seq;
    (void)rx_count;
    (void)replace_pending;
    return STC8H_OK;
#endif
}

static stc8h_u16 payload_tx_count(void)
{
    return (stc8h_u16)(payload[3] | ((stc8h_u16)payload[4] << 8));
}

static void print_prx_summary(stc8h_u16 rx_count, const nrf24_pair_diag_rx_stats_t *stats,
                              stc8h_u16 bad_width, stc8h_u16 ack_load,
                              stc8h_u16 ack_busy, stc8h_u16 ack_fail)
{
    stc8h_uart_write_code(STC8H_UART1, "PRX_SUM");
    print_u16_field("rx_count", rx_count);
    print_hex_field("seq", stats->last_seq);
    print_u16_field("lost", stats->lost_count);
    print_u16_field("dup", stats->dup_count);
    print_u16_field("ptx_reset", stats->ptx_reset_count);
    print_u16_field("bad_width", bad_width);
    print_hex_field("STATUS", drv_nrf24l01_read_status());
    print_hex_field("FIFO_STATUS", drv_nrf24l01_read_fifo_status());
    print_u16_field("ack_load", ack_load);
    print_u16_field("ack_busy", ack_busy);
    print_u16_field("ack_fail", ack_fail);
    uart_crlf();
}

static void run_prx(void)
{
    stc8h_u8 status;
    stc8h_u8 len;
    stc8h_u8 idle_ticks;
    stc8h_u16 rx_count;
    stc8h_u16 bad_width;
    stc8h_u16 ack_load;
    stc8h_u16 ack_busy;
    stc8h_u16 ack_fail;
    nrf24_pair_diag_rx_stats_t stats;
    stc8h_status_t rx_status;
#if NRF24_PAIR_ACK_PAYLOAD
    stc8h_status_t ack_status;
#endif

    if (configure_radio_common() != STC8H_OK) {
        while (1) {
            stc8h_delay_ms(1000u);
        }
    }

    rx_count = 0u;
    bad_width = 0u;
    ack_load = 0u;
    ack_busy = 0u;
    ack_fail = 0u;
    idle_ticks = 0u;
    nrf24_pair_diag_rx_stats_init(&stats);

    drv_nrf24l01_enter_rx();
#if NRF24_PAIR_ACK_PAYLOAD
    ack_status = load_ack_payload(stats.last_seq, rx_count, nrf24_pair_diag_ack_replace_on_recover());
    if (ack_status == STC8H_OK) {
        ++ack_load;
    } else if (ack_status == STC8H_BUSY) {
        ++ack_busy;
    } else {
        ++ack_fail;
    }
#endif

    while (1) {
#if NRF24_PAIR_DYNAMIC_PAYLOAD
        rx_status = drv_nrf24l01_read_rx_packet(payload, &len, NRF24_PAIR_PAYLOAD_SIZE);
        status = drv_nrf24l01_read_status();
        if (rx_status == STC8H_OK) {
#else
        status = drv_nrf24l01_read_status();
        if ((status & DRV_NRF24L01_STATUS_RX_READY) != 0u) {
            (void)drv_nrf24l01_read_payload(payload, NRF24_PAIR_PAYLOAD_SIZE);
            drv_nrf24l01_clear_irq(status);
            len = NRF24_PAIR_PAYLOAD_SIZE;
            rx_status = STC8H_OK;
#endif
            ++rx_count;
            nrf24_pair_diag_rx_stats_update(&stats, payload[2], payload_tx_count());
#if NRF24_PAIR_ACK_PAYLOAD
            ack_status = load_ack_payload(stats.last_seq, rx_count, nrf24_pair_diag_ack_replace_after_rx());
            if (ack_status == STC8H_OK) {
                ++ack_load;
            } else if (ack_status == STC8H_BUSY) {
                ++ack_busy;
            } else {
                ++ack_fail;
            }
#endif

#if NRF24_PAIR_LOG_EACH_PACKET
            stc8h_uart_write_code(STC8H_UART1, "PRX_PACKET");
            print_u16_field("rx_count", rx_count);
            print_hex_field("seq", stats.last_seq);
            print_hex_field("LEN", len);
            print_hex_field("STATUS", status);
            print_hex_field("FIFO", drv_nrf24l01_read_fifo_status());
            print_payload_head(payload, len);
            uart_crlf();
#endif
            if ((rx_count % NRF24_PAIR_SUMMARY_INTERVAL) == 0u) {
                print_prx_summary(rx_count, &stats, bad_width,
                                  ack_load, ack_busy, ack_fail);
            }
            idle_ticks = 0u;
#if NRF24_PAIR_DYNAMIC_PAYLOAD
        } else if (rx_status == STC8H_ERROR) {
            ++bad_width;
            if ((bad_width % NRF24_PAIR_SUMMARY_INTERVAL) == 0u) {
                print_prx_summary(rx_count, &stats, bad_width,
                                  ack_load, ack_busy, ack_fail);
            }
#endif
        } else {
            ++idle_ticks;
            if (idle_ticks >= 100u) {
                idle_ticks = 0u;
                print_prx_summary(rx_count, &stats, bad_width,
                                  ack_load, ack_busy, ack_fail);
            }
            stc8h_delay_ms(10u);
        }
    }
}
#endif

void main(void)
{
    init_platform();
#if NRF24_PAIR_MATRIX_DIAG
#if NRF24_PAIR_DIAG_PTX
    run_matrix_ptx();
#else
    run_matrix_prx();
#endif
#elif NRF24_PAIR_DIAG_PTX
    run_ptx();
#else
    run_prx();
#endif
}
