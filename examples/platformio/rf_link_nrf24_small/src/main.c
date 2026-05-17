#include "drv_nrf24l01.h"
#include "proto_rf_link.h"
#include "stc8h_spi.h"

#define RF_LINK_CHANNEL 40u
#define RF_LINK_ADDR_LEN 5u

static const stc8h_u8 rf_link_addr[RF_LINK_ADDR_LEN] = {'T', 'O', 'Y', 'R', '1'};
static proto_rf_link_t link;
static stc8h_u8 packet[PROTO_RF_LINK_PACKET_SIZE];
static stc8h_u8 payload[PROTO_RF_LINK_FIXED_PAYLOAD_LEN] = {
    1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u
};

static stc8h_status_t radio_init(void)
{
    drv_nrf24l01_init_pins();
    drv_nrf24l01_power_down();
    drv_nrf24l01_flush_tx();
    drv_nrf24l01_flush_rx();
    drv_nrf24l01_clear_irq(0x70u);

    if (drv_nrf24l01_check_present() != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (drv_nrf24l01_set_channel(RF_LINK_CHANNEL) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (drv_nrf24l01_config_pipe0_fixed(rf_link_addr) != STC8H_OK) {
        return STC8H_ERROR;
    }

    drv_nrf24l01_set_auto_ack(DRV_NRF24L01_PIPE0);
    if (drv_nrf24l01_set_auto_retransmit(3u, 10u) != STC8H_OK) {
        return STC8H_ERROR;
    }
    if (drv_nrf24l01_set_rate_power(DRV_NRF24L01_RATE_1MBPS, DRV_NRF24L01_POWER_0DBM) != STC8H_OK) {
        return STC8H_ERROR;
    }

    drv_nrf24l01_enter_tx();
    return STC8H_OK;
}

void main(void)
{
    stc8h_spi_init();
    proto_rf_link_init(&link);
    proto_rf_link_set_ids(&link, 1u, 2u);
    (void)radio_init();

    while (1) {
        if (proto_rf_link_send_data_fixed(&link, packet, payload) == STC8H_OK) {
            (void)drv_nrf24l01_write_payload(packet, PROTO_RF_LINK_PACKET_SIZE);
            drv_nrf24l01_pulse_ce();
        }
        (void)proto_rf_link_poll_data_fixed(&link, packet, payload);
    }
}
