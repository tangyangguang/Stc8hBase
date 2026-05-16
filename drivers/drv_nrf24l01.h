#ifndef DRV_NRF24L01_H
#define DRV_NRF24L01_H

#include "stc8h_config.h"

#define DRV_NRF24L01_STATUS_RX_READY 0x40u
#define DRV_NRF24L01_STATUS_TX_DONE 0x20u
#define DRV_NRF24L01_STATUS_MAX_RETRY 0x10u
#define DRV_NRF24L01_STATUS_TX_FULL 0x01u

#define DRV_NRF24L01_FIFO_TX_REUSE 0x40u
#define DRV_NRF24L01_FIFO_TX_FULL 0x20u
#define DRV_NRF24L01_FIFO_TX_EMPTY 0x10u
#define DRV_NRF24L01_FIFO_RX_FULL 0x02u
#define DRV_NRF24L01_FIFO_RX_EMPTY 0x01u

#define DRV_NRF24L01_PIPE0 0x01u

typedef enum {
    DRV_NRF24L01_RATE_250KBPS = 0,
    DRV_NRF24L01_RATE_1MBPS,
    DRV_NRF24L01_RATE_2MBPS
} drv_nrf24l01_rate_t;

typedef enum {
    DRV_NRF24L01_POWER_NEG18DBM = 0,
    DRV_NRF24L01_POWER_NEG12DBM,
    DRV_NRF24L01_POWER_NEG6DBM,
    DRV_NRF24L01_POWER_0DBM
} drv_nrf24l01_power_t;

void drv_nrf24l01_init_pins(void);
stc8h_status_t drv_nrf24l01_check_present(void);

stc8h_u8 drv_nrf24l01_read_status(void);
stc8h_u8 drv_nrf24l01_read_fifo_status(void);
stc8h_u8 drv_nrf24l01_read_observe_tx(void);

stc8h_u8 drv_nrf24l01_read_reg(stc8h_u8 reg);
stc8h_u8 drv_nrf24l01_write_reg(stc8h_u8 reg, stc8h_u8 value);
stc8h_u8 drv_nrf24l01_read_buf(stc8h_u8 cmd, stc8h_u8 *buf, stc8h_u8 len);
stc8h_u8 drv_nrf24l01_write_buf(stc8h_u8 cmd, const stc8h_u8 *buf, stc8h_u8 len);
stc8h_u8 drv_nrf24l01_command(stc8h_u8 cmd);

void drv_nrf24l01_power_down(void);
void drv_nrf24l01_enter_standby(void);
void drv_nrf24l01_enter_rx(void);
void drv_nrf24l01_enter_tx(void);

stc8h_status_t drv_nrf24l01_set_channel(stc8h_u8 channel);
stc8h_status_t drv_nrf24l01_set_address_width(stc8h_u8 width);
stc8h_status_t drv_nrf24l01_set_tx_address(const stc8h_u8 *addr, stc8h_u8 len);
stc8h_status_t drv_nrf24l01_set_rx_address(stc8h_u8 pipe, const stc8h_u8 *addr, stc8h_u8 len);
stc8h_status_t drv_nrf24l01_set_payload_size(stc8h_u8 pipe, stc8h_u8 len);
stc8h_status_t drv_nrf24l01_set_rate_power(drv_nrf24l01_rate_t rate, drv_nrf24l01_power_t power);
stc8h_status_t drv_nrf24l01_set_auto_retransmit(stc8h_u8 delay_code, stc8h_u8 count);
void drv_nrf24l01_set_auto_ack(stc8h_u8 pipe_mask);

stc8h_u8 drv_nrf24l01_write_payload(const stc8h_u8 *data, stc8h_u8 len);
stc8h_u8 drv_nrf24l01_read_payload(stc8h_u8 *data, stc8h_u8 len);
void drv_nrf24l01_pulse_ce(void);

void drv_nrf24l01_clear_irq(stc8h_u8 flags);
void drv_nrf24l01_flush_tx(void);
void drv_nrf24l01_flush_rx(void);

stc8h_status_t drv_nrf24l01_enable_dynamic_payload(stc8h_u8 pipe_mask);
void drv_nrf24l01_disable_dynamic_payload(void);
stc8h_status_t drv_nrf24l01_enable_ack_payload(stc8h_u8 pipe_mask);
void drv_nrf24l01_disable_ack_payload(void);
stc8h_u8 drv_nrf24l01_read_dynamic_payload_size(void);
stc8h_u8 drv_nrf24l01_write_ack_payload(stc8h_u8 pipe, const stc8h_u8 *data, stc8h_u8 len);

#endif
