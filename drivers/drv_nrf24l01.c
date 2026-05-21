#include "drv_nrf24l01.h"
#include "stc8h_spi.h"

#define NRF24_CMD_R_REGISTER 0x00u
#define NRF24_CMD_W_REGISTER 0x20u
#define NRF24_CMD_R_RX_PAYLOAD 0x61u
#define NRF24_CMD_W_TX_PAYLOAD 0xA0u
#define NRF24_CMD_FLUSH_TX 0xE1u
#define NRF24_CMD_FLUSH_RX 0xE2u
#define NRF24_CMD_ACTIVATE 0x50u
#define NRF24_CMD_R_RX_PL_WID 0x60u
#define NRF24_CMD_W_ACK_PAYLOAD 0xA8u
#define NRF24_CMD_NOP 0xFFu

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

#define NRF24_CONFIG_EN_CRC 0x08u
#define NRF24_CONFIG_CRCO 0x04u
#define NRF24_CONFIG_PWR_UP 0x02u
#define NRF24_CONFIG_PRIM_RX 0x01u

#define NRF24_FEATURE_EN_DPL 0x04u
#define NRF24_FEATURE_EN_ACK_PAY 0x02u

#ifndef DRV_NRF24L01_CE_PULSE_MIN_US
#define DRV_NRF24L01_CE_PULSE_MIN_US 12UL
#endif

#ifndef DRV_NRF24L01_TIMING_LOOP_CYCLES
#define DRV_NRF24L01_TIMING_LOOP_CYCLES 3UL
#endif

#define DRV_NRF24L01_CE_PULSE_DELAY_LOOPS_CALC \
    ((((STC8H_SYSCLK_HZ / 1000UL) * DRV_NRF24L01_CE_PULSE_MIN_US) + ((DRV_NRF24L01_TIMING_LOOP_CYCLES * 1000UL) - 1UL)) / (DRV_NRF24L01_TIMING_LOOP_CYCLES * 1000UL))

#if DRV_NRF24L01_CE_PULSE_DELAY_LOOPS_CALC < 64UL
#define DRV_NRF24L01_CE_PULSE_DELAY_LOOPS 64UL
#else
#define DRV_NRF24L01_CE_PULSE_DELAY_LOOPS DRV_NRF24L01_CE_PULSE_DELAY_LOOPS_CALC
#endif

#ifndef DRV_NRF24L01_POWER_UP_DELAY_US
#define DRV_NRF24L01_POWER_UP_DELAY_US 5000u
#endif

#define DRV_NRF24L01_DELAY_LOOPS_PER_US_CALC \
    ((STC8H_SYSCLK_HZ + ((DRV_NRF24L01_TIMING_LOOP_CYCLES * 1000000UL) - 1UL)) / (DRV_NRF24L01_TIMING_LOOP_CYCLES * 1000000UL))

#if DRV_NRF24L01_DELAY_LOOPS_PER_US_CALC == 0UL
#define DRV_NRF24L01_DELAY_LOOPS_PER_US 1UL
#else
#define DRV_NRF24L01_DELAY_LOOPS_PER_US DRV_NRF24L01_DELAY_LOOPS_PER_US_CALC
#endif

#ifndef DRV_NRF24L01_DEFAULT_CONFIG
#define DRV_NRF24L01_DEFAULT_CONFIG (NRF24_CONFIG_EN_CRC | NRF24_CONFIG_CRCO)
#endif

#if DRV_NRF24L01_ENABLE_RAW_API
#define DRV_NRF24L01_RAW_SCOPE
#else
#define DRV_NRF24L01_RAW_SCOPE static
#endif

#ifndef DRV_NRF24L01_CE_PULSE_DELAY
static void drv_nrf24l01_ce_pulse_delay(void)
{
#if DRV_NRF24L01_CE_PULSE_DELAY_LOOPS > 255UL
    stc8h_u16 i;
#else
    stc8h_u8 i;
#endif

    for (i = 0u; i < DRV_NRF24L01_CE_PULSE_DELAY_LOOPS; ++i) {
        STC8H_NOP();
    }
}
#define DRV_NRF24L01_CE_PULSE_DELAY() drv_nrf24l01_ce_pulse_delay()
#endif

#ifndef DRV_NRF24L01_POWER_UP_DELAY
static void drv_nrf24l01_delay_us(stc8h_u16 us)
{
    stc8h_u8 i;

    while (us != 0u) {
        for (i = 0u; i < DRV_NRF24L01_DELAY_LOOPS_PER_US; ++i) {
            STC8H_NOP();
        }
        --us;
    }
}
#define DRV_NRF24L01_POWER_UP_DELAY() drv_nrf24l01_delay_us(DRV_NRF24L01_POWER_UP_DELAY_US)
#endif

void drv_nrf24l01_init_pins(void)
{
    DRV_NRF24L01_CONFIGURE_PINS();
    DRV_NRF24L01_CE_LOW();
    DRV_NRF24L01_CSN_HIGH();
}

DRV_NRF24L01_RAW_SCOPE stc8h_u8 drv_nrf24l01_command(stc8h_u8 cmd)
{
    stc8h_u8 status;

    DRV_NRF24L01_CSN_LOW();
    status = stc8h_spi_transfer(cmd);
    DRV_NRF24L01_CSN_HIGH();
    return status;
}

#if DRV_NRF24L01_ENABLE_READ_STATUS
stc8h_u8 drv_nrf24l01_read_status(void)
{
    return drv_nrf24l01_command(NRF24_CMD_NOP);
}
#endif

DRV_NRF24L01_RAW_SCOPE stc8h_u8 drv_nrf24l01_read_reg(stc8h_u8 reg)
{
    stc8h_u8 value;

    DRV_NRF24L01_CSN_LOW();
    (void)stc8h_spi_transfer((stc8h_u8)(NRF24_CMD_R_REGISTER | (reg & 0x1Fu)));
    value = stc8h_spi_transfer(NRF24_CMD_NOP);
    DRV_NRF24L01_CSN_HIGH();
    return value;
}

DRV_NRF24L01_RAW_SCOPE stc8h_u8 drv_nrf24l01_write_reg(stc8h_u8 reg, stc8h_u8 value)
{
    stc8h_u8 status;

    DRV_NRF24L01_CSN_LOW();
    status = stc8h_spi_transfer((stc8h_u8)(NRF24_CMD_W_REGISTER | (reg & 0x1Fu)));
    (void)stc8h_spi_transfer(value);
    DRV_NRF24L01_CSN_HIGH();
    return status;
}

DRV_NRF24L01_RAW_SCOPE stc8h_u8 drv_nrf24l01_read_buf(stc8h_u8 cmd, stc8h_u8 *buf, stc8h_u8 len)
{
    stc8h_u8 status;
    stc8h_u8 i;

    DRV_NRF24L01_CSN_LOW();
    status = stc8h_spi_transfer(cmd);
    for (i = 0u; i < len; ++i) {
        buf[i] = stc8h_spi_transfer(NRF24_CMD_NOP);
    }
    DRV_NRF24L01_CSN_HIGH();
    return status;
}

DRV_NRF24L01_RAW_SCOPE stc8h_u8 drv_nrf24l01_write_buf(stc8h_u8 cmd, const stc8h_u8 *buf, stc8h_u8 len)
{
    stc8h_u8 status;
    stc8h_u8 i;

    DRV_NRF24L01_CSN_LOW();
    status = stc8h_spi_transfer(cmd);
    for (i = 0u; i < len; ++i) {
        (void)stc8h_spi_transfer(buf[i]);
    }
    DRV_NRF24L01_CSN_HIGH();
    return status;
}

#if DRV_NRF24L01_ENABLE_CHECK_PRESENT
stc8h_status_t drv_nrf24l01_check_present(void)
{
    static STC8H_CODE stc8h_u8 pattern[5] = { 0x11u, 0x22u, 0x33u, 0x44u, 0x55u };
    stc8h_u8 i;
    stc8h_u8 ok;

    DRV_NRF24L01_CSN_LOW();
    (void)stc8h_spi_transfer((stc8h_u8)(NRF24_CMD_W_REGISTER | NRF24_REG_RX_ADDR_P0));
    for (i = 0u; i < 5u; ++i) {
        (void)stc8h_spi_transfer(pattern[i]);
    }
    DRV_NRF24L01_CSN_HIGH();

    ok = 1u;
    DRV_NRF24L01_CSN_LOW();
    (void)stc8h_spi_transfer((stc8h_u8)(NRF24_CMD_R_REGISTER | NRF24_REG_RX_ADDR_P0));
    for (i = 0u; i < 5u; ++i) {
        if (stc8h_spi_transfer(NRF24_CMD_NOP) != pattern[i]) {
            ok = 0u;
        }
    }
    DRV_NRF24L01_CSN_HIGH();

    return (ok != 0u) ? STC8H_OK : STC8H_ERROR;
}
#endif

#if DRV_NRF24L01_ENABLE_READ_FIFO_STATUS
stc8h_u8 drv_nrf24l01_read_fifo_status(void)
{
    return drv_nrf24l01_read_reg(NRF24_REG_FIFO_STATUS);
}
#endif

#if DRV_NRF24L01_ENABLE_READ_OBSERVE_TX
stc8h_u8 drv_nrf24l01_read_observe_tx(void)
{
    return drv_nrf24l01_read_reg(NRF24_REG_OBSERVE_TX);
}
#endif

#if DRV_NRF24L01_ENABLE_POWER_DOWN
void drv_nrf24l01_power_down(void)
{
    DRV_NRF24L01_CE_LOW();
    (void)drv_nrf24l01_write_reg(NRF24_REG_CONFIG, DRV_NRF24L01_DEFAULT_CONFIG);
}
#endif

#if DRV_NRF24L01_ENABLE_ENTER_STANDBY
void drv_nrf24l01_enter_standby(void)
{
    DRV_NRF24L01_CE_LOW();
    (void)drv_nrf24l01_write_reg(NRF24_REG_CONFIG, (stc8h_u8)(DRV_NRF24L01_DEFAULT_CONFIG | NRF24_CONFIG_PWR_UP));
    DRV_NRF24L01_POWER_UP_DELAY();
}
#endif

#if DRV_NRF24L01_ENABLE_ENTER_RX
void drv_nrf24l01_enter_rx(void)
{
    (void)drv_nrf24l01_write_reg(NRF24_REG_CONFIG, (stc8h_u8)(DRV_NRF24L01_DEFAULT_CONFIG | NRF24_CONFIG_PWR_UP | NRF24_CONFIG_PRIM_RX));
    DRV_NRF24L01_POWER_UP_DELAY();
    DRV_NRF24L01_CE_HIGH();
}
#endif

void drv_nrf24l01_enter_tx(void)
{
    DRV_NRF24L01_CE_LOW();
    (void)drv_nrf24l01_write_reg(NRF24_REG_CONFIG, (stc8h_u8)(DRV_NRF24L01_DEFAULT_CONFIG | NRF24_CONFIG_PWR_UP));
    DRV_NRF24L01_POWER_UP_DELAY();
}

stc8h_status_t drv_nrf24l01_set_channel(stc8h_u8 channel)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if (channel > 125u) {
        return STC8H_ERROR;
    }
#endif
    (void)drv_nrf24l01_write_reg(NRF24_REG_RF_CH, channel);
    return STC8H_OK;
}

#if DRV_NRF24L01_ENABLE_ADDRESS_API
stc8h_status_t drv_nrf24l01_set_address_width(stc8h_u8 width)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((width < 3u) || (width > 5u)) {
        return STC8H_ERROR;
    }
#endif
    (void)drv_nrf24l01_write_reg(NRF24_REG_SETUP_AW, (stc8h_u8)(width - 2u));
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_tx_address(const stc8h_u8 *addr, stc8h_u8 len)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((addr == 0) || (len < 3u) || (len > 5u)) {
        return STC8H_ERROR;
    }
#endif
    (void)drv_nrf24l01_write_buf((stc8h_u8)(NRF24_CMD_W_REGISTER | 0x10u), addr, len);
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_rx_address(stc8h_u8 pipe, const stc8h_u8 *addr, stc8h_u8 len)
{
    stc8h_u8 reg;

#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((addr == 0) || (pipe > 5u)) {
        return STC8H_ERROR;
    }
    if (pipe <= 1u) {
        if ((len < 3u) || (len > 5u)) {
            return STC8H_ERROR;
        }
    } else if (len != 1u) {
        return STC8H_ERROR;
    }
#endif

    reg = (stc8h_u8)(NRF24_REG_RX_ADDR_P0 + pipe);
    (void)drv_nrf24l01_write_buf((stc8h_u8)(NRF24_CMD_W_REGISTER | reg), addr, len);
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_payload_size(stc8h_u8 pipe, stc8h_u8 len)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((pipe > 5u) || (len > 32u)) {
        return STC8H_ERROR;
    }
#endif
    (void)drv_nrf24l01_write_reg((stc8h_u8)(NRF24_REG_RX_PW_P0 + pipe), len);
    return STC8H_OK;
}
#endif

#if DRV_NRF24L01_ENABLE_PIPE0_FIXED_API
stc8h_status_t drv_nrf24l01_config_pipe0_fixed(const stc8h_u8 *addr)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if (addr == 0) {
        return STC8H_ERROR;
    }
#endif
    (void)drv_nrf24l01_write_reg(NRF24_REG_SETUP_AW, (stc8h_u8)(DRV_NRF24L01_FIXED_ADDRESS_WIDTH - 2u));
    (void)drv_nrf24l01_write_buf((stc8h_u8)(NRF24_CMD_W_REGISTER | 0x10u), addr, DRV_NRF24L01_FIXED_ADDRESS_WIDTH);
    (void)drv_nrf24l01_write_buf((stc8h_u8)(NRF24_CMD_W_REGISTER | NRF24_REG_RX_ADDR_P0), addr, DRV_NRF24L01_FIXED_ADDRESS_WIDTH);
    (void)drv_nrf24l01_write_reg(NRF24_REG_RX_PW_P0, DRV_NRF24L01_FIXED_PAYLOAD_SIZE);
    return STC8H_OK;
}
#endif

stc8h_status_t drv_nrf24l01_set_rate_power(drv_nrf24l01_rate_t rate, drv_nrf24l01_power_t power)
{
    stc8h_u8 value;

#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if (power > DRV_NRF24L01_POWER_0DBM) {
        return STC8H_ERROR;
    }
#endif
    value = (stc8h_u8)(((stc8h_u8)power & 0x03u) << 1);
    if (rate == DRV_NRF24L01_RATE_250KBPS) {
        value |= 0x20u;
    } else if (rate == DRV_NRF24L01_RATE_2MBPS) {
        value |= 0x08u;
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    } else if (rate != DRV_NRF24L01_RATE_1MBPS) {
        return STC8H_ERROR;
#endif
    }
    (void)drv_nrf24l01_write_reg(NRF24_REG_RF_SETUP, value);
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_auto_retransmit(stc8h_u8 delay_code, stc8h_u8 count)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((delay_code > 15u) || (count > 15u)) {
        return STC8H_ERROR;
    }
#endif
    (void)drv_nrf24l01_write_reg(NRF24_REG_SETUP_RETR, (stc8h_u8)((delay_code << 4) | count));
    return STC8H_OK;
}

void drv_nrf24l01_set_auto_ack(stc8h_u8 pipe_mask)
{
    pipe_mask &= DRV_NRF24L01_PIPE_MASK_ALL;
    (void)drv_nrf24l01_write_reg(NRF24_REG_EN_AA, pipe_mask);
}

#if DRV_NRF24L01_ENABLE_RX_PIPE_API
void drv_nrf24l01_set_rx_pipes(stc8h_u8 pipe_mask)
{
    pipe_mask &= DRV_NRF24L01_PIPE_MASK_ALL;
    (void)drv_nrf24l01_write_reg(NRF24_REG_EN_RXADDR, pipe_mask);
}
#endif

stc8h_u8 drv_nrf24l01_write_payload(const stc8h_u8 *data, stc8h_u8 len)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((data == 0) || (len > DRV_NRF24L01_PAYLOAD_MAX)) {
#if DRV_NRF24L01_ENABLE_READ_STATUS
        return drv_nrf24l01_read_status();
#else
        return 0u;
#endif
    }
#endif
    return drv_nrf24l01_write_buf(NRF24_CMD_W_TX_PAYLOAD, data, len);
}

#if DRV_NRF24L01_ENABLE_READ_PAYLOAD
stc8h_u8 drv_nrf24l01_read_payload(stc8h_u8 *data, stc8h_u8 len)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((data == 0) || (len > DRV_NRF24L01_PAYLOAD_MAX)) {
#if DRV_NRF24L01_ENABLE_READ_STATUS
        return drv_nrf24l01_read_status();
#else
        return 0u;
#endif
    }
#endif
    return drv_nrf24l01_read_buf(NRF24_CMD_R_RX_PAYLOAD, data, len);
}
#endif

void drv_nrf24l01_pulse_ce(void)
{
    DRV_NRF24L01_CE_HIGH();
    DRV_NRF24L01_CE_PULSE_DELAY();
    DRV_NRF24L01_CE_LOW();
}

void drv_nrf24l01_clear_irq(stc8h_u8 flags)
{
    (void)drv_nrf24l01_write_reg(NRF24_REG_STATUS, (stc8h_u8)(flags & 0x70u));
}

void drv_nrf24l01_flush_tx(void)
{
    (void)drv_nrf24l01_command(NRF24_CMD_FLUSH_TX);
}

void drv_nrf24l01_flush_rx(void)
{
    (void)drv_nrf24l01_command(NRF24_CMD_FLUSH_RX);
}

#if DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD || DRV_NRF24L01_ENABLE_ACK_PAYLOAD
static stc8h_status_t drv_nrf24l01_enable_feature_bits(stc8h_u8 bits)
{
    stc8h_u8 value;

    value = drv_nrf24l01_read_reg(NRF24_REG_FEATURE);
    value |= bits;
    (void)drv_nrf24l01_write_reg(NRF24_REG_FEATURE, value);
    if ((drv_nrf24l01_read_reg(NRF24_REG_FEATURE) & bits) == bits) {
        return STC8H_OK;
    }

#if DRV_NRF24L01_REQUIRES_ACTIVATE
    DRV_NRF24L01_CSN_LOW();
    (void)stc8h_spi_transfer(NRF24_CMD_ACTIVATE);
    (void)stc8h_spi_transfer(0x73u);
    DRV_NRF24L01_CSN_HIGH();

    value = drv_nrf24l01_read_reg(NRF24_REG_FEATURE);
    value |= bits;
    (void)drv_nrf24l01_write_reg(NRF24_REG_FEATURE, value);
    if ((drv_nrf24l01_read_reg(NRF24_REG_FEATURE) & bits) == bits) {
        return STC8H_OK;
    }
#endif
    return STC8H_ERROR;
}
#endif

#if DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD
stc8h_status_t drv_nrf24l01_enable_dynamic_payload(stc8h_u8 pipe_mask)
{
    pipe_mask &= DRV_NRF24L01_PIPE_MASK_ALL;
    if (drv_nrf24l01_enable_feature_bits(NRF24_FEATURE_EN_DPL) != STC8H_OK) {
        return STC8H_ERROR;
    }
    (void)drv_nrf24l01_write_reg(NRF24_REG_DYNPD, pipe_mask);
    return STC8H_OK;
}

void drv_nrf24l01_disable_dynamic_payload(void)
{
    stc8h_u8 feature;

    (void)drv_nrf24l01_write_reg(NRF24_REG_DYNPD, 0u);
    feature = drv_nrf24l01_read_reg(NRF24_REG_FEATURE);
    feature &= (stc8h_u8)~NRF24_FEATURE_EN_DPL;
    (void)drv_nrf24l01_write_reg(NRF24_REG_FEATURE, feature);
}
#endif

#if DRV_NRF24L01_ENABLE_ACK_PAYLOAD || DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD
#if !DRV_NRF24L01_ENABLE_ACK_PAYLOAD
#error "DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD requires DRV_NRF24L01_ENABLE_ACK_PAYLOAD."
#endif
#endif

#if DRV_NRF24L01_ENABLE_ACK_PAYLOAD
stc8h_status_t drv_nrf24l01_enable_ack_payload(stc8h_u8 pipe_mask)
{
    pipe_mask &= DRV_NRF24L01_PIPE_MASK_ALL;
    if (drv_nrf24l01_enable_feature_bits((stc8h_u8)(NRF24_FEATURE_EN_DPL | NRF24_FEATURE_EN_ACK_PAY)) != STC8H_OK) {
        return STC8H_ERROR;
    }
    (void)drv_nrf24l01_write_reg(NRF24_REG_DYNPD, pipe_mask);
    return STC8H_OK;
}
#endif

#if DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD
stc8h_u8 drv_nrf24l01_write_ack_payload(stc8h_u8 pipe, const stc8h_u8 *data, stc8h_u8 len)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((pipe > 5u) || (data == 0) || (len == 0u) || (len > DRV_NRF24L01_PAYLOAD_MAX)) {
#if DRV_NRF24L01_ENABLE_READ_STATUS
        return drv_nrf24l01_read_status();
#else
        return 0u;
#endif
    }
#endif
    return drv_nrf24l01_write_buf((stc8h_u8)(NRF24_CMD_W_ACK_PAYLOAD | pipe), data, len);
}
#endif

#if DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD
void drv_nrf24l01_disable_ack_payload(void)
{
    stc8h_u8 feature;

    feature = drv_nrf24l01_read_reg(NRF24_REG_FEATURE);
    feature &= (stc8h_u8)~NRF24_FEATURE_EN_ACK_PAY;
    (void)drv_nrf24l01_write_reg(NRF24_REG_FEATURE, feature);
}
#endif

#if DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE
stc8h_u8 drv_nrf24l01_read_dynamic_payload_size(void)
{
    stc8h_u8 value;

    DRV_NRF24L01_CSN_LOW();
    (void)stc8h_spi_transfer(NRF24_CMD_R_RX_PL_WID);
    value = stc8h_spi_transfer(NRF24_CMD_NOP);
    DRV_NRF24L01_CSN_HIGH();
    return value;
}
#endif

#if DRV_NRF24L01_ENABLE_TX_RESULT_API
drv_nrf24l01_tx_result_t drv_nrf24l01_complete_tx(stc8h_u8 status, stc8h_u8 *ack_payload, stc8h_u8 *ack_len, stc8h_u8 ack_max_len)
{
    stc8h_u8 width;

    if (ack_len != 0) {
        *ack_len = 0u;
    }

    if ((status & DRV_NRF24L01_STATUS_MAX_RETRY) != 0u) {
        drv_nrf24l01_flush_tx();
        drv_nrf24l01_clear_irq(DRV_NRF24L01_STATUS_MAX_RETRY);
        return DRV_NRF24L01_TX_MAX_RT;
    }

    if ((status & DRV_NRF24L01_STATUS_TX_DONE) == 0u) {
        return DRV_NRF24L01_TX_PENDING;
    }

    if (((status & DRV_NRF24L01_STATUS_RX_READY) != 0u) ||
        ((ack_len != 0) && ((drv_nrf24l01_read_fifo_status() & DRV_NRF24L01_FIFO_RX_EMPTY) == 0u))) {
        width = drv_nrf24l01_read_dynamic_payload_size();
        if (width == 0u) {
            drv_nrf24l01_flush_rx();
            drv_nrf24l01_clear_irq((stc8h_u8)(status | DRV_NRF24L01_STATUS_RX_READY));
            return DRV_NRF24L01_TX_ACK_EMPTY;
        }
        if ((width > DRV_NRF24L01_PAYLOAD_MAX) || (ack_payload == 0) || (ack_len == 0) || (width > ack_max_len)) {
            drv_nrf24l01_flush_rx();
            drv_nrf24l01_clear_irq((stc8h_u8)(status | DRV_NRF24L01_STATUS_RX_READY));
            return DRV_NRF24L01_TX_ACK_PAYLOAD_INVALID;
        }
        (void)drv_nrf24l01_read_payload(ack_payload, width);
        *ack_len = width;
        drv_nrf24l01_clear_irq((stc8h_u8)(status | DRV_NRF24L01_STATUS_RX_READY));
        return DRV_NRF24L01_TX_ACK_PAYLOAD_OK;
    }

    drv_nrf24l01_clear_irq(status);
    if (ack_len != 0) {
        return DRV_NRF24L01_TX_ACK_EMPTY;
    }
    return DRV_NRF24L01_TX_DONE;
}
#endif

#if DRV_NRF24L01_ENABLE_RX_PACKET_API
stc8h_status_t drv_nrf24l01_read_rx_packet(stc8h_u8 *data, stc8h_u8 *len, stc8h_u8 max_len)
{
    stc8h_u8 status;
    stc8h_u8 width;

#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((data == 0) || (len == 0) || (max_len == 0u) || (max_len > DRV_NRF24L01_PAYLOAD_MAX)) {
        return STC8H_ERROR;
    }
#endif
    *len = 0u;
    status = drv_nrf24l01_read_status();
    if ((status & DRV_NRF24L01_STATUS_RX_READY) == 0u) {
        return STC8H_BUSY;
    }

    width = drv_nrf24l01_read_dynamic_payload_size();
    if ((width == 0u) || (width > DRV_NRF24L01_PAYLOAD_MAX) || (width > max_len)) {
        drv_nrf24l01_flush_rx();
        drv_nrf24l01_clear_irq(status);
        return STC8H_ERROR;
    }

    (void)drv_nrf24l01_read_payload(data, width);
    *len = width;
    drv_nrf24l01_clear_irq(status);
    return STC8H_OK;
}
#endif

#if DRV_NRF24L01_ENABLE_ACK_PRELOAD_API
stc8h_status_t drv_nrf24l01_preload_ack_payload(stc8h_u8 pipe, const stc8h_u8 *data, stc8h_u8 len, stc8h_u8 replace_pending)
{
#if DRV_NRF24L01_ENABLE_ARG_CHECK
    if ((pipe > 5u) || (data == 0) || (len == 0u) || (len > DRV_NRF24L01_PAYLOAD_MAX)) {
        return STC8H_ERROR;
    }
#endif
    if (replace_pending != 0u) {
        drv_nrf24l01_flush_tx();
    } else if ((drv_nrf24l01_read_fifo_status() & DRV_NRF24L01_FIFO_TX_FULL) != 0u) {
        return STC8H_BUSY;
    }

    (void)drv_nrf24l01_write_ack_payload(pipe, data, len);
    return STC8H_OK;
}
#endif

#if DRV_NRF24L01_ENABLE_RECOVER
void drv_nrf24l01_recover(drv_nrf24l01_recover_mode_t mode)
{
    DRV_NRF24L01_CE_LOW();
    drv_nrf24l01_flush_tx();
    drv_nrf24l01_flush_rx();
    drv_nrf24l01_clear_irq(DRV_NRF24L01_IRQ_MASK);

    if (mode == DRV_NRF24L01_RECOVER_PRX) {
        drv_nrf24l01_enter_rx();
    } else if (mode == DRV_NRF24L01_RECOVER_PTX) {
        drv_nrf24l01_enter_tx();
    } else {
        drv_nrf24l01_enter_standby();
    }
}
#endif
