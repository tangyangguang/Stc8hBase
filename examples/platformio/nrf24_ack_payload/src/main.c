#include "drv_nrf24l01.h"
#include "stc8h_spi.h"
#include "stc8h_uart.h"

#define NRF24_PAYLOAD_SIZE 8u

static const stc8h_u8 address[5] = {'S', 'T', 'A', 'C', 'K'};
static stc8h_u8 tx_payload[NRF24_PAYLOAD_SIZE] = {0xA5u, 0x01u, 0u, 0u, 0u, 0u, 0u, 0u};
static stc8h_u8 ack_payload[NRF24_PAYLOAD_SIZE];

static void configure_radio(void)
{
    (void)drv_nrf24l01_set_channel(2u);
    (void)drv_nrf24l01_set_address_width(5u);
    (void)drv_nrf24l01_set_tx_address(address, 5u);
    (void)drv_nrf24l01_set_rx_address(0u, address, 5u);
    drv_nrf24l01_set_auto_ack(DRV_NRF24L01_PIPE0);
    (void)drv_nrf24l01_set_auto_retransmit(3u, 10u);
    (void)drv_nrf24l01_set_rate_power(DRV_NRF24L01_RATE_1MBPS, DRV_NRF24L01_POWER_NEG6DBM);
    (void)drv_nrf24l01_enable_ack_payload(DRV_NRF24L01_PIPE0);
}

void main(void)
{
    stc8h_u8 status;
    stc8h_u8 width;

    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_spi_init();
    drv_nrf24l01_init_pins();
    configure_radio();
    drv_nrf24l01_enter_tx();

    while (1) {
        ++tx_payload[2];
        (void)drv_nrf24l01_write_payload(tx_payload, NRF24_PAYLOAD_SIZE);
        drv_nrf24l01_pulse_ce();
        status = drv_nrf24l01_read_status();

        if ((status & DRV_NRF24L01_STATUS_RX_READY) != 0u) {
            width = drv_nrf24l01_read_dynamic_payload_size();
            if ((width > 0u) && (width <= NRF24_PAYLOAD_SIZE)) {
                (void)drv_nrf24l01_read_payload(ack_payload, width);
                stc8h_uart_write_code(STC8H_UART1, "ack payload\r\n");
            }
        }
        if ((status & DRV_NRF24L01_STATUS_MAX_RETRY) != 0u) {
            drv_nrf24l01_flush_tx();
            stc8h_uart_write_code(STC8H_UART1, "max retry\r\n");
        }
        drv_nrf24l01_clear_irq(status);
    }
}
