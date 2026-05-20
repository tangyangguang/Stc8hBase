# nRF24 Stability Diagnostics Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stabilize the reusable nRF24L01 driver and add standalone UART/pair diagnostics for the STC8H1K08 PCB pinout.

**Architecture:** Keep `drv_nrf24l01` as a low-level chip driver with small checked helpers for common state transitions and FIFO recovery. Keep diagnostics in `examples/platformio/nrf24_uart_diag` and `examples/platformio/nrf24_pair_diag`, with behavior selected by compile-time macros.

**Tech Stack:** C89/SDCC for STC8H, PlatformIO `intel_mcs51`, host C tests through `tools/check_host_tests.sh`.

---

### Task 1: Driver Behavior Tests

**Files:**
- Create: `tests/host/test_drv_nrf24l01_core.c`
- Modify: `tools/check_host_tests.sh`

- [x] Add a host fake SPI register model that can observe `EN_AA`, `EN_RXADDR`, `STATUS`, FIFO flush commands, dynamic payload width, RX payload bytes, and ACK payload commands.
- [x] Add failing tests for independent RX pipe enable, PTX result classification/recovery, dynamic RX width validation, and replace-pending ACK preload.
- [x] Register the test in `tools/check_host_tests.sh` and run it to verify the current driver fails before production code changes.

### Task 2: Driver API

**Files:**
- Modify: `drivers/drv_nrf24l01.h`
- Modify: `drivers/drv_nrf24l01.c`

- [x] Add `drv_nrf24l01_set_rx_pipes()`.
- [x] Change `drv_nrf24l01_set_auto_ack()` to touch only `EN_AA`; update examples to call `set_rx_pipes()` explicitly.
- [x] Add `drv_nrf24l01_complete_tx()`.
- [x] Add `drv_nrf24l01_read_rx_packet()`.
- [x] Add `drv_nrf24l01_preload_ack_payload()`.
- [x] Add `drv_nrf24l01_recover()`.
- [x] Run host tests and keep the small fixed-path example within its ROM guard.

### Task 3: Diagnostics

**Files:**
- Modify: `examples/platformio/nrf24_uart_diag/src/main.c`
- Modify: `examples/platformio/nrf24_pair_diag/src/main.c`
- Modify: `examples/platformio/nrf24_pair_diag/platformio.ini`

- [x] Add compile-time macro defaults for channel, data rate, power, payload length, ACK payload, dynamic payload, ARD/ARC, send period, summary interval, and optional per-packet logging.
- [x] Use the new driver helpers for PTX completion, PRX packet reading, ACK preload, and recovery.
- [x] Print summary statistics required for soak tests.
- [x] Keep UART1 as the only output and keep the PCB pinout in the boot banner.

### Task 4: Documentation and Validation

**Files:**
- Modify: `docs/21_NRF24L01_DESIGN.md`
- Modify: `docs/10_REFERENCES.md`
- Modify if needed: `tools/check_nrf24_examples.sh`

- [x] Record the API, diagnostics, verification matrix, PASS/FAIL rules, and PlatformIO build/upload commands.
- [x] Run `sh tools/check_host_tests.sh`.
- [x] Run `sh tools/check_nrf24_examples.sh`.
- [x] Run `sh tools/check_examples.sh`.
- [ ] Commit and push only `Stc8hBase` changes.
