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

int main(void)
{
    int failures;

    failures = 0;
    failures += test_counter_reset_does_not_count_lost_packets();
    failures += test_forward_gap_counts_lost_packets();
    failures += test_duplicate_packet_counts_duplicate();
    failures += test_ack_payload_loaded_after_rx_does_not_replace_pending_fifo();

    return failures == 0 ? 0 : 1;
}
