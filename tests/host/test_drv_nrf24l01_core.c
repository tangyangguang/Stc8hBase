#include <stdio.h>
#include <string.h>

#define STC8H_SYSCLK_HZ 11059200UL

static void test_ce_high(void);
static void test_ce_low(void);
static void test_csn_high(void);
static void test_csn_low(void);
static void test_configure_pins(void);
static void test_power_up_delay(void);
static void test_ce_pulse_delay(void);

#define DRV_NRF24L01_CE_HIGH() test_ce_high()
#define DRV_NRF24L01_CE_LOW() test_ce_low()
#define DRV_NRF24L01_CSN_HIGH() test_csn_high()
#define DRV_NRF24L01_CSN_LOW() test_csn_low()
#define DRV_NRF24L01_CONFIGURE_PINS() test_configure_pins()
#define DRV_NRF24L01_POWER_UP_DELAY() test_power_up_delay()
#define DRV_NRF24L01_CE_PULSE_DELAY() test_ce_pulse_delay()

#include "../../drivers/drv_nrf24l01.c"

#define TEST_CMD_R_REGISTER 0x00u
#define TEST_CMD_W_REGISTER 0x20u
#define TEST_CMD_R_RX_PAYLOAD 0x61u
#define TEST_CMD_FLUSH_TX 0xE1u
#define TEST_CMD_FLUSH_RX 0xE2u
#define TEST_CMD_R_RX_PL_WID 0x60u
#define TEST_CMD_W_ACK_PAYLOAD 0xA8u
#define TEST_CMD_NOP 0xFFu

#define TEST_REG_EN_AA 0x01u
#define TEST_REG_EN_RXADDR 0x02u
#define TEST_REG_STATUS 0x07u
#define TEST_REG_FIFO_STATUS 0x17u

static stc8h_u8 regs[0x20];
static stc8h_u8 rx_payload[32];
static stc8h_u8 ack_payload_written[32];
static stc8h_u8 ack_payload_len;
static stc8h_u8 ack_payload_pipe;
static stc8h_u8 rx_payload_width;
static stc8h_u8 spi_cmd;
static stc8h_u8 spi_index;
static unsigned int flush_tx_count;
static unsigned int flush_rx_count;
static unsigned int ce_low_count;
static unsigned int ce_high_count;
static unsigned int csn_low_count;
static unsigned int csn_high_count;

static void fake_reset(void)
{
    memset(regs, 0, sizeof(regs));
    memset(rx_payload, 0, sizeof(rx_payload));
    memset(ack_payload_written, 0, sizeof(ack_payload_written));
    regs[TEST_REG_STATUS] = 0x0Eu;
    regs[TEST_REG_FIFO_STATUS] = DRV_NRF24L01_FIFO_TX_EMPTY | DRV_NRF24L01_FIFO_RX_EMPTY;
    ack_payload_len = 0u;
    ack_payload_pipe = 0xFFu;
    rx_payload_width = 0u;
    spi_cmd = TEST_CMD_NOP;
    spi_index = 0u;
    flush_tx_count = 0u;
    flush_rx_count = 0u;
    ce_low_count = 0u;
    ce_high_count = 0u;
    csn_low_count = 0u;
    csn_high_count = 0u;
}

static void test_ce_high(void)
{
    ++ce_high_count;
}

static void test_ce_low(void)
{
    ++ce_low_count;
}

static void test_csn_high(void)
{
    ++csn_high_count;
}

static void test_csn_low(void)
{
    ++csn_low_count;
    spi_index = 0u;
}

static void test_configure_pins(void)
{
}

static void test_power_up_delay(void)
{
}

static void test_ce_pulse_delay(void)
{
}

stc8h_u8 stc8h_spi_transfer(stc8h_u8 value)
{
    stc8h_u8 data_index;
    stc8h_u8 reg;

    if (spi_index == 0u) {
        spi_cmd = value;
        spi_index = 1u;
        if (value == TEST_CMD_FLUSH_TX) {
            ++flush_tx_count;
            regs[TEST_REG_FIFO_STATUS] |= DRV_NRF24L01_FIFO_TX_EMPTY;
        } else if (value == TEST_CMD_FLUSH_RX) {
            ++flush_rx_count;
            regs[TEST_REG_FIFO_STATUS] |= DRV_NRF24L01_FIFO_RX_EMPTY;
        }
        return regs[TEST_REG_STATUS];
    }

    data_index = (stc8h_u8)(spi_index - 1u);
    ++spi_index;

    if ((spi_cmd & 0xE0u) == TEST_CMD_W_REGISTER) {
        reg = (stc8h_u8)(spi_cmd & 0x1Fu);
        if (reg == TEST_REG_STATUS) {
            regs[TEST_REG_STATUS] &= (stc8h_u8)~(value & 0x70u);
        } else {
            regs[reg] = value;
        }
        return regs[TEST_REG_STATUS];
    }

    if ((spi_cmd & 0xE0u) == TEST_CMD_R_REGISTER) {
        return regs[spi_cmd & 0x1Fu];
    }

    if (spi_cmd == TEST_CMD_R_RX_PL_WID) {
        return rx_payload_width;
    }

    if (spi_cmd == TEST_CMD_R_RX_PAYLOAD) {
        if (data_index < sizeof(rx_payload)) {
            return rx_payload[data_index];
        }
        return 0u;
    }

    if ((spi_cmd & 0xF8u) == TEST_CMD_W_ACK_PAYLOAD) {
        ack_payload_pipe = (stc8h_u8)(spi_cmd & 0x07u);
        if (data_index < sizeof(ack_payload_written)) {
            ack_payload_written[data_index] = value;
            ack_payload_len = (stc8h_u8)(data_index + 1u);
        }
        regs[TEST_REG_FIFO_STATUS] &= (stc8h_u8)~DRV_NRF24L01_FIFO_TX_EMPTY;
        return regs[TEST_REG_STATUS];
    }

    return regs[TEST_REG_STATUS];
}

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static int test_auto_ack_does_not_disable_rx_pipe(void)
{
    int failures;

    failures = 0;
    fake_reset();
    regs[TEST_REG_EN_RXADDR] = DRV_NRF24L01_PIPE0;

    drv_nrf24l01_set_auto_ack(0u);
    failures += require(regs[TEST_REG_EN_AA] == 0u,
                        "set_auto_ack(0) must disable EN_AA");
    failures += require(regs[TEST_REG_EN_RXADDR] == DRV_NRF24L01_PIPE0,
                        "set_auto_ack(0) must not disable EN_RXADDR");

    drv_nrf24l01_set_rx_pipes(0xFFu);
    failures += require(regs[TEST_REG_EN_RXADDR] == 0x3Fu,
                        "set_rx_pipes must mask to six nRF24 pipes");
    return failures;
}

static int test_complete_tx_reads_ack_payload_and_clears_irq(void)
{
    int failures;
    stc8h_u8 ack[4];
    stc8h_u8 ack_len;
    drv_nrf24l01_tx_result_t result;

    failures = 0;
    fake_reset();
    regs[TEST_REG_STATUS] = DRV_NRF24L01_STATUS_TX_DONE | DRV_NRF24L01_STATUS_RX_READY;
    rx_payload_width = 3u;
    rx_payload[0] = 'A';
    rx_payload[1] = 'C';
    rx_payload[2] = 'K';
    ack_len = 0u;

    result = drv_nrf24l01_complete_tx(regs[TEST_REG_STATUS], ack, &ack_len, sizeof(ack));

    failures += require(result == DRV_NRF24L01_TX_ACK_PAYLOAD_OK,
                        "complete_tx must classify TX_DS+RX_DR as ACK payload OK");
    failures += require(ack_len == 3u, "complete_tx must report ACK payload length");
    failures += require((ack[0] == 'A') && (ack[1] == 'C') && (ack[2] == 'K'),
                        "complete_tx must read ACK payload bytes");
    failures += require((regs[TEST_REG_STATUS] & (DRV_NRF24L01_STATUS_TX_DONE | DRV_NRF24L01_STATUS_RX_READY)) == 0u,
                        "complete_tx must clear TX_DS/RX_DR after reading ACK payload");
    return failures;
}

static int test_complete_tx_reads_ack_payload_when_status_rx_ready_lags_fifo(void)
{
    int failures;
    stc8h_u8 ack[4];
    stc8h_u8 ack_len;
    drv_nrf24l01_tx_result_t result;

    failures = 0;
    fake_reset();
    regs[TEST_REG_STATUS] = DRV_NRF24L01_STATUS_TX_DONE;
    regs[TEST_REG_FIFO_STATUS] = DRV_NRF24L01_FIFO_TX_EMPTY;
    rx_payload_width = 3u;
    rx_payload[0] = 'A';
    rx_payload[1] = 'C';
    rx_payload[2] = 'K';
    ack_len = 0u;

    result = drv_nrf24l01_complete_tx(regs[TEST_REG_STATUS], ack, &ack_len, sizeof(ack));

    failures += require(result == DRV_NRF24L01_TX_ACK_PAYLOAD_OK,
                        "complete_tx must read ACK payload when RX FIFO is non-empty even if RX_DR is absent from the supplied STATUS");
    failures += require(ack_len == 3u,
                        "complete_tx must report ACK payload length from FIFO fallback");
    failures += require((ack[0] == 'A') && (ack[1] == 'C') && (ack[2] == 'K'),
                        "complete_tx must read ACK payload bytes from FIFO fallback");
    failures += require((regs[TEST_REG_STATUS] & DRV_NRF24L01_STATUS_TX_DONE) == 0u,
                        "complete_tx FIFO fallback must clear TX_DS");
    return failures;
}

static int test_complete_tx_max_rt_flushes_tx_and_clears_irq(void)
{
    int failures;
    stc8h_u8 ack_len;
    drv_nrf24l01_tx_result_t result;

    failures = 0;
    fake_reset();
    regs[TEST_REG_STATUS] = DRV_NRF24L01_STATUS_MAX_RETRY;
    ack_len = 9u;

    result = drv_nrf24l01_complete_tx(regs[TEST_REG_STATUS], 0, &ack_len, 0u);

    failures += require(result == DRV_NRF24L01_TX_MAX_RT,
                        "complete_tx must classify MAX_RT");
    failures += require(flush_tx_count == 1u,
                        "complete_tx must flush TX after MAX_RT");
    failures += require(ack_len == 0u,
                        "complete_tx must clear ACK length on MAX_RT");
    failures += require((regs[TEST_REG_STATUS] & DRV_NRF24L01_STATUS_MAX_RETRY) == 0u,
                        "complete_tx must clear MAX_RT IRQ");
    return failures;
}

static int test_read_rx_packet_flushes_invalid_dynamic_width(void)
{
    int failures;
    stc8h_u8 data[8];
    stc8h_u8 len;
    stc8h_status_t status;

    failures = 0;
    fake_reset();
    regs[TEST_REG_STATUS] = DRV_NRF24L01_STATUS_RX_READY;
    rx_payload_width = 33u;
    len = 8u;

    status = drv_nrf24l01_read_rx_packet(data, &len, sizeof(data));

    failures += require(status == STC8H_ERROR,
                        "read_rx_packet must reject dynamic width > 32");
    failures += require(flush_rx_count == 1u,
                        "read_rx_packet must flush RX on invalid dynamic width");
    failures += require(len == 0u, "read_rx_packet must clear length on invalid width");
    failures += require((regs[TEST_REG_STATUS] & DRV_NRF24L01_STATUS_RX_READY) == 0u,
                        "read_rx_packet must clear RX_DR after invalid width recovery");
    return failures;
}

static int test_preload_ack_payload_replace_policy(void)
{
    int failures;
    static const stc8h_u8 ack[3] = { 'O', 'K', '!' };
    stc8h_status_t status;

    failures = 0;
    fake_reset();
    status = drv_nrf24l01_preload_ack_payload(0u, ack, sizeof(ack), 1u);
    failures += require(status == STC8H_OK, "preload_ack_payload replace must succeed");
    failures += require(flush_tx_count == 1u,
                        "preload_ack_payload replace must flush stale PRX ACK FIFO first");
    failures += require((ack_payload_pipe == 0u) && (ack_payload_len == 3u),
                        "preload_ack_payload must write ACK payload to selected pipe");
    failures += require((ack_payload_written[0] == 'O') && (ack_payload_written[1] == 'K') && (ack_payload_written[2] == '!'),
                        "preload_ack_payload must preserve ACK bytes");

    fake_reset();
    regs[TEST_REG_FIFO_STATUS] = DRV_NRF24L01_FIFO_TX_FULL;
    status = drv_nrf24l01_preload_ack_payload(0u, ack, sizeof(ack), 0u);
    failures += require(status == STC8H_BUSY,
                        "preload_ack_payload without replace must report busy when TX FIFO is full");
    failures += require(ack_payload_len == 0u,
                        "preload_ack_payload must not write when TX FIFO is full and replace is false");
    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_auto_ack_does_not_disable_rx_pipe();
    failures += test_complete_tx_reads_ack_payload_and_clears_irq();
    failures += test_complete_tx_reads_ack_payload_when_status_rx_ready_lags_fifo();
    failures += test_complete_tx_max_rt_flushes_tx_and_clears_irq();
    failures += test_read_rx_packet_flushes_invalid_dynamic_width();
    failures += test_preload_ack_payload_replace_policy();

    return failures == 0 ? 0 : 1;
}
