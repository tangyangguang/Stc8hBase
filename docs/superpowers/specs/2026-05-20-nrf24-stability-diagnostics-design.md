# nRF24 Stability Diagnostics Design

## Goal

Make `drv_nrf24l01` a small reusable chip driver with enough checked helpers to diagnose PTX/PRX stability without depending on `Stc8hToyRemote` application code.

## Evidence

Nordic nRF24L01+ Product Specification v1.0 says CSN must start every SPI command, SPI shifts command/data MSB first, CE high for one PTX packet must be at least 10us, and leaving power down requires `Tpd2stby` before CE is raised. It also says DPL requires `FEATURE.EN_DPL` and `DYNPD`, ACK payload requires DPL, `TX_DS` and `RX_DR` are asserted together when a PTX receives ACK payload, `MAX_RT` leaves the TX payload in FIFO, and PRX ACK payload uses a three-level TX FIFO that can be blocked by stale pending payloads. The same spec requires longer ARD when ACK payloads are large, especially at 250kbps.

The STC8H manual confirms the current SPI group 0 pins are `SS/P1.2`, `MOSI/P1.3`, `MISO/P1.4`, `SCLK/P1.5`; `SPCTL=0xD0` means SS ignored, SPI enabled, MSB first, master, CPOL=0, CPHA=0, SYSclk/4; and XFR registers such as `P1IE/P3IE` require `P_SW2.EAXFR=1`.

## Driver Shape

Keep the driver register-oriented and compile-time configurable. Add only helpers that remove repeated, error-prone nRF24 state handling:

- separate RX pipe enable from auto-ack enable;
- classify a completed PTX transaction as pending, TX done, MAX_RT, ACK empty, ACK payload OK, or ACK payload invalid;
- read one dynamic RX packet with the datasheet-mandated width check and flush-on-invalid behavior;
- preload PRX ACK payload with an explicit replace-pending flag, so single-peer diagnostics can flush stale three-level FIFO entries before loading the latest ACK;
- recover to standby, PTX, or PRX by driving CE low, flushing FIFOs, clearing IRQs, then entering the requested mode.

Existing raw register and FIFO APIs stay available for diagnostics. No application payload format, binding protocol, display handling, motor/servo logic, or ToyRemote compatibility layer belongs in this driver.

## Diagnostic Shape

`nrf24_uart_diag` remains a single-board SPI/register diagnostic. It prints the PCB pinout, compile-time RF settings, repeated presence checks, key register dump, and FEATURE/DYNPD enable checks.

`nrf24_pair_diag` becomes the long-run RF diagnostic. The same source builds PTX or PRX with macros for channel, data rate, RF power, payload length, ACK payload, dynamic payload, ARD/ARC, send period, and summary interval. Defaults use the PCB pinout `CE=P1.6`, `CSN=P1.2`, `SCK=P1.5`, `MOSI=P1.3`, `MISO=P1.4`, `IRQ=P3.2` and UART1 logging only.

PTX prints per-summary totals: `tx_count`, `tx_ok`, `max_rt`, `ack_ok`, `ack_empty`, last `OBSERVE_TX`, `STATUS`, and `FIFO_STATUS`. PRX prints `rx_count`, last sequence, lost/duplicate counters, `STATUS`, `FIFO_STATUS`, and ACK preload counters. Optional per-packet logging can be enabled by macro when needed.

## Verification Matrix

Recommended starting points:

| Case | Data rate | Payload | ACK payload | Dynamic payload | ARD/ARC |
| --- | --- | --- | --- | --- | --- |
| A | 1Mbps | 15 | on | on | 500us / 15 |
| B | 1Mbps | 15 | off | off | 500us / 15 |
| C | 250kbps | 15 | on | on | 1000us / 15 |
| D | 250kbps | 32 | on | on | 1500us / 15 |
| E | 2Mbps | 15 | off | off | 500us / 15 |

Case D is expected to be the most sensitive to power and RF margin because 250kbps keeps the ACK packet on air much longer; the Nordic table requires 1500us ARD for all ACK payload sizes at 250kbps with 5-byte addresses. Cases with ACK payload off still use auto-ack unless explicitly disabled, so they isolate ACK-payload FIFO/timing problems from basic auto-ack link quality.

## PASS/FAIL Rules

Single-board PASS: `STATUS=0x0E` at idle or another non-0/non-0xFF sane value, presence checks `8/8 PASS`, and FEATURE/DYNPD checks print OK.

Pair PASS: recommended matrix cases run at least hundreds to thousands of packets with no repeating `MAX_RT` bursts. `OBSERVE_TX.ARC_CNT` should stay low in a clean short-range setup. ACK-payload cases should show stable `ack_ok`; no-ACK-payload cases should show `tx_ok` without requiring `ack_ok`.

FAIL direction: single-board failures point to SPI/pin/power/module wiring. Pair failures with single-board PASS point to RF settings, supply integrity, antenna/distance/interference, ACK payload timing, or PRX ACK FIFO handling.
