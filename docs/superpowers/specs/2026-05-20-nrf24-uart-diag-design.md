# nRF24 UART Diagnostics Design

## Goal

Add a foundation-library nRF24L01 diagnostic firmware that runs on the ToyRemote PCB pinout, reports through UART only, and isolates SPI/register/module health from the ToyRemote application protocol.

## Evidence

ToyRemote `radio_diag` now reaches `C001`, so nRF24 register write/read can pass. It can also briefly show `F076`, which means a valid ACK payload was received once. The remaining `L076` means later packets hit MAX_RT and no ACK arrived. This is no longer enough evidence to keep changing the application layer because RF state, receiver configuration, ACK payload FIFO state, channel binding, and module power can all produce the same display code.

Stc8hBase already records that STC8H SPI should keep MOSI/MISO/SCLK quasi-bidirectional, release MISO, and enable the selected MISO `PxIE`. It also records nRF24 ACK payload rules: ACK payload requires dynamic payload, PRX ACK payload occupies the TX FIFO, and MAX_RT paths must clear IRQ and often flush TX.

## Design

Create `examples/platformio/nrf24_uart_diag` as a single-board diagnostic example. The example uses the ToyRemote PCB nRF24 wiring: CE=P1.6, CSN=P1.2, SCK=P1.5, MOSI=P1.3, MISO=P1.4, IRQ=P3.2. UART1 on P3.0/P3.1 is the only output.

The diagnostic first drives CE/CSN to idle before SPI init, then initializes UART and SPI. It prints a boot banner, reads STATUS repeatedly, runs repeated `drv_nrf24l01_check_present()` write/read tests, dumps key registers, and verifies that dynamic payload plus ACK payload FEATURE/DYNPD bits can be enabled. It does not use TM1637, buttons, motors, EEPROM, receiver firmware, or ToyRemote protocol.

Keep the nRF24 driver as a chip driver. Pin ownership remains in board macros. Add no runtime pin abstraction and no software SPI.

## Related Fixes

Update the demo board nRF24 pin macros to define `DRV_NRF24L01_CONFIGURE_PINS()` for CE/CSN quasi-bidirectional mode and idle latches. Existing nRF24 examples must call `drv_nrf24l01_init_pins()` before `stc8h_spi_init()` so CSN cannot float during early startup.

## Verification

Add a codegen check for every PlatformIO nRF24 example. It must verify that generated main assembly calls `drv_nrf24l01_init_pins` before `stc8h_spi_init`, and that generated nRF24 wrapper code contains the CE/CSN P1 mode and idle latch operations. Run host tests and all examples after implementation.
