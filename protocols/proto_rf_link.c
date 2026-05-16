#include "proto_rf_link.h"

#ifndef PROTO_RF_LINK_LOST_TIMEOUT_MS
#define PROTO_RF_LINK_LOST_TIMEOUT_MS 1000u
#endif

#if PROTO_RF_LINK_ENABLE_CONNECT || PROTO_RF_LINK_ENABLE_SEND_DATA || PROTO_RF_LINK_ENABLE_SEND_STATUS || PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT
static void proto_rf_link_clear_packet(stc8h_u8 *packet)
{
    stc8h_u8 i;

    for (i = 0u; i < PROTO_RF_LINK_PACKET_SIZE; ++i) {
        packet[i] = 0u;
    }
}

static stc8h_status_t proto_rf_link_make_packet(proto_rf_link_t *link, stc8h_u8 *packet, stc8h_u8 type, stc8h_u8 flags, const stc8h_u8 *data, stc8h_u8 len)
{
    stc8h_u8 i;

    if ((link == 0) || (packet == 0) || (len > PROTO_RF_LINK_PAYLOAD_MAX) || ((len != 0u) && (data == 0))) {
        return STC8H_ERROR;
    }

    proto_rf_link_clear_packet(packet);
    packet[0] = PROTO_RF_LINK_MAGIC;
    packet[1] = PROTO_RF_LINK_VERSION;
    packet[2] = type;
    packet[3] = link->seq_tx;
    packet[4] = link->seq_rx;
    packet[5] = flags;
    packet[6] = link->local_id;
    packet[7] = link->peer_id;
    packet[8] = len;

    for (i = 0u; i < len; ++i) {
        packet[PROTO_RF_LINK_HEADER_SIZE + i] = data[i];
    }

    ++link->seq_tx;
    return STC8H_OK;
}
#endif

void proto_rf_link_init(proto_rf_link_t *link)
{
    if (link == 0) {
        return;
    }

    link->state = PROTO_RF_LINK_STATE_IDLE;
    link->local_id = 0u;
    link->peer_id = 0u;
    link->seq_tx = 0u;
    link->seq_rx = 0u;
    link->ack_pending = 0u;
    link->timeout_ms = 0u;
    link->heartbeat_ms = 0u;
}

void proto_rf_link_set_ids(proto_rf_link_t *link, stc8h_u8 local_id, stc8h_u8 peer_id)
{
    if (link == 0) {
        return;
    }

    link->local_id = local_id;
    link->peer_id = peer_id;
}

#if PROTO_RF_LINK_ENABLE_RESET
void proto_rf_link_reset(proto_rf_link_t *link)
{
    if (link == 0) {
        return;
    }

    link->state = PROTO_RF_LINK_STATE_IDLE;
    link->seq_tx = 0u;
    link->seq_rx = 0u;
    link->ack_pending = 0u;
    link->timeout_ms = 0u;
    link->heartbeat_ms = 0u;
}
#endif

#if PROTO_RF_LINK_ENABLE_TICK
void proto_rf_link_tick(proto_rf_link_t *link, stc8h_u16 elapsed_ms)
{
    if (link == 0) {
        return;
    }

    if (link->state == PROTO_RF_LINK_STATE_CONNECTED) {
        link->timeout_ms = (stc8h_u16)(link->timeout_ms + elapsed_ms);
        link->heartbeat_ms = (stc8h_u16)(link->heartbeat_ms + elapsed_ms);
        if (link->timeout_ms >= PROTO_RF_LINK_LOST_TIMEOUT_MS) {
            link->state = PROTO_RF_LINK_STATE_LOST;
        }
    }
}
#endif

#if PROTO_RF_LINK_ENABLE_CONNECT
stc8h_status_t proto_rf_link_connect(proto_rf_link_t *link, stc8h_u8 *packet)
{
    stc8h_status_t status;

    status = proto_rf_link_make_packet(link, packet, PROTO_RF_LINK_PACKET_HELLO, PROTO_RF_LINK_FLAG_ACK_REQUIRED, 0, 0u);
    if (status == STC8H_OK) {
        link->state = PROTO_RF_LINK_STATE_CONNECTING;
        link->ack_pending = 1u;
    }
    return status;
}
#endif

#if PROTO_RF_LINK_ENABLE_SEND_DATA
stc8h_status_t proto_rf_link_send_data(proto_rf_link_t *link, stc8h_u8 *packet, const stc8h_u8 *data, stc8h_u8 len)
{
    stc8h_status_t status;

    status = proto_rf_link_make_packet(link, packet, PROTO_RF_LINK_PACKET_DATA, PROTO_RF_LINK_FLAG_ACK_REQUIRED, data, len);
    if (status == STC8H_OK) {
        link->ack_pending = 1u;
    }
    return status;
}
#endif

#if PROTO_RF_LINK_ENABLE_SEND_STATUS
stc8h_status_t proto_rf_link_send_status(proto_rf_link_t *link, stc8h_u8 *packet, const stc8h_u8 *data, stc8h_u8 len)
{
    return proto_rf_link_make_packet(link, packet, PROTO_RF_LINK_PACKET_STATUS, 0u, data, len);
}
#endif

#if PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT
stc8h_status_t proto_rf_link_send_heartbeat(proto_rf_link_t *link, stc8h_u8 *packet)
{
    stc8h_status_t status;

    status = proto_rf_link_make_packet(link, packet, PROTO_RF_LINK_PACKET_HEARTBEAT, 0u, 0, 0u);
    if (status == STC8H_OK) {
        link->heartbeat_ms = 0u;
    }
    return status;
}
#endif

#if PROTO_RF_LINK_ENABLE_POLL
proto_rf_link_event_t proto_rf_link_poll(proto_rf_link_t *link, const stc8h_u8 *packet, stc8h_u8 *type, stc8h_u8 *data, stc8h_u8 *len)
{
    stc8h_u8 i;
    stc8h_u8 payload_len;
    stc8h_u8 packet_type;

    if ((link == 0) || (packet == 0) || (len == 0)) {
        return PROTO_RF_LINK_EVENT_NONE;
    }
    if ((packet[0] != PROTO_RF_LINK_MAGIC) || (packet[1] != PROTO_RF_LINK_VERSION)) {
        return PROTO_RF_LINK_EVENT_NONE;
    }
    if ((packet[7] != link->local_id) || (packet[6] != link->peer_id)) {
        return PROTO_RF_LINK_EVENT_NONE;
    }

    payload_len = packet[8];
    if (payload_len > PROTO_RF_LINK_PAYLOAD_MAX) {
        return PROTO_RF_LINK_EVENT_NONE;
    }

    packet_type = packet[2];
    if (type != 0) {
        *type = packet_type;
    }
    if (data != 0) {
        for (i = 0u; i < payload_len; ++i) {
            data[i] = packet[PROTO_RF_LINK_HEADER_SIZE + i];
        }
    }
    *len = payload_len;

    link->seq_rx = packet[3];
    link->ack_pending = 0u;
    link->timeout_ms = 0u;

    if ((packet_type == PROTO_RF_LINK_PACKET_HELLO_ACK) || (packet_type == PROTO_RF_LINK_PACKET_HELLO)) {
        link->state = PROTO_RF_LINK_STATE_CONNECTED;
        return PROTO_RF_LINK_EVENT_CONNECTED;
    }
    if (packet_type == PROTO_RF_LINK_PACKET_DATA) {
        link->state = PROTO_RF_LINK_STATE_CONNECTED;
        return PROTO_RF_LINK_EVENT_DATA;
    }
    if (packet_type == PROTO_RF_LINK_PACKET_STATUS) {
        link->state = PROTO_RF_LINK_STATE_CONNECTED;
        return PROTO_RF_LINK_EVENT_STATUS;
    }
    if (packet_type == PROTO_RF_LINK_PACKET_HEARTBEAT) {
        link->state = PROTO_RF_LINK_STATE_CONNECTED;
    }

    return PROTO_RF_LINK_EVENT_NONE;
}
#endif

#if PROTO_RF_LINK_ENABLE_GET_STATE
proto_rf_link_state_t proto_rf_link_get_state(const proto_rf_link_t *link)
{
    if (link == 0) {
        return PROTO_RF_LINK_STATE_IDLE;
    }
    return (proto_rf_link_state_t)link->state;
}
#endif
