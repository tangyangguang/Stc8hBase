# SPI MISO Input Enable Design

## Goal

Make the hardware SPI HAL reliably read MISO on STC8H, fixing the nRF24L01 case where STATUS and register readback return `0x00` even though the same hardware works with the legacy firmware.

## Evidence

The archived STC8H manual lists SPI pin selection in `P_SW1[3:2]`: group 0 is `SS/P1.2`, `MOSI/P1.3`, `MISO/P1.4`, `SCLK/P1.5`; group 1 is `P2.2/P2.3/P2.4/P2.5`; group 2 is `P5.4/P4.0/P4.1/P4.3`; group 3 is `P3.5/P3.4/P3.3/P3.2`. The same manual states `P_SW2.EAXFR` must be set before XFR access, and `PxIE` must be `1` for digital ports or the MCU cannot read the external port level.

The official SPI switch example initializes all port mode registers to `0x00` before selecting the SPI group. The ToyRemote legacy transmitter also keeps `P1M1/P1M0` at `0x00` and uses `SPCTL=0xD0`; the current HAL already uses `SPCTL=0xD0` but changes group 0 pins to push-pull/high-impedance and does not explicitly enable `P1IE.4`.

## Design

`stc8h_spi_init()` remains a small compile-time configured hardware SPI initializer. It will:

- select the SPI pin group through `P_SW1[3:2]`;
- by default keep MOSI/MISO/SCLK in quasi-bidirectional mode, matching the official examples and legacy firmware;
- by default enable digital input on the selected group's MISO pin with `P_SW2 |= 0x80` followed by the matching `PxIE` bit set;
- preserve `SPCTL=STC8H_SPI_SPCTL` and polling transfer behavior.

Two compile-time switches keep the policy explicit:

- `STC8H_SPI_CONFIGURE_PORT_MODE`, default `1`, controls whether the HAL sets the SPI pin mode bits to quasi-bidirectional.
- `STC8H_SPI_ENABLE_MISO_INPUT`, default `1`, controls whether the HAL touches `P_SW2` and the selected `PxIE` MISO bit.

The hardware SS pin remains board/application-owned because the HAL default uses `SSIG=1` and does not save a CS pin. This keeps the default safe for nRF24L01 and official STC8H usage while allowing an application with unusual board-level pin policy to opt out.

## Tradeoffs

Leaving the current push-pull/high-impedance mode and only adding `P1IE.4` is the smallest code change, but it preserves a difference from both STC's SPI example and the working legacy firmware. Returning to quasi-bidirectional by default is the more conservative foundation-library default because it follows the vendor example and avoids board-specific output-strength policy in the HAL.

Supporting all four SPI groups in one initializer adds a few compile-time branches, but only the selected group is emitted after preprocessing. The extra ROM cost for the default group is limited to a small number of bit operations.

## Verification

Add an SDCC codegen check that compiles `hal/stc8h_spi.c` for each `STC8H_SPI_PIN_GROUP` and fails unless the generated output contains access to `P_SW2` and the expected `PxIE` address for that group's MISO pin.

Run:

```sh
sh tools/check_host_tests.sh
sh tools/check_examples.sh
```

For ToyRemote hardware validation, rebuild and upload controller `STC8H1K08_radio_diag`; the expected visible progress is no stop at `C000`, then `C001` before later TX diagnostics.
