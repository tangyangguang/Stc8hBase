# STC8H8K64U LQFP48 Support Design

## Goal

Add explicit, opt-in base-library support for `STC8H8K64U-45I-LQFP48` while keeping existing `STC8H1K08` programs unchanged.

This support is for reusable MCU-level capabilities only. It is not a product framework and does not bind UART2 or UART3 to RS485, 433 MHz modules, Modbus, or any application protocol.

## Evidence

Primary sources:

- `docs/vendor/stc/STC8H-en.pdf`
- `https://www.stcmicro.com/datasheet/STC8H8K64U_Features.pdf`

Facts used in this design:

- `STC8H8K64U` supports 4 UARTs, 5 16-bit timers, 12-bit ADC with 15 external channels plus internal reference channel, configurable EEPROM size, USB, DMA, RTC, LCM, and up to 61 GPIOs.
- LQFP48 exposes the UART2 pin pairs `RXD2/P1.0, TXD2/P1.1` and `RXD2_2/P4.6, TXD2_2/P4.7`.
- LQFP48 exposes the UART3 pin pairs `RXD3/P0.0, TXD3/P0.1` and `RXD3_2/P5.0, TXD3_2/P5.1`.
- `P_SW2` selects UART2 and UART3 pin groups: UART2 bit0, UART3 bit1.
- UART2 can use Timer2 as baud-rate generator. UART3 can use Timer3 as baud-rate generator. This is the preferred concurrent-use split so UART2 and UART3 do not share one baud-rate timer.
- `ADC_VRef+` must not float. If USB download is not used, `P3.0/P3.1/P3.2` must not all be low at reset.

## Scope

### In scope

- Add a chip-selection path for `STC8H_CHIP_STC8H8K64U`.
- Add a board profile for `STC8H8K64U-45I-LQFP48`.
- Add only the SFR/XFR definitions required by the first supported modules.
- Keep `STC8H1K08` as the default chip when no board config is supplied.
- Extend GPIO support only through compile-time masks.
- Make ADC width, valid channels, and result assembly chip-configurable.
- Make EEPROM/IAP size chip-configurable and keep sector size explicit.
- Extend UART HAL to support UART2 and UART3 as generic serial ports.
- Keep UART1 as the default download/debug path.
- Add minimal build examples for H8K64U-LQFP48:
  - GPIO blink
  - UART2 hello/echo
  - UART3 hello/echo
  - ADC read
  - EEPROM safe build
- Add verification that existing STC8H1K08 examples still build and do not gain H8K64U-only symbols.

### Out of scope

- USB device, USB download helpers, USB CDC, or endpoint support.
- DMA, RTC, LCM/TFT, comparator, MDU16 helper, full I/O interrupt framework.
- UART4 support.
- RS485 protocol, Modbus, 433 MHz module driver, packet framing, or addressing.
- Runtime chip detection, device tree, object registry, or generic peripheral tables.

## Architecture

Chip differences remain compile-time decisions. `core/stc8h_config.h` continues to default to `STC8H_CHIP_STC8H1K08=1`. The H8K64U board profile explicitly disables that default by defining `STC8H_CHIP_STC8H8K64U=1` before common chip defaults are selected.

The library keeps its current thin-HAL shape:

- `core/stc8h_sfr.h` owns SFR/XFR names.
- `core/stc8h_config.h` owns default chip constants.
- `board/stc8h8k64u_lqfp48_base/` owns board-visible pin choices.
- `hal/stc8h_uart.*` owns UART byte-level initialization and polling I/O.
- Application projects own RS485 DE/RE pins, 433 module wiring, protocol framing, and state machines.

## UART Design

UART IDs become:

```c
typedef enum {
    STC8H_UART1 = 0,
    STC8H_UART2,
    STC8H_UART3
} stc8h_uart_id_t;
```

UART1 keeps the current implementation and remains the compatibility path for STC8H1K08.

UART2 is enabled only when `STC8H_UART_ENABLE_UART2` is non-zero. It uses Timer2 by default and selects pins through `STC8H_UART2_PIN_GROUP`:

```c
#define STC8H_UART2_PIN_GROUP_0 0u /* RXD2=P1.0, TXD2=P1.1 */
#define STC8H_UART2_PIN_GROUP_1 1u /* RXD2_2=P4.6, TXD2_2=P4.7 */
```

UART3 is enabled only when `STC8H_UART_ENABLE_UART3` is non-zero. It uses Timer3 by default and selects pins through `STC8H_UART3_PIN_GROUP`:

```c
#define STC8H_UART3_PIN_GROUP_0 0u /* RXD3=P0.0, TXD3=P0.1 */
#define STC8H_UART3_PIN_GROUP_1 1u /* RXD3_2=P5.0, TXD3_2=P5.1 */
```

UART2 and UART3 get independent baud macros:

```c
#define STC8H_UART2_BAUD 9600UL
#define STC8H_UART3_BAUD 9600UL
```

The first implementation supports polling transmit and polling receive. Interrupt-driven receive is excluded from the first implementation and needs a separate design when a real project requires it.

RS485 is not part of UART HAL. Applications should bind communication roles in board/application config:

```c
#define BOARD_RS485_UART STC8H_UART2
#define BOARD_RF433_UART STC8H_UART3
```

If a board swaps roles, only these application-level macros change.

## GPIO Design

The H8K64U-LQFP48 board profile sets a GPIO mask matching only ports actually used by the package and board. P6/P7 support should be added only in a separate scoped change if the selected LQFP48 pins or a future package actually require it.

Existing `STC8H1K08` builds keep their current defaults:

```c
#define STC8H_GPIO_PORT_COUNT 6u
#define STC8H_GPIO_PORT_MASK 0x3Fu
```

H8K64U-LQFP48 board config may override these after pin verification.

## ADC Design

ADC behavior becomes chip-configurable:

```c
#define STC8H_ADC_BITS 12u
#define STC8H_ADC_CHANNEL_MASK 0xFFFFu
```

For `STC8H1K08`, defaults remain 10-bit and current channel validity.

For `STC8H8K64U`, valid external channels are 0..14 and channel 15 is internal reference. The API still returns `stc8h_u16`, so no public type change is required. Existing 10-bit callers are unaffected when they build for `STC8H1K08`.

## EEPROM/IAP Design

EEPROM size becomes configurable:

```c
#ifndef STC8H_EEPROM_SIZE
#define STC8H_EEPROM_SIZE 4096u
#endif
```

The H8K64U board/application config must set the actual EEPROM size chosen in ISP/project configuration before enabling destructive EEPROM tests. Safe examples continue to default to no write/erase.

## Compatibility Requirements

- `core/stc8h_config.h` must continue to default to `STC8H_CHIP_STC8H1K08`.
- Existing STC8H1K08 board files and examples must not be renamed.
- H8K64U-only branches must be guarded by chip or feature macros.
- Disabled UART2/UART3 code must not emit UART2/UART3 symbols.
- STC8H1K08 ADC must keep 10-bit result behavior.
- STC8H1K08 EEPROM must keep 4KB default size.

## Verification

Required before claiming support:

1. Run existing full build:

```sh
tools/check_examples.sh
```

2. Add compile checks for H8K64U-LQFP48 examples.
3. Add symbol checks that UART2/UART3 code is absent from existing UART1-only examples.
4. On hardware, verify:
   - UART1 download still works through normal default flow.
   - UART2 group 0 or group 1 transmits and receives.
   - UART3 group 0 or group 1 transmits and receives.
   - GPIO output does not glitch unsafe pins during init.
   - ADC reads a stable known voltage with `ADC_VRef+` connected.
   - EEPROM write/erase only runs in an explicitly selected destructive-test environment.

## Self-review

- Scope is intentionally MCU-level and avoids business logic.
- UART2 and UART3 are generic ports, so 485/433 roles are not frozen into the base library.
- UART2 uses Timer2 and UART3 uses Timer3 to avoid a default timer conflict.
- The plan does not require changing the STC8H1K08 default chip path.
- USB/DMA/RTC/LCM are excluded even though H8K64U supports them, because they would materially increase library scope.
- The main uncertainty is the exact H8K64U EEPROM size in the final ISP configuration; destructive EEPROM examples must remain opt-in until that is confirmed.
