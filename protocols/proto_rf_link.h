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

#ifndef PROTO_RF_LINK_ENABLE_SET_IDS
#define PROTO_RF_LINK_ENABLE_SET_IDS 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_PACKET_ARG_CHECK
#define PROTO_RF_LINK_ENABLE_PACKET_ARG_CHECK 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS
#define PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS 1
#endif

#ifndef PROTO_RF_LINK_TRACK_STATE
#define PROTO_RF_LINK_TRACK_STATE 1
#endif

#ifndef PROTO_RF_LINK_TRACK_SEQ_RX
#define PROTO_RF_LINK_TRACK_SEQ_RX 1
#endif

#ifndef PROTO_RF_LINK_TRACK_ACK_PENDING
#define PROTO_RF_LINK_TRACK_ACK_PENDING 1
#endif

/* Include timeout_ms / heartbeat_ms in the proto_rf_link_t struct.
 * Default 1 keeps existing ABI. Apps that never tick the link and
 * never read those fields can set this to 0 to save 4 bytes of RAM
 * per link instance. When 0, all _TICK / _TIMEOUT bookkeeping is
 * automatically off and any code referencing the fields must also
 * be gated by this same macro. */
#ifndef PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS
#define PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS 1
#endif

#ifndef PROTO_RF_LINK_FIXED_PAYLOAD_LEN
#define PROTO_RF_LINK_FIXED_PAYLOAD_LEN 11u
#endif

#ifndef PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED 0
#endif

#ifndef PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED
#define PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED 0
#endif

#ifndef PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_TRACK_ACK
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_TRACK_ACK 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED_TRACK_LINK
#define PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED_TRACK_LINK 1
#endif

#if PROTO_RF_LINK_FIXED_PAYLOAD_LEN > PROTO_RF_LINK_PAYLOAD_MAX
#error "PROTO_RF_LINK_FIXED_PAYLOAD_LEN must be <= PROTO_RF_LINK_PAYLOAD_MAX."
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

#if !PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS
#if PROTO_RF_LINK_ENABLE_TICK
#error "PROTO_RF_LINK_ENABLE_TICK requires PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS."
#endif
#if PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS
#error "PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS requires PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS."
#endif
#if PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT
#error "PROTO_RF_LINK_ENABLE_SEND_HEARTBEAT requires PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS."
#endif
#endif

#if !PROTO_RF_LINK_TRACK_STATE && PROTO_RF_LINK_ENABLE_TICK
#error "PROTO_RF_LINK_ENABLE_TICK requires PROTO_RF_LINK_TRACK_STATE."
#endif

#if !PROTO_RF_LINK_TRACK_STATE && PROTO_RF_LINK_ENABLE_GET_STATE
#error "PROTO_RF_LINK_ENABLE_GET_STATE requires PROTO_RF_LINK_TRACK_STATE."
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
#if PROTO_RF_LINK_TRACK_STATE
    stc8h_u8 state;
#endif
    stc8h_u8 local_id;
    stc8h_u8 peer_id;
    stc8h_u8 seq_tx;
#if PROTO_RF_LINK_TRACK_SEQ_RX
    stc8h_u8 seq_rx;
#endif
#if PROTO_RF_LINK_TRACK_ACK_PENDING
    stc8h_u8 ack_pending;
#endif
#if PROTO_RF_LINK_INCLUDE_TIMEOUT_FIELDS
    stc8h_u16 timeout_ms;
    stc8h_u16 heartbeat_ms;
#endif
} proto_rf_link_t;

void proto_rf_link_init(proto_rf_link_t *link);
#if PROTO_RF_LINK_ENABLE_SET_IDS
void proto_rf_link_set_ids(proto_rf_link_t *link, stc8h_u8 local_id, stc8h_u8 peer_id);
#endif
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
#if PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED
stc8h_status_t proto_rf_link_send_data_fixed(proto_rf_link_t *link, stc8h_u8 *packet, const stc8h_u8 *data);
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
#if PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED
stc8h_status_t proto_rf_link_poll_data_fixed(proto_rf_link_t *link, const stc8h_u8 *packet, stc8h_u8 *data);
#endif
#if PROTO_RF_LINK_ENABLE_GET_STATE
proto_rf_link_state_t proto_rf_link_get_state(const proto_rf_link_t *link);
#endif

#endif
