#include "drv_nrf24l01.h"
#include "stc8h_spi.h"
#include "stc8h_uart.h"

#define NRF24_PAYLOAD_SIZE 8u
#define NRF24_TX_WAIT_LIMIT 30000u

static const stc8h_u8 address[5] = {'S', 'T', 'A', 'C', 'K'};
static stc8h_u8 tx_payload[NRF24_PAYLOAD_SIZE] = {0xA5u, 0x01u, 0u, 0u, 0u, 0u, 0u, 0u};
static stc8h_u8 ack_payload[NRF24_PAYLOAD_SIZE];

static void configure_radio(void)
{
    (void)drv_nrf24l01_set_channel(2u);
    (void)drv_nrf24l01_set_address_width(5u);
    (void)drv_nrf24l01_set_tx_address(address, 5u);
    (void)drv_nrf24l01_set_rx_address(0u, address, 5u);
    drv_nrf24l01_set_rx_pipes(DRV_NRF24L01_PIPE0);
    drv_nrf24l01_set_auto_ack(DRV_NRF24L01_PIPE0);
    (void)drv_nrf24l01_set_auto_retransmit(3u, 10u);
    (void)drv_nrf24l01_set_rate_power(DRV_NRF24L01_RATE_1MBPS, DRV_NRF24L01_POWER_NEG6DBM);
    (void)drv_nrf24l01_enable_ack_payload(DRV_NRF24L01_PIPE0);
}

void main(void)
{
    stc8h_u8 status;
    stc8h_u8 ack_len;
    stc8h_u16 wait;
    drv_nrf24l01_tx_result_t result;

    (void)stc8h_uart_init(STC8H_UART1);
    drv_nrf24l01_init_pins();
    stc8h_spi_init();
    configure_radio();
    drv_nrf24l01_enter_tx();

    while (1) {
        ++tx_payload[2];
        drv_nrf24l01_flush_tx();
        drv_nrf24l01_flush_rx();
        drv_nrf24l01_clear_irq(DRV_NRF24L01_IRQ_MASK);
        (void)drv_nrf24l01_write_payload(tx_payload, NRF24_PAYLOAD_SIZE);
        drv_nrf24l01_pulse_ce();

        status = 0u;
        for (wait = 0u; wait < NRF24_TX_WAIT_LIMIT; ++wait) {
            status = drv_nrf24l01_read_status();
            if ((status & (DRV_NRF24L01_STATUS_TX_DONE | DRV_NRF24L01_STATUS_MAX_RETRY)) != 0u) {
                break;
            }
        }

        if (wait >= NRF24_TX_WAIT_LIMIT) {
            drv_nrf24l01_recover(DRV_NRF24L01_RECOVER_PTX);
            stc8h_uart_write_code(STC8H_UART1, "tx timeout\r\n");
        } else {
            result = drv_nrf24l01_complete_tx(status, ack_payload, &ack_len, NRF24_PAYLOAD_SIZE);
            if (result == DRV_NRF24L01_TX_ACK_PAYLOAD_OK) {
                stc8h_uart_write_code(STC8H_UART1, "ack payload\r\n");
            } else if (result == DRV_NRF24L01_TX_ACK_EMPTY) {
                stc8h_uart_write_code(STC8H_UART1, "ack empty\r\n");
            } else if (result == DRV_NRF24L01_TX_MAX_RT) {
                stc8h_uart_write_code(STC8H_UART1, "max retry\r\n");
            } else {
                stc8h_uart_write_code(STC8H_UART1, "tx error\r\n");
            }
        }
    }
}
