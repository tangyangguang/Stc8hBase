#include "drv_nrf24l01.h"
#include "stc8h_spi.h"
#include "stc8h_uart.h"

#define NRF24_PING_PACKET_SIZE 32u

static const stc8h_u8 address[5] = {'S', 'T', 'C', '0', '1'};
static stc8h_u8 payload[NRF24_PING_PACKET_SIZE];

static void print_status(stc8h_u8 status)
{
    if ((status & DRV_NRF24L01_STATUS_TX_DONE) != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "nrf24 tx ok\r\n");
    } else if ((status & DRV_NRF24L01_STATUS_MAX_RETRY) != 0u) {
        stc8h_uart_write_code(STC8H_UART1, "nrf24 max retry\r\n");
    } else {
        stc8h_uart_write_code(STC8H_UART1, "nrf24 tx pending\r\n");
    }
}

void main(void)
{
    stc8h_u8 status;

    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_spi_init();
    drv_nrf24l01_init_pins();

    payload[0] = 0xA5u;
    payload[1] = 0x01u;

    if (drv_nrf24l01_check_present() != STC8H_OK) {
        stc8h_uart_write_code(STC8H_UART1, "nrf24 absent\r\n");
    }

    (void)drv_nrf24l01_set_channel(2u);
    (void)drv_nrf24l01_set_address_width(5u);
    (void)drv_nrf24l01_set_tx_address(address, 5u);
    (void)drv_nrf24l01_set_rx_address(0u, address, 5u);
    (void)drv_nrf24l01_set_payload_size(0u, NRF24_PING_PACKET_SIZE);
    drv_nrf24l01_set_auto_ack(0x01u);
    (void)drv_nrf24l01_set_auto_retransmit(3u, 10u);
    (void)drv_nrf24l01_set_rate_power(DRV_NRF24L01_RATE_1MBPS, DRV_NRF24L01_POWER_NEG6DBM);
    drv_nrf24l01_enter_tx();

    while (1) {
        (void)drv_nrf24l01_write_payload(payload, NRF24_PING_PACKET_SIZE);
        drv_nrf24l01_pulse_ce();
        status = drv_nrf24l01_read_status();
        drv_nrf24l01_clear_irq(status);
        if ((status & DRV_NRF24L01_STATUS_MAX_RETRY) != 0u) {
            drv_nrf24l01_flush_tx();
        }
        print_status(status);
    }
}
