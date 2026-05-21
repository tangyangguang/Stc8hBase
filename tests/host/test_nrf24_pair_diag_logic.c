#include <stdio.h>
#include <string.h>

#include "../../examples/platformio/nrf24_pair_diag/src/nrf24_pair_diag_logic.h"

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

static int test_counter_reset_does_not_count_lost_packets(void)
{
    int failures;
    nrf24_pair_diag_rx_stats_t stats;

    failures = 0;
    nrf24_pair_diag_rx_stats_init(&stats);

    nrf24_pair_diag_rx_stats_update(&stats, 0x1Au, 26u);
    nrf24_pair_diag_rx_stats_update(&stats, 0x1Bu, 27u);
    nrf24_pair_diag_rx_stats_update(&stats, 0x01u, 1u);
    nrf24_pair_diag_rx_stats_update(&stats, 0x02u, 2u);

    failures += require(stats.lost_count == 0u,
                        "PTX counter reset must resync instead of adding lost packets");
    failures += require(stats.ptx_reset_count == 1u,
                        "PTX counter reset must be counted separately");
    failures += require(stats.dup_count == 0u,
                        "PTX counter reset must not be counted as duplicate");
    failures += require(stats.last_seq == 0x02u,
                        "PTX counter reset path must keep tracking the new stream");
    return failures;
}

static int test_forward_gap_counts_lost_packets(void)
{
    int failures;
    nrf24_pair_diag_rx_stats_t stats;

    failures = 0;
    nrf24_pair_diag_rx_stats_init(&stats);

    nrf24_pair_diag_rx_stats_update(&stats, 0x10u, 16u);
    nrf24_pair_diag_rx_stats_update(&stats, 0x13u, 19u);

    failures += require(stats.lost_count == 2u,
                        "forward TX counter gap must count missing packets");
    failures += require(stats.ptx_reset_count == 0u,
                        "forward TX counter gap must not be counted as reset");
    return failures;
}

static int test_duplicate_packet_counts_duplicate(void)
{
    int failures;
    nrf24_pair_diag_rx_stats_t stats;

    failures = 0;
    nrf24_pair_diag_rx_stats_init(&stats);

    nrf24_pair_diag_rx_stats_update(&stats, 0x20u, 32u);
    nrf24_pair_diag_rx_stats_update(&stats, 0x20u, 32u);

    failures += require(stats.dup_count == 1u,
                        "same TX counter must be counted as duplicate");
    failures += require(stats.lost_count == 0u,
                        "duplicate packet must not add lost packets");
    return failures;
}

static int test_ack_payload_loaded_after_rx_does_not_replace_pending_fifo(void)
{
    int failures;

    failures = 0;
    failures += require(nrf24_pair_diag_ack_replace_after_rx() == 0u,
                        "ACK preload after RX must append, not FLUSH_TX/replace");
    failures += require(nrf24_pair_diag_ack_replace_on_recover() == 1u,
                        "ACK preload during recovery may replace stale FIFO");
    return failures;
}

static int has_matrix_stage(stc8h_u8 rate, stc8h_u8 payload_size, stc8h_u8 ack_payload,
                            stc8h_u8 dynamic_payload, stc8h_u8 ard_code,
                            stc8h_u16 packet_count)
{
    stc8h_u8 i;
    nrf24_pair_diag_matrix_stage_t stage;

    for (i = 0u; i < nrf24_pair_diag_matrix_stage_count(); ++i) {
        if ((nrf24_pair_diag_matrix_stage_init(i, &stage) != 0u) &&
            (stage.rate == rate) &&
            (stage.payload_size == payload_size) &&
            (stage.ack_payload == ack_payload) &&
            (stage.dynamic_payload == dynamic_payload) &&
            (stage.ard_code == ard_code) &&
            (stage.packet_count == packet_count)) {
            return 1;
        }
    }
    return 0;
}

static int test_fast_matrix_covers_required_conditions(void)
{
    int failures;

    failures = 0;
    failures += require(nrf24_pair_diag_matrix_stage_count() == 7u,
                        "fast matrix must cover all seven required soak-test stages");
    failures += require(has_matrix_stage(NRF24_PAIR_DIAG_RATE_1MBPS, 15u, 1u, 1u, 1u, 2000u) != 0,
                        "matrix must include 1Mbps + 15-byte ACK payload at 500us ARD");
    failures += require(has_matrix_stage(NRF24_PAIR_DIAG_RATE_1MBPS, 15u, 0u, 0u, 1u, 2000u) != 0,
                        "matrix must include 1Mbps + no ACK payload at 500us ARD");
    failures += require(has_matrix_stage(NRF24_PAIR_DIAG_RATE_250KBPS, 15u, 1u, 1u, 3u, 2000u) != 0,
                        "matrix must include 250kbps + 15-byte ACK payload at 1000us ARD");
    failures += require(has_matrix_stage(NRF24_PAIR_DIAG_RATE_250KBPS, 32u, 1u, 1u, 5u, 2000u) != 0,
                        "matrix must include 250kbps + 32-byte ACK payload at 1500us ARD");
    failures += require(has_matrix_stage(NRF24_PAIR_DIAG_RATE_250KBPS, 32u, 1u, 1u, 7u, 5000u) != 0,
                        "matrix must include 250kbps + 32-byte ACK payload at 2000us ARD");
    failures += require(has_matrix_stage(NRF24_PAIR_DIAG_RATE_250KBPS, 32u, 1u, 1u, 9u, 5000u) != 0,
                        "matrix must include 250kbps + 32-byte ACK payload at 2500us ARD");
    failures += require(has_matrix_stage(NRF24_PAIR_DIAG_RATE_2MBPS, 15u, 0u, 0u, 1u, 2000u) != 0,
                        "matrix must include 2Mbps + no ACK payload at 500us ARD");
    return failures;
}

static int test_fast_matrix_uses_warmup_packets_outside_statistics(void)
{
    int failures;

    failures = 0;
    failures += require(nrf24_pair_diag_matrix_warmup_packets() == 16u,
                        "fast matrix must use 16 warmup packets before counted statistics");
    return failures;
}

int main(void)
{
    int failures;

    failures = 0;
    failures += test_counter_reset_does_not_count_lost_packets();
    failures += test_forward_gap_counts_lost_packets();
    failures += test_duplicate_packet_counts_duplicate();
    failures += test_ack_payload_loaded_after_rx_does_not_replace_pending_fifo();
    failures += test_fast_matrix_covers_required_conditions();
    failures += test_fast_matrix_uses_warmup_packets_outside_statistics();

    return failures == 0 ? 0 : 1;
}
