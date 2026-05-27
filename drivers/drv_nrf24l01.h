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
#define DRV_NRF24L01_PIPE_MASK_ALL 0x3Fu
#define DRV_NRF24L01_IRQ_MASK 0x70u
#define DRV_NRF24L01_PAYLOAD_MAX 32u

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

#ifndef DRV_NRF24L01_ENABLE_WRITE_PAYLOAD
#define DRV_NRF24L01_ENABLE_WRITE_PAYLOAD 1
#endif

#ifndef DRV_NRF24L01_ENABLE_FIXED_PAYLOAD_API
#define DRV_NRF24L01_ENABLE_FIXED_PAYLOAD_API 0
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

#ifndef DRV_NRF24L01_ENABLE_RX_PIPE_API
#define DRV_NRF24L01_ENABLE_RX_PIPE_API 1
#endif

#ifndef DRV_NRF24L01_ENABLE_TX_RESULT_API
#define DRV_NRF24L01_ENABLE_TX_RESULT_API (DRV_NRF24L01_ENABLE_READ_PAYLOAD && DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE && DRV_NRF24L01_ENABLE_READ_FIFO_STATUS)
#endif

#ifndef DRV_NRF24L01_ENABLE_RX_PACKET_API
#define DRV_NRF24L01_ENABLE_RX_PACKET_API (DRV_NRF24L01_ENABLE_READ_STATUS && DRV_NRF24L01_ENABLE_READ_PAYLOAD && DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE && DRV_NRF24L01_ENABLE_READ_FIFO_STATUS)
#endif

#ifndef DRV_NRF24L01_ENABLE_ACK_PRELOAD_API
#define DRV_NRF24L01_ENABLE_ACK_PRELOAD_API (DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD && DRV_NRF24L01_ENABLE_READ_FIFO_STATUS)
#endif

#if DRV_NRF24L01_ENABLE_RX_PACKET_API && !DRV_NRF24L01_ENABLE_READ_FIFO_STATUS
#error "DRV_NRF24L01_ENABLE_RX_PACKET_API requires DRV_NRF24L01_ENABLE_READ_FIFO_STATUS."
#endif

#ifndef DRV_NRF24L01_ENABLE_RECOVER
#define DRV_NRF24L01_ENABLE_RECOVER (DRV_NRF24L01_ENABLE_ENTER_STANDBY && DRV_NRF24L01_ENABLE_ENTER_RX)
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

/* Optional nRF24L01+ fast path: write FEATURE directly and skip the
 * FEATURE readback plus ACTIVATE fallback. Default 0 preserves the
 * safer startup detection and original nRF24L01 compatibility path. */
#ifndef DRV_NRF24L01_FEATURE_ENABLE_DIRECT_WRITE
#define DRV_NRF24L01_FEATURE_ENABLE_DIRECT_WRITE 0
#endif

/* Timing defaults follow nRF24L01+ Product Specification v1.0.
 * Override these only after confirming the module crystal/startup
 * characteristics and target clock. */
#ifndef DRV_NRF24L01_CE_PULSE_MIN_US
#define DRV_NRF24L01_CE_PULSE_MIN_US 12UL
#endif

#ifndef DRV_NRF24L01_POWER_UP_DELAY_US
#define DRV_NRF24L01_POWER_UP_DELAY_US 5000u
#endif

/* Optional board hook called by drv_nrf24l01_init_pins() before CE/CSN
 * are driven. Use it to configure CE/CSN port modes and latches on MCUs
 * where GPIOs do not reset to output-capable quasi-bidirectional mode. */
#ifndef DRV_NRF24L01_CONFIGURE_PINS
#define DRV_NRF24L01_CONFIGURE_PINS() do { } while (0)
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

typedef enum {
    DRV_NRF24L01_TX_PENDING = 0,
    DRV_NRF24L01_TX_DONE,
    DRV_NRF24L01_TX_MAX_RT,
    DRV_NRF24L01_TX_ACK_EMPTY,
    DRV_NRF24L01_TX_ACK_PAYLOAD_OK,
    DRV_NRF24L01_TX_ACK_PAYLOAD_INVALID
} drv_nrf24l01_tx_result_t;

typedef enum {
    DRV_NRF24L01_RECOVER_STANDBY = 0,
    DRV_NRF24L01_RECOVER_PTX,
    DRV_NRF24L01_RECOVER_PRX
} drv_nrf24l01_recover_mode_t;

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
#if DRV_NRF24L01_ENABLE_RX_PIPE_API
void drv_nrf24l01_set_rx_pipes(stc8h_u8 pipe_mask);
#endif

#if DRV_NRF24L01_ENABLE_WRITE_PAYLOAD
stc8h_u8 drv_nrf24l01_write_payload(const stc8h_u8 *data, stc8h_u8 len);
#endif
#if DRV_NRF24L01_ENABLE_READ_PAYLOAD
stc8h_u8 drv_nrf24l01_read_payload(stc8h_u8 *data, stc8h_u8 len);
#endif
#if DRV_NRF24L01_ENABLE_FIXED_PAYLOAD_API
stc8h_u8 drv_nrf24l01_write_payload_fixed(const stc8h_u8 *data);
stc8h_u8 drv_nrf24l01_read_payload_fixed(stc8h_u8 *data);
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

#if DRV_NRF24L01_ENABLE_TX_RESULT_API
drv_nrf24l01_tx_result_t drv_nrf24l01_complete_tx(stc8h_u8 status, stc8h_u8 *ack_payload, stc8h_u8 *ack_len, stc8h_u8 ack_max_len);
#endif
#if DRV_NRF24L01_ENABLE_RX_PACKET_API
stc8h_status_t drv_nrf24l01_read_rx_packet(stc8h_u8 *data, stc8h_u8 *len, stc8h_u8 max_len);
#endif
#if DRV_NRF24L01_ENABLE_ACK_PRELOAD_API
stc8h_status_t drv_nrf24l01_preload_ack_payload(stc8h_u8 pipe, const stc8h_u8 *data, stc8h_u8 len, stc8h_u8 replace_pending);
#endif
#if DRV_NRF24L01_ENABLE_RECOVER
void drv_nrf24l01_recover(drv_nrf24l01_recover_mode_t mode);
#endif

#endif
