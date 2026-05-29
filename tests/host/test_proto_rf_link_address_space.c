#include <stdio.h>
#include <string.h>

#define PROTO_RF_LINK_ENABLE_RESET 0
#define PROTO_RF_LINK_ENABLE_TICK 0
#define PROTO_RF_LINK_ENABLE_CONNECT 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA 0
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED 1
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_FAST_PATH 1
#define PROTO_RF_LINK_ENABLE_XDATA_FIXED_API 1
#define PROTO_RF_LINK_ENABLE_PACKET_ARG_CHECK 1
#define PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS 0
#define PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS 0
#define PROTO_RF_LINK_TRACK_STATE 0
#define PROTO_RF_LINK_TRACK_SEQ_RX 1
#define PROTO_RF_LINK_TRACK_ACK_PENDING 1
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_TRACK_ACK 1
#define PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED_TRACK_LINK 1
#define PROTO_RF_LINK_ENABLE_SEND_STATUS 0
#define PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT 0
#define PROTO_RF_LINK_ENABLE_POLL 0
#define PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED 1
#define PROTO_RF_LINK_ENABLE_GET_STATE 0

#include "../../protocols/proto_rf_link.c"

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static void fill_payload(stc8h_u8 *payload)
{
    stc8h_u8 i;

    for (i = 0u; i < PROTO_RF_LINK_FIXED_PAYLOAD_LEN; ++i) {
        payload[i] = (stc8h_u8)(0x30u + i);
    }
}

static int test_xdata_fixed_send_matches_generic_fixed_send(void)
{
    int failures;
    proto_rf_link_t generic_link;
    proto_rf_link_t xdata_link;
    stc8h_u8 generic_packet[PROTO_RF_LINK_PACKET_SIZE];
    stc8h_u8 xdata_packet[PROTO_RF_LINK_PACKET_SIZE];
    stc8h_u8 payload[PROTO_RF_LINK_FIXED_PAYLOAD_LEN];
    stc8h_status_t generic_status;
    stc8h_status_t xdata_status;

    failures = 0;
    fill_payload(payload);
    memset(generic_packet, 0xAA, sizeof(generic_packet));
    memset(xdata_packet, 0x55, sizeof(xdata_packet));

    proto_rf_link_init(&generic_link);
    proto_rf_link_set_ids(&generic_link, 0x11u, 0x22u);
    generic_link.seq_rx = 0x44u;

    proto_rf_link_init_xdata(&xdata_link);
    proto_rf_link_set_ids_xdata(&xdata_link, 0x11u, 0x22u);
    xdata_link.seq_rx = 0x44u;

    generic_status = proto_rf_link_send_data_fixed(&generic_link, generic_packet, payload);
    xdata_status = proto_rf_link_send_data_fixed_xdata(&xdata_link, xdata_packet, payload);

    failures += require(generic_status == STC8H_OK, "generic fixed send must succeed");
    failures += require(xdata_status == STC8H_OK, "xdata fixed send must succeed");
    failures += require(memcmp(generic_packet, xdata_packet, sizeof(generic_packet)) == 0,
                        "xdata fixed send must produce the same packet bytes as generic fixed send");
    failures += require(generic_link.seq_tx == xdata_link.seq_tx,
                        "xdata fixed send must advance seq_tx like generic fixed send");
    failures += require(xdata_link.ack_pending == 1u,
                        "xdata fixed send must preserve fixed ACK tracking behavior");

    return failures;
}

static int test_xdata_fixed_poll_matches_generic_fixed_poll(void)
{
    int failures;
    proto_rf_link_t tx_link;
    proto_rf_link_t generic_rx;
    proto_rf_link_t xdata_rx;
    stc8h_u8 packet[PROTO_RF_LINK_PACKET_SIZE];
    stc8h_u8 payload[PROTO_RF_LINK_FIXED_PAYLOAD_LEN];
    stc8h_u8 generic_out[PROTO_RF_LINK_FIXED_PAYLOAD_LEN];
    stc8h_u8 xdata_out[PROTO_RF_LINK_FIXED_PAYLOAD_LEN];
    stc8h_status_t generic_status;
    stc8h_status_t xdata_status;

    failures = 0;
    fill_payload(payload);
    memset(generic_out, 0, sizeof(generic_out));
    memset(xdata_out, 0, sizeof(xdata_out));

    proto_rf_link_init(&tx_link);
    proto_rf_link_set_ids(&tx_link, 0x11u, 0x22u);
    (void)proto_rf_link_send_data_fixed(&tx_link, packet, payload);

    proto_rf_link_init(&generic_rx);
    proto_rf_link_set_ids(&generic_rx, 0x22u, 0x11u);
    generic_rx.ack_pending = 1u;

    proto_rf_link_init_xdata(&xdata_rx);
    proto_rf_link_set_ids_xdata(&xdata_rx, 0x22u, 0x11u);
    xdata_rx.ack_pending = 1u;

    generic_status = proto_rf_link_poll_data_fixed(&generic_rx, packet, generic_out);
    xdata_status = proto_rf_link_poll_data_fixed_xdata(&xdata_rx, packet, xdata_out);

    failures += require(generic_status == STC8H_OK, "generic fixed poll must accept fixed DATA packet");
    failures += require(xdata_status == STC8H_OK, "xdata fixed poll must accept fixed DATA packet");
    failures += require(memcmp(generic_out, xdata_out, sizeof(generic_out)) == 0,
                        "xdata fixed poll must copy the same payload bytes as generic fixed poll");
    failures += require(memcmp(payload, xdata_out, sizeof(payload)) == 0,
                        "xdata fixed poll must copy the original payload");
    failures += require(generic_rx.seq_rx == xdata_rx.seq_rx,
                        "xdata fixed poll must update seq_rx like generic fixed poll");
    failures += require(xdata_rx.ack_pending == 0u,
                        "xdata fixed poll must preserve fixed link tracking behavior");

    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_xdata_fixed_send_matches_generic_fixed_send();
    failures += test_xdata_fixed_poll_matches_generic_fixed_poll();

    return failures == 0 ? 0 : 1;
}
