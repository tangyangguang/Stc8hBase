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

#ifndef DRV_NRF24L01_DEFAULT_CONFIG
#define DRV_NRF24L01_DEFAULT_CONFIG (NRF24_CONFIG_EN_CRC | NRF24_CONFIG_CRCO)
#endif

#if DRV_NRF24L01_ENABLE_RAW_API
#define DRV_NRF24L01_RAW_SCOPE
#else
#define DRV_NRF24L01_RAW_SCOPE static
#endif

static void drv_nrf24l01_short_delay(void)
{
    stc8h_u8 i;

    for (i = 0u; i < 64u; ++i) {
        STC8H_NOP();
    }
}

void drv_nrf24l01_init_pins(void)
{
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

stc8h_u8 drv_nrf24l01_read_status(void)
{
    return drv_nrf24l01_command(NRF24_CMD_NOP);
}

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
    stc8h_u8 ok;

    DRV_NRF24L01_CSN_LOW();
    (void)stc8h_spi_transfer((stc8h_u8)(NRF24_CMD_W_REGISTER | NRF24_REG_RX_ADDR_P0));
    (void)stc8h_spi_transfer(0x11u);
    (void)stc8h_spi_transfer(0x22u);
    (void)stc8h_spi_transfer(0x33u);
    (void)stc8h_spi_transfer(0x44u);
    (void)stc8h_spi_transfer(0x55u);
    DRV_NRF24L01_CSN_HIGH();

    DRV_NRF24L01_CSN_LOW();
    (void)stc8h_spi_transfer((stc8h_u8)(NRF24_CMD_R_REGISTER | NRF24_REG_RX_ADDR_P0));
    ok = (stc8h_spi_transfer(NRF24_CMD_NOP) == 0x11u) ? 1u : 0u;
    ok = (stc8h_spi_transfer(NRF24_CMD_NOP) == 0x22u) ? ok : 0u;
    ok = (stc8h_spi_transfer(NRF24_CMD_NOP) == 0x33u) ? ok : 0u;
    ok = (stc8h_spi_transfer(NRF24_CMD_NOP) == 0x44u) ? ok : 0u;
    ok = (stc8h_spi_transfer(NRF24_CMD_NOP) == 0x55u) ? ok : 0u;
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
}
#endif

void drv_nrf24l01_enter_rx(void)
{
    (void)drv_nrf24l01_write_reg(NRF24_REG_CONFIG, (stc8h_u8)(DRV_NRF24L01_DEFAULT_CONFIG | NRF24_CONFIG_PWR_UP | NRF24_CONFIG_PRIM_RX));
    DRV_NRF24L01_CE_HIGH();
}

void drv_nrf24l01_enter_tx(void)
{
    DRV_NRF24L01_CE_LOW();
    (void)drv_nrf24l01_write_reg(NRF24_REG_CONFIG, (stc8h_u8)(DRV_NRF24L01_DEFAULT_CONFIG | NRF24_CONFIG_PWR_UP));
}

stc8h_status_t drv_nrf24l01_set_channel(stc8h_u8 channel)
{
    if (channel > 125u) {
        return STC8H_ERROR;
    }
    (void)drv_nrf24l01_write_reg(NRF24_REG_RF_CH, channel);
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_address_width(stc8h_u8 width)
{
    if ((width < 3u) || (width > 5u)) {
        return STC8H_ERROR;
    }
    (void)drv_nrf24l01_write_reg(NRF24_REG_SETUP_AW, (stc8h_u8)(width - 2u));
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_tx_address(const stc8h_u8 *addr, stc8h_u8 len)
{
    if ((addr == 0) || (len < 3u) || (len > 5u)) {
        return STC8H_ERROR;
    }
    (void)drv_nrf24l01_write_buf((stc8h_u8)(NRF24_CMD_W_REGISTER | 0x10u), addr, len);
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_rx_address(stc8h_u8 pipe, const stc8h_u8 *addr, stc8h_u8 len)
{
    stc8h_u8 reg;

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

    reg = (stc8h_u8)(NRF24_REG_RX_ADDR_P0 + pipe);
    (void)drv_nrf24l01_write_buf((stc8h_u8)(NRF24_CMD_W_REGISTER | reg), addr, len);
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_payload_size(stc8h_u8 pipe, stc8h_u8 len)
{
    if ((pipe > 5u) || (len > 32u)) {
        return STC8H_ERROR;
    }
    (void)drv_nrf24l01_write_reg((stc8h_u8)(NRF24_REG_RX_PW_P0 + pipe), len);
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_rate_power(drv_nrf24l01_rate_t rate, drv_nrf24l01_power_t power)
{
    stc8h_u8 value;

    if (power > DRV_NRF24L01_POWER_0DBM) {
        return STC8H_ERROR;
    }
    value = (stc8h_u8)(((stc8h_u8)power & 0x03u) << 1);
    if (rate == DRV_NRF24L01_RATE_250KBPS) {
        value |= 0x20u;
    } else if (rate == DRV_NRF24L01_RATE_2MBPS) {
        value |= 0x08u;
    } else if (rate != DRV_NRF24L01_RATE_1MBPS) {
        return STC8H_ERROR;
    }
    (void)drv_nrf24l01_write_reg(NRF24_REG_RF_SETUP, value);
    return STC8H_OK;
}

stc8h_status_t drv_nrf24l01_set_auto_retransmit(stc8h_u8 delay_code, stc8h_u8 count)
{
    if ((delay_code > 15u) || (count > 15u)) {
        return STC8H_ERROR;
    }
    (void)drv_nrf24l01_write_reg(NRF24_REG_SETUP_RETR, (stc8h_u8)((delay_code << 4) | count));
    return STC8H_OK;
}

void drv_nrf24l01_set_auto_ack(stc8h_u8 pipe_mask)
{
    pipe_mask &= 0x3Fu;
    (void)drv_nrf24l01_write_reg(NRF24_REG_EN_AA, pipe_mask);
    (void)drv_nrf24l01_write_reg(NRF24_REG_EN_RXADDR, pipe_mask);
}

stc8h_u8 drv_nrf24l01_write_payload(const stc8h_u8 *data, stc8h_u8 len)
{
    if ((data == 0) || (len > 32u)) {
        return drv_nrf24l01_read_status();
    }
    return drv_nrf24l01_write_buf(NRF24_CMD_W_TX_PAYLOAD, data, len);
}

stc8h_u8 drv_nrf24l01_read_payload(stc8h_u8 *data, stc8h_u8 len)
{
    if ((data == 0) || (len > 32u)) {
        return drv_nrf24l01_read_status();
    }
    return drv_nrf24l01_read_buf(NRF24_CMD_R_RX_PAYLOAD, data, len);
}

void drv_nrf24l01_pulse_ce(void)
{
    DRV_NRF24L01_CE_HIGH();
    drv_nrf24l01_short_delay();
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
    return STC8H_ERROR;
}
#endif

#if DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD
stc8h_status_t drv_nrf24l01_enable_dynamic_payload(stc8h_u8 pipe_mask)
{
    pipe_mask &= 0x3Fu;
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
    pipe_mask &= 0x3Fu;
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
    if ((pipe > 5u) || (data == 0) || (len > 32u)) {
        return drv_nrf24l01_read_status();
    }
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
