#ifndef NRF24_PAIR_DIAG_LOGIC_H
#define NRF24_PAIR_DIAG_LOGIC_H

#include "stc8h_types.h"

#ifndef NRF24_PAIR_ACK_REPLACE_AFTER_RX
#define NRF24_PAIR_ACK_REPLACE_AFTER_RX 0u
#endif

#ifndef NRF24_PAIR_ACK_REPLACE_ON_RECOVER
#define NRF24_PAIR_ACK_REPLACE_ON_RECOVER 1u
#endif

#define NRF24_PAIR_DIAG_RATE_250KBPS 0u
#define NRF24_PAIR_DIAG_RATE_1MBPS 1u
#define NRF24_PAIR_DIAG_RATE_2MBPS 2u

#define NRF24_PAIR_DIAG_MATRIX_STAGE_COUNT 7u

typedef struct {
    stc8h_u8 have_counter;
    stc8h_u8 last_seq;
    stc8h_u16 last_tx_count;
    stc8h_u16 lost_count;
    stc8h_u16 dup_count;
    stc8h_u16 ptx_reset_count;
} nrf24_pair_diag_rx_stats_t;

typedef struct {
    stc8h_u8 rate;
    stc8h_u8 payload_size;
    stc8h_u8 ack_payload;
    stc8h_u8 dynamic_payload;
    stc8h_u8 ard_code;
    stc8h_u8 arc_count;
    stc8h_u16 packet_count;
} nrf24_pair_diag_matrix_stage_t;

static stc8h_u8 nrf24_pair_diag_matrix_stage_count(void)
{
    return NRF24_PAIR_DIAG_MATRIX_STAGE_COUNT;
}

static stc8h_u8 nrf24_pair_diag_matrix_stage_init(stc8h_u8 index,
                                                  nrf24_pair_diag_matrix_stage_t *stage)
{
    if (stage == 0) {
        return 0u;
    }

    stage->arc_count = 15u;
    switch (index) {
    case 0u:
        stage->rate = NRF24_PAIR_DIAG_RATE_1MBPS;
        stage->payload_size = 15u;
        stage->ack_payload = 1u;
        stage->dynamic_payload = 1u;
        stage->ard_code = 1u;
        stage->packet_count = 2000u;
        return 1u;
    case 1u:
        stage->rate = NRF24_PAIR_DIAG_RATE_1MBPS;
        stage->payload_size = 15u;
        stage->ack_payload = 0u;
        stage->dynamic_payload = 0u;
        stage->ard_code = 1u;
        stage->packet_count = 2000u;
        return 1u;
    case 2u:
        stage->rate = NRF24_PAIR_DIAG_RATE_250KBPS;
        stage->payload_size = 15u;
        stage->ack_payload = 1u;
        stage->dynamic_payload = 1u;
        stage->ard_code = 3u;
        stage->packet_count = 2000u;
        return 1u;
    case 3u:
        stage->rate = NRF24_PAIR_DIAG_RATE_250KBPS;
        stage->payload_size = 32u;
        stage->ack_payload = 1u;
        stage->dynamic_payload = 1u;
        stage->ard_code = 5u;
        stage->packet_count = 2000u;
        return 1u;
    case 4u:
        stage->rate = NRF24_PAIR_DIAG_RATE_250KBPS;
        stage->payload_size = 32u;
        stage->ack_payload = 1u;
        stage->dynamic_payload = 1u;
        stage->ard_code = 7u;
        stage->packet_count = 5000u;
        return 1u;
    case 5u:
        stage->rate = NRF24_PAIR_DIAG_RATE_250KBPS;
        stage->payload_size = 32u;
        stage->ack_payload = 1u;
        stage->dynamic_payload = 1u;
        stage->ard_code = 9u;
        stage->packet_count = 5000u;
        return 1u;
    case 6u:
        stage->rate = NRF24_PAIR_DIAG_RATE_2MBPS;
        stage->payload_size = 15u;
        stage->ack_payload = 0u;
        stage->dynamic_payload = 0u;
        stage->ard_code = 1u;
        stage->packet_count = 2000u;
        return 1u;
    default:
        break;
    }
    return 0u;
}

static void nrf24_pair_diag_rx_stats_init(nrf24_pair_diag_rx_stats_t *stats)
{
    stats->have_counter = 0u;
    stats->last_seq = 0u;
    stats->last_tx_count = 0u;
    stats->lost_count = 0u;
    stats->dup_count = 0u;
    stats->ptx_reset_count = 0u;
}

static void nrf24_pair_diag_rx_stats_update(nrf24_pair_diag_rx_stats_t *stats,
                                            stc8h_u8 seq,
                                            stc8h_u16 tx_count)
{
    stc8h_u16 expected;
    stc8h_u16 gap;

    if (stats->have_counter == 0u) {
        stats->have_counter = 1u;
        stats->last_seq = seq;
        stats->last_tx_count = tx_count;
        return;
    }

    if (tx_count == stats->last_tx_count) {
        ++stats->dup_count;
        stats->last_seq = seq;
        return;
    }

    expected = (stc8h_u16)(stats->last_tx_count + 1u);
    gap = (stc8h_u16)(tx_count - expected);
    if (gap < 0x8000u) {
        stats->lost_count = (stc8h_u16)(stats->lost_count + gap);
    } else {
        ++stats->ptx_reset_count;
    }

    stats->last_seq = seq;
    stats->last_tx_count = tx_count;
}

static stc8h_u8 nrf24_pair_diag_ack_replace_after_rx(void)
{
    return (stc8h_u8)NRF24_PAIR_ACK_REPLACE_AFTER_RX;
}

static stc8h_u8 nrf24_pair_diag_ack_replace_on_recover(void)
{
    return (stc8h_u8)NRF24_PAIR_ACK_REPLACE_ON_RECOVER;
}

#endif
