# Driver API Trimming Design

## Goal

Support STC8H1K08 builds that combine nRF24L01 ACK payload, TM1637 raw display, PWM, and ADC without copying or rewriting foundation-library drivers in application projects.

## Context

SDCC/mcs51 does not reliably remove unused public functions from a compiled wrapper translation unit, and unused public functions can still consume code space and overlay/parameter RAM. The existing `proto_rf_link` trimming macros solve this at the source level by making unused APIs disappear from both declarations and definitions.

## Design

Use the same default-on compile-time trimming style for the affected drivers. Defaults keep the current API surface intact for existing examples. Small-memory applications explicitly disable unused capabilities in board or build configuration.

`drv_nrf24l01` receives grouped public API switches:

- argument checks: `DRV_NRF24L01_ENABLE_ARG_CHECK`
- generic address configuration: `DRV_NRF24L01_ENABLE_ADDRESS_API`
- fixed pipe0 helper: `DRV_NRF24L01_ENABLE_PIPE0_FIXED_API`, `DRV_NRF24L01_FIXED_ADDRESS_WIDTH`, `DRV_NRF24L01_FIXED_PAYLOAD_SIZE`
- diagnostic status helpers: `DRV_NRF24L01_ENABLE_READ_FIFO_STATUS`, `DRV_NRF24L01_ENABLE_READ_OBSERVE_TX`
- raw register/buffer helpers: `DRV_NRF24L01_ENABLE_RAW_API`
- presence check: `DRV_NRF24L01_ENABLE_CHECK_PRESENT`
- optional mode helpers: `DRV_NRF24L01_ENABLE_POWER_DOWN`, `DRV_NRF24L01_ENABLE_ENTER_STANDBY`
- plus features: `DRV_NRF24L01_ENABLE_DYNAMIC_PAYLOAD`, `DRV_NRF24L01_ENABLE_ACK_PAYLOAD`
- ACK-payload-adjacent helpers: `DRV_NRF24L01_ENABLE_WRITE_ACK_PAYLOAD`, `DRV_NRF24L01_ENABLE_DISABLE_ACK_PAYLOAD`, `DRV_NRF24L01_ENABLE_READ_DYNAMIC_PAYLOAD_SIZE`

Status, payload, IRQ, and flush APIs remain always available because they are the small reusable core and are used directly by compact applications. Raw register, buffer, and command APIs remain default-on, but small applications can close the public raw API surface while keeping the same internal helpers compiled as `static` driver code. Fixed pipe0 configuration removes the generic address-width, TX address, RX address, and payload-size public API path when the application always uses pipe0 with a fixed address width and payload size.

`proto_rf_link` receives fixed-path helpers:

- `PROTO_RF_LINK_ENABLE_SET_IDS`
- `PROTO_RF_LINK_ENABLE_PACKET_ARG_CHECK`
- `PROTO_RF_LINK_ENABLE_INIT_TIMEOUT_FIELDS`
- `PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED`
- `PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED`
- `PROTO_RF_LINK_FIXED_PAYLOAD_LEN`

The fixed send helper constructs a DATA packet with ACK_REQUIRED and a compile-time payload length. The fixed poll helper accepts only DATA packets with the same compile-time payload length. The generic `send_data()` and `poll()` APIs remain default-on and unchanged.

`drv_tm1637` receives API switches for the formatting layer:

- `DRV_TM1637_ENABLE_DISPLAY_RAW`
- `DRV_TM1637_ENABLE_DISPLAY_RAW4`
- `DRV_TM1637_ENABLE_SET_DISPLAY`
- `DRV_TM1637_ENABLE_BRIGHTNESS_STATE`
- `DRV_TM1637_FIXED_BRIGHTNESS`
- `DRV_TM1637_ENABLE_RAW_LEN_CHECK`
- `DRV_TM1637_ENABLE_DISPLAY_DIGITS`
- `DRV_TM1637_ENABLE_DISPLAY_NUMBER`
- `DRV_TM1637_ENABLE_ENCODE_DIGIT`
- `DRV_TM1637_ENABLE_CLEAR`

`display_raw4` is the small fixed path for four TM1637 digits. The existing `display_raw()` API remains default-on and keeps its current checks by default. `display_digits` requires `encode_digit`; `display_number` requires `encode_digit` and owns the divide-by-10 helper. Invalid macro combinations fail at compile time.

`drv_ec11` receives a small fixed API:

- `DRV_EC11_ENABLE_FULL_API`
- `DRV_EC11_ENABLE_SMALL_API`
- `DRV_EC11_SMALL_STEPS_PER_DETENT`
- `DRV_EC11_SMALL_REVERSE`

The small API stores only last state and step accumulator, omits fast rotation and runtime setters, and returns a delta directly from scan. The full API remains default-on.

`stc8h_pwm` keeps the existing group and channel masks. It adds only the small-memory API switch that is currently known to matter:

- `STC8H_PWM_ENABLE_DISABLE`

The PWM period and prescaler state is also compiled only for enabled groups, so closing `PWMA` or `PWMB` removes its two 16-bit state variables. More PWM API switches are intentionally not added until a map file proves they are needed.

## Tradeoffs

This design favors explicit compile-time control over automatic linker cleanup because the target toolchain does not make automatic cleanup reliable enough for 8KB/8051 builds. It adds a few build macros, but avoids splitting every driver into many tiny source files or pushing application-specific wrappers into application projects.

The nRF24 and TM1637 switches are more granular than a desktop library would need, but each switch corresponds to observed or high-cost functions. PWM is deliberately less granular to avoid an API-switch matrix before there is evidence it pays for itself.

## Validation

- Add map checks to ensure trimmed example builds do not contain disabled nRF24, TM1637, and PWM symbols.
- Run `tools/check_examples.sh` in this repository.
- Run `/Users/tyg/dir/codex_dir/Stc8hToyRemote/tools/check_all.sh`.
- Run `pio run -d /Users/tyg/dir/codex_dir/Stc8hToyRemote/receiver`.
