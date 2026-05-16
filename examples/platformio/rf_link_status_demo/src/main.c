#include "proto_rf_link.h"

static proto_rf_link_t link;
static stc8h_u8 packet[PROTO_RF_LINK_PACKET_SIZE];
static stc8h_u8 payload[3] = {1u, 2u, 3u};
static stc8h_u8 out_type;
static stc8h_u8 out_len;

void main(void)
{
    proto_rf_link_init(&link);
    proto_rf_link_set_ids(&link, 1u, 2u);

    (void)proto_rf_link_connect(&link, packet);
    (void)proto_rf_link_send_data(&link, packet, payload, sizeof(payload));
    (void)proto_rf_link_poll(&link, packet, &out_type, payload, &out_len);

    while (1) {
        proto_rf_link_tick(&link, 10u);
    }
}
