# nRF24 Pin Initialization Design

## Goal

Fix the remaining ToyRemote `radio_diag` failure where nRF24L01 register write/read still stops at `C000` after SPI MISO input enable was added.

## Evidence

The generated ToyRemote `radio_diag` firmware now contains `P_SW2 |= 0x80`, `P1IE |= 0x10`, and `SPCTL=0xD0`, so the previous MISO input-enable fix is present in the burned image.

The STC8H manual states that most STC8H I/O pins power up in high-impedance input mode, unlike traditional 8051 quasi-bidirectional reset behavior. The same GPIO chapter states that quasi-bidirectional pins should latch `1` before reading external state. Legacy ToyRemote called `IO_Init()` and set `P1M1/P1M0` to `0x00`, so CE/CSN/SPI pins were output-capable quasi-bidirectional before nRF24 access.

Current ToyRemote only writes CE/CSN latch bits through `drv_nrf24l01_init_pins()`. Without a board-level mode hook, `CE=P1.6` and `CSN=P1.2` may remain high-impedance, leaving the nRF24 unselected or unstable even though SPI transfers run.

## Design

Keep `drv_nrf24l01` as a chip driver with board macros, not a runtime GPIO owner. Add optional `DRV_NRF24L01_CONFIGURE_PINS()` with a default empty implementation. `drv_nrf24l01_init_pins()` calls this hook before driving `CE_LOW` and `CSN_HIGH`.

Keep SPI HAL responsible for SPI pins. In addition to configuring MOSI/MISO/SCLK as quasi-bidirectional and enabling `PxIE`, SPI init releases the selected MISO latch before reading external MISO. Add `STC8H_SPI_LATCH_MISO_HIGH` so unusual board policies can opt out.

ToyRemote should define `DRV_NRF24L01_CONFIGURE_PINS()` in board pin headers to configure P1.2/P1.6 as quasi-bidirectional, release both latches, then drive CE low and leave CSN high.

## Verification

Add host coverage proving `drv_nrf24l01_init_pins()` invokes the board hook before CE/CSN writes. Extend SPI codegen checks to require the MISO latch write for all four SPI pin groups. ToyRemote adds a codegen check that radio_diag and receiver firmware contain CE/CSN P1M0/P1M1 quasi-bidirectional setup and idle latch writes.
