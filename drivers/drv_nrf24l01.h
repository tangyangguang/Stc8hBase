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

#ifndef DRV_NRF24L01_ENABLE_CHECK_PRESENT
#define DRV_NRF24L01_ENABLE_CHECK_PRESENT 1
#endif

#ifndef DRV_NRF24L01_ENABLE_ARG_CHECK
#define DRV_NRF24L01_ENABLE_ARG_CHECK 1
#endif

#ifndef DRV_NRF24L01_ENABLE_ADDRESS_API
#define DRV_NRF24L01_ENABLE_ADDRESS_API 1
#endif

#ifndef DRV_NRF24L01_ENABLE_PIPE0_FIXED_API
#define DRV_NRF24L01_ENABLE_PIPE0_FIXED_API 0
#endif

#ifndef DRV_NRF24L01_FIXED_ADDRESS_WIDTH
#define DRV_NRF24L01_FIXED_ADDRESS_WIDTH 5u
#endif

#ifndef DRV_NRF24L01_FIXED_PAYLOAD_SIZE
#define DRV_NRF24L01_FIXED_PAYLOAD_SIZE 32u
#endif

#if (DRV_NRF24L01_FIXED_ADDRESS_WIDTH < 3u) || (DRV_NRF24L01_FIXED_ADDRESS_WIDTH > 5u)
#error "DRV_NRF24L01_FIXED_ADDRESS_WIDTH must be 3..5."
#endif

#if DRV_NRF24L01_FIXED_PAYLOAD_SIZE > 32u
#error "DRV_NRF24L01_FIXED_PAYLOAD_SIZE must be <= 32."
#endif

#ifndef DRV_NRF24L01_ENABLE_READ_FIFO_STATUS
#define DRV_NRF24L01_ENABLE_READ_FIFO_STATUS 1
#endif

#ifndef DRV_NRF24L01_ENABLE_READ_OBSERVE_TX
#define DRV_NRF24L01_ENABLE_READ_OBSERVE_TX 1
#endif

#ifndef DRV_NRF24L01_ENABLE_READ_STATUS
#define DRV_NRF24L01_ENABLE_READ_STATUS 1
#endif

#ifndef DRV_NRF24L01_ENABLE_RAW_API
#define DRV_NRF24L01_ENABLE_RAW_API 1
#endif

#ifndef DRV_NRF24L01_ENABLE_POWER_DOWN
#define DRV_NRF24L01_ENABLE_POWER_DOWN 1
#endif

#ifndef DRV_NRF24L01_ENABLE_ENTER_STANDBY
#define DRV_NRF24L01_ENABLE_ENTER_STANDBY 1
#endif

#ifndef DRV_NRF24L01_ENABLE_ENTER_RX
#define DRV_NRF24L01_ENABLE_ENTER_RX 1
#endif

#ifndef DRV_NRF24L01_ENABLE_READ_PAYLOAD
#define DRV_NRF24L01_ENABLE_READ_PAYLOAD 1
#endif

#ifndef DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD
#define DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD 1
#endif

#ifndef DRV_NRF24L01_ENABLE_ACK_PAYLOAD
#define DRV_NRF24L01_ENABLE_ACK_PAYLOAD 1
#endif

#ifndef DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD
#define DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD DRV_NRF24L01_ENABLE_ACK_PAYLOAD
#endif

#ifndef DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD
#define DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD DRV_NRF24L01_ENABLE_ACK_PAYLOAD
#endif

#ifndef DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE
#define DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE (DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD || DRV_NRF24L01_ENABLE_ACK_PAYLOAD)
#endif

/* The original nRF24L01 (non +) requires sending the ACTIVATE 0x73
 * command before FEATURE bits become writable. nRF24L01+ accepts
 * FEATURE writes directly. Apps that have positively identified the
 * chip as nRF24L01+ can set this to 0 to drop the second-attempt
 * ACTIVATE fallback inside enable_dynamic_payload /
 * enable_ack_payload. Default 1 preserves the legacy-compatible
 * path. */
#ifndef DRV_NRF24L01_REQUIRES_ACTIVATE
#define DRV_NRF24L01_REQUIRES_ACTIVATE 1
#endif

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
#if DRV_NRF24L01_ENABLE_CHECK_PRESENT
stc8h_status_t drv_nrf24l01_check_present(void);
#endif

#if DRV_NRF24L01_ENABLE_READ_STATUS
stc8h_u8 drv_nrf24l01_read_status(void);
#endif
#if DRV_NRF24L01_ENABLE_READ_FIFO_STATUS
stc8h_u8 drv_nrf24l01_read_fifo_status(void);
#endif
#if DRV_NRF24L01_ENABLE_READ_OBSERVE_TX
stc8h_u8 drv_nrf24l01_read_observe_tx(void);
#endif

#if DRV_NRF24L01_ENABLE_RAW_API
stc8h_u8 drv_nrf24l01_read_reg(stc8h_u8 reg);
stc8h_u8 drv_nrf24l01_write_reg(stc8h_u8 reg, stc8h_u8 value);
stc8h_u8 drv_nrf24l01_read_buf(stc8h_u8 cmd, stc8h_u8 *buf, stc8h_u8 len);
stc8h_u8 drv_nrf24l01_write_buf(stc8h_u8 cmd, const stc8h_u8 *buf, stc8h_u8 len);
stc8h_u8 drv_nrf24l01_command(stc8h_u8 cmd);
#endif

#if DRV_NRF24L01_ENABLE_POWER_DOWN
void drv_nrf24l01_power_down(void);
#endif
#if DRV_NRF24L01_ENABLE_ENTER_STANDBY
void drv_nrf24l01_enter_standby(void);
#endif
#if DRV_NRF24L01_ENABLE_ENTER_RX
void drv_nrf24l01_enter_rx(void);
#endif
void drv_nrf24l01_enter_tx(void);

stc8h_status_t drv_nrf24l01_set_channel(stc8h_u8 channel);
#if DRV_NRF24L01_ENABLE_ADDRESS_API
stc8h_status_t drv_nrf24l01_set_address_width(stc8h_u8 width);
stc8h_status_t drv_nrf24l01_set_tx_address(const stc8h_u8 *addr, stc8h_u8 len);
stc8h_status_t drv_nrf24l01_set_rx_address(stc8h_u8 pipe, const stc8h_u8 *addr, stc8h_u8 len);
stc8h_status_t drv_nrf24l01_set_payload_size(stc8h_u8 pipe, stc8h_u8 len);
#endif
#if DRV_NRF24L01_ENABLE_PIPE0_FIXED_API
stc8h_status_t drv_nrf24l01_config_pipe0_fixed(const stc8h_u8 *addr);
#endif
stc8h_status_t drv_nrf24l01_set_rate_power(drv_nrf24l01_rate_t rate, drv_nrf24l01_power_t power);
stc8h_status_t drv_nrf24l01_set_auto_retransmit(stc8h_u8 delay_code, stc8h_u8 count);
void drv_nrf24l01_set_auto_ack(stc8h_u8 pipe_mask);

stc8h_u8 drv_nrf24l01_write_payload(const stc8h_u8 *data, stc8h_u8 len);
#if DRV_NRF24L01_ENABLE_READ_PAYLOAD
stc8h_u8 drv_nrf24l01_read_payload(stc8h_u8 *data, stc8h_u8 len);
#endif
void drv_nrf24l01_pulse_ce(void);

void drv_nrf24l01_clear_irq(stc8h_u8 flags);
void drv_nrf24l01_flush_tx(void);
void drv_nrf24l01_flush_rx(void);

#if DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD
stc8h_status_t drv_nrf24l01_enable_dynamic_payload(stc8h_u8 pipe_mask);
void drv_nrf24l01_disable_dynamic_payload(void);
#endif
#if DRV_NRF24L01_ENABLE_ACK_PAYLOAD
stc8h_status_t drv_nrf24l01_enable_ack_payload(stc8h_u8 pipe_mask);
#endif
#if DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD
stc8h_u8 drv_nrf24l01_write_ack_payload(stc8h_u8 pipe, const stc8h_u8 *data, stc8h_u8 len);
#endif
#if DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD
void drv_nrf24l01_disable_ack_payload(void);
#endif
#if DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE
stc8h_u8 drv_nrf24l01_read_dynamic_payload_size(void);
#endif

#endif
