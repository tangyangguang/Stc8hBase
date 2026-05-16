#ifndef PROTO_RF_LINK_H
#define PROTO_RF_LINK_H

#include "stc8h_config.h"

#define PROTO_RF_LINK_PACKET_SIZE 32u
#define PROTO_RF_LINK_HEADER_SIZE 9u
#define PROTO_RF_LINK_PAYLOAD_MAX 23u

#define PROTO_RF_LINK_MAGIC 0xA5u
#define PROTO_RF_LINK_VERSION 0x01u

#define PROTO_RF_LINK_FLAG_ACK_REQUIRED 0x01u

#ifndef PROTO_RF_LINK_ENABLE_RESET
#define PROTO_RF_LINK_ENABLE_RESET 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_TICK
#define PROTO_RF_LINK_ENABLE_TICK 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_CONNECT
#define PROTO_RF_LINK_ENABLE_CONNECT 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_SEND_DATA
#define PROTO_RF_LINK_ENABLE_SEND_DATA 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_SEND_STATUS
#define PROTO_RF_LINK_ENABLE_SEND_STATUS 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT
#define PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_POLL
#define PROTO_RF_LINK_ENABLE_POLL 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_GET_STATE
#define PROTO_RF_LINK_ENABLE_GET_STATE 1
#endif

typedef enum {
    PROTO_RF_LINK_PACKET_HELLO = 1,
    PROTO_RF_LINK_PACKET_HELLO_ACK = 2,
    PROTO_RF_LINK_PACKET_DATA = 3,
    PROTO_RF_LINK_PACKET_ACK = 4,
    PROTO_RF_LINK_PACKET_HEARTBEAT = 5,
    PROTO_RF_LINK_PACKET_STATUS = 6
} proto_rf_link_packet_type_t;

typedef enum {
    PROTO_RF_LINK_STATE_IDLE = 0,
    PROTO_RF_LINK_STATE_CONNECTING,
    PROTO_RF_LINK_STATE_CONNECTED,
    PROTO_RF_LINK_STATE_LOST
} proto_rf_link_state_t;

typedef enum {
    PROTO_RF_LINK_EVENT_NONE = 0,
    PROTO_RF_LINK_EVENT_CONNECTED,
    PROTO_RF_LINK_EVENT_LOST,
    PROTO_RF_LINK_EVENT_DATA,
    PROTO_RF_LINK_EVENT_STATUS
} proto_rf_link_event_t;

typedef struct {
    stc8h_u8 state;
    stc8h_u8 local_id;
    stc8h_u8 peer_id;
    stc8h_u8 seq_tx;
    stc8h_u8 seq_rx;
    stc8h_u8 ack_pending;
    stc8h_u16 timeout_ms;
    stc8h_u16 heartbeat_ms;
} proto_rf_link_t;

void proto_rf_link_init(proto_rf_link_t *link);
void proto_rf_link_set_ids(proto_rf_link_t *link, stc8h_u8 local_id, stc8h_u8 peer_id);
#if PROTO_RF_LINK_ENABLE_RESET
void proto_rf_link_reset(proto_rf_link_t *link);
#endif
#if PROTO_RF_LINK_ENABLE_TICK
void proto_rf_link_tick(proto_rf_link_t *link, stc8h_u16 elapsed_ms);
#endif

#if PROTO_RF_LINK_ENABLE_CONNECT
stc8h_status_t proto_rf_link_connect(proto_rf_link_t *link, stc8h_u8 *packet);
#endif
#if PROTO_RF_LINK_ENABLE_SEND_DATA
stc8h_status_t proto_rf_link_send_data(proto_rf_link_t *link, stc8h_u8 *packet, const stc8h_u8 *data, stc8h_u8 len);
#endif
#if PROTO_RF_LINK_ENABLE_SEND_STATUS
stc8h_status_t proto_rf_link_send_status(proto_rf_link_t *link, stc8h_u8 *packet, const stc8h_u8 *data, stc8h_u8 len);
#endif
#if PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT
stc8h_status_t proto_rf_link_send_heartbeat(proto_rf_link_t *link, stc8h_u8 *packet);
#endif

#if PROTO_RF_LINK_ENABLE_POLL
proto_rf_link_event_t proto_rf_link_poll(proto_rf_link_t *link, const stc8h_u8 *packet, stc8h_u8 *type, stc8h_u8 *data, stc8h_u8 *len);
#endif
#if PROTO_RF_LINK_ENABLE_GET_STATE
proto_rf_link_state_t proto_rf_link_get_state(const proto_rf_link_t *link);
#endif

#endif
