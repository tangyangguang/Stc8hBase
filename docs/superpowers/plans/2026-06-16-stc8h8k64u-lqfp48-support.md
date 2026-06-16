# STC8H8K64U LQFP48 Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add explicit `STC8H8K64U-45I-LQFP48` base-library support with generic UART2/UART3 communication ports and require explicit chip profiles for all projects.

**Architecture:** Keep the current thin HAL and compile-time configuration model. Add H8K64U chip constants and board defaults separately, make chip selection one-hot and explicit, guard UART2/UART3 code behind feature macros, and keep RS485/433 role binding in application or board configuration.

**Tech Stack:** C for SDCC/Keil C51, STC8H official documentation, PlatformIO/Makefile examples, shell verification scripts.

---

## File Structure

- Modify `core/stc8h_config.h`: replace implicit chip defaulting with explicit one-hot chip selection.
- Modify `core/stc8h_sfr.h`: add only SFR/XFR definitions needed by H8K64U GPIO, ADC, EEPROM, UART2, UART3, Timer2, and Timer3.
- Modify `hal/stc8h_adc.h` and `hal/stc8h_adc.c`: make ADC width and channel validity configurable.
- Modify `hal/stc8h_eeprom.h`: make EEPROM size configurable.
- Modify `hal/stc8h_uart.h` and `hal/stc8h_uart.c`: add optional UART2/UART3 polling support.
- Create `board/stc8h8k64u_lqfp48_base/board_config.h`: H8K64U-LQFP48 chip and board defaults.
- Create `board/stc8h8k64u_lqfp48_base/board_pins.h`: pin aliases for UART2/UART3 role examples and safe GPIO defaults.
- Create minimal PlatformIO examples under `examples/platformio/`:
  - `h8k64u_uart2_hello`
  - `h8k64u_uart3_hello`
  - `h8k64u_gpio_blink`
  - `h8k64u_adc_read`
  - `h8k64u_eeprom_safe`
  - `h8k64u_wdt_feed`
  - wrapper source files for every HAL/core module each example calls.
- Create: `examples/platformio/boards/STC8H8K64U.json` if PlatformIO does not already provide a suitable board definition in the installed platform.
- Create: `docs/24_EXPLICIT_CHIP_PROFILE_MIGRATION.md`
- Modify `tools/check_examples.sh`: make existing temporary compile checks use explicit chip profiles, then add compile-only H8K64U checks and symbol absence checks.
- Modify `docs/03_CHIP_SUPPORT.md`, `docs/10_REFERENCES.md`, `docs/vendor/stc/README.md`, and `docs/13_RESOURCE_POLICY.md`: document support level, source, and resource rules.

### Task 1: Document Source And Support Level

**Files:**
- Modify: `docs/03_CHIP_SUPPORT.md`
- Modify: `docs/10_REFERENCES.md`
- Modify: `docs/vendor/stc/README.md`
- Modify: `docs/13_RESOURCE_POLICY.md`
- Create: `docs/24_EXPLICIT_CHIP_PROFILE_MIGRATION.md`

- [ ] **Step 1: Add support level text**

Add a subsection to `docs/03_CHIP_SUPPORT.md`:

```markdown
### STC8H8K64U-45I-LQFP48 opt-in support

`STC8H8K64U-45I-LQFP48` is supported as an explicit chip profile.
The library requires every build to select exactly one supported chip profile.
This removes the old implicit `STC8H1K08` fallback.

Initial support covers core configuration, GPIO, UART1, UART2, UART3,
Timer resources needed by those UARTs, ADC, EEPROM/IAP, WDT, and small examples.
USB, DMA, RTC, LCM, UART4, full I/O interrupt support, RS485 protocol, and 433 MHz
module drivers are outside the initial support scope.
```

- [ ] **Step 2: Add official source record**

Add to `docs/10_REFERENCES.md` under STC official sources:

```markdown
- `https://www.stcmicro.com/datasheet/STC8H8K64U_Features.pdf`
  - Used to verify `STC8H8K64U-45I-LQFP48` resources, package pins, UART2/UART3 pin groups, ADC width, and reset/download notes.
  - Local SHA-256 when downloaded for review: `7b5e88e8b0fbb248cd839c4aeeae7b3c3078900055a222e2ff75df76b0ea8088`.
```

- [ ] **Step 3: Add migration note**

Create `docs/24_EXPLICIT_CHIP_PROFILE_MIGRATION.md`:

````markdown
# Explicit Chip Profile Migration

## Why this changed

The base library no longer silently defaults to `STC8H1K08` when no chip is selected.
Every firmware must select exactly one chip profile. This prevents a project for one
chip from compiling under another chip's register and resource assumptions.

## Who is affected

Existing projects are affected only if they include the base library without a board
configuration or without defining `STC8H_CHIP_STC8H1K08`.

Existing projects that already include `board/stc8h1k08_tssop20_demo/board_config.h`
and define `STC8H_CHIP_STC8H1K08 1` should continue to build.

## Migration for STC8H1K08 projects

Add an explicit board config or compile flag:

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
```

If the project uses ADC, keep the STC8H1K08 behavior explicit:

```c
#define STC8H_ADC_BITS 10u
#define STC8H_ADC_CHANNEL_MASK 0xFF03u
```

If the project uses EEPROM/IAP, keep the STC8H1K08 range explicit:

```c
#define STC8H_EEPROM_SIZE 4096u
#define STC8H_EEPROM_SECTOR_SIZE 512u
```

## Migration for STC8H8K64U-LQFP48 projects

Use a board config that selects:

```c
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
```

Then set the board-specific UART, ADC, GPIO, and EEPROM choices. Do not enable
destructive EEPROM tests until the actual ISP EEPROM allocation is confirmed.
````

- [ ] **Step 4: Run documentation grep**

Run:

```sh
rg -n "STC8H8K64U|UART2|UART3|LQFP48|explicit chip" docs/03_CHIP_SUPPORT.md docs/10_REFERENCES.md docs/13_RESOURCE_POLICY.md docs/vendor/stc/README.md docs/24_EXPLICIT_CHIP_PROFILE_MIGRATION.md
```

Expected: the new support level and source notes appear.

- [ ] **Step 5: Commit docs**

```sh
git add docs/03_CHIP_SUPPORT.md docs/10_REFERENCES.md docs/vendor/stc/README.md docs/13_RESOURCE_POLICY.md docs/24_EXPLICIT_CHIP_PROFILE_MIGRATION.md
git commit -m "docs: plan stc8h8k64u lqfp48 support"
```

### Task 2: Add Explicit One-Hot Chip Profiles

**Files:**
- Modify: `core/stc8h_config.h`
- Modify: `board/stc8h1k08_tssop20_demo/board_config.h`
- Modify: `tools/check_examples.sh`
- Create: `board/stc8h8k64u_lqfp48_base/board_config.h`

- [ ] **Step 1: Add failing compile check**

Create a temporary compile input in `/tmp/h8k64u_config_check.c`:

```c
#define STC8H_CONFIG_INCLUDE "board_config.h"
#include "stc8h_config.h"
#if !STC8H_CHIP_STC8H8K64U
#error "H8K64U chip macro not selected"
#endif
#if STC8H_CHIP_STC8H1K08
#error "STC8H1K08 default leaked into H8K64U board config"
#endif
void main(void) {}
```

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Iboard/stc8h8k64u_lqfp48_base -c /tmp/h8k64u_config_check.c
```

Expected before implementation: fail because board config does not exist.

- [ ] **Step 2: Create board config**

Create `board/stc8h8k64u_lqfp48_base/board_config.h`:

```c
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1

#define STC8H_SYSCLK_HZ 11059200UL
#define STC8H_UART1_BAUD 115200UL
#define STC8H_UART2_BAUD 9600UL
#define STC8H_UART3_BAUD 9600UL

#define STC8H_UART_ENABLE_UART2 0
#define STC8H_UART_ENABLE_UART3 0
#define STC8H_UART2_PIN_GROUP 0u
#define STC8H_UART3_PIN_GROUP 0u

#define STC8H_ADC_BITS 12u
#define STC8H_ADC_CHANNEL_MASK 0xFFFFu

#define STC8H_EEPROM_SIZE 0u
#define STC8H_EEPROM_SECTOR_SIZE 512u

#endif
```

- [ ] **Step 3: Make STC8H1K08 board config explicit**

Ensure `board/stc8h1k08_tssop20_demo/board_config.h` defines both chip macros:

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
```

- [ ] **Step 4: Replace implicit chip default logic**

In `core/stc8h_config.h`, replace the current default chip block with explicit one-hot validation:

```c
#ifndef STC8H_CHIP_STC8H8K64U
#define STC8H_CHIP_STC8H8K64U 0
#endif

#ifndef STC8H_CHIP_STC8H1K08
#define STC8H_CHIP_STC8H1K08 0
#endif

#if ((STC8H_CHIP_STC8H1K08 != 0) && (STC8H_CHIP_STC8H1K08 != 1)) || \
    ((STC8H_CHIP_STC8H8K64U != 0) && (STC8H_CHIP_STC8H8K64U != 1))
#error "STC8H chip profile macros must be 0 or 1."
#endif

#if (STC8H_CHIP_STC8H1K08 + STC8H_CHIP_STC8H8K64U) != 1
#error "Select exactly one STC8H chip profile."
#endif
```

- [ ] **Step 5: Run config checks**

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Iboard/stc8h8k64u_lqfp48_base -c /tmp/h8k64u_config_check.c
cat > /tmp/stc8h1k08_config_check.c <<'EOF'
#define STC8H_CONFIG_INCLUDE "board_config.h"
#include "stc8h_config.h"
#if !STC8H_CHIP_STC8H1K08
#error "default chip changed"
#endif
void main(void) {}
EOF
sdcc -mmcs51 --std-sdcc11 -Icore -Iboard/stc8h1k08_tssop20_demo -c /tmp/stc8h1k08_config_check.c
```

Expected: both commands pass.

- [ ] **Step 6: Update existing temporary compile checks**

In `tools/check_examples.sh`, add the explicit STC8H1K08 chip profile to every temporary C file that directly includes a HAL, driver, protocol, or utility source file.

For `eeprom_read_only.c`, the generated file starts with:

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_EEPROM_ENABLE_READ 1
#define STC8H_EEPROM_ENABLE_WRITE 0
#define STC8H_EEPROM_ENABLE_ERASE 0
#include "${ROOT_DIR}/hal/stc8h_eeprom.c"
```

For `eeprom_fixed.c`, the generated file starts with:

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_EEPROM_ENABLE_READ 0
#define STC8H_EEPROM_ENABLE_WRITE 0
#define STC8H_EEPROM_ENABLE_ERASE 0
#define STC8H_EEPROM_ENABLE_FIXED_BLOCK 1
#define STC8H_EEPROM_FIXED_ADDR 0x0E00u
#define STC8H_EEPROM_FIXED_SIZE 8u
#include "${ROOT_DIR}/hal/stc8h_eeprom.c"
```

For `ec11_small_isr.c`, add the chip profile before driver feature macros:

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define DRV_EC11_ENABLE_FULL_API 0
#define DRV_EC11_ENABLE_SMALL_API 0
#define DRV_EC11_ENABLE_SMALL_ISR_API 1
#define DRV_EC11_ENABLE_NULL_CHECK 0
```

For each generated `spi_group_${group}.c`, add:

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_SPI_PIN_GROUP ${group}u
#include "${ROOT_DIR}/hal/stc8h_spi.c"
```

For `pwm_xfr.c`, add:

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#include "${ROOT_DIR}/hal/stc8h_pwm.c"
```

`test_using.c` only includes `stc8h_compiler.h`, so it does not need a chip profile.

- [ ] **Step 7: Verify no-config builds fail**

Run:

```sh
cat > /tmp/stc8h_no_config_check.c <<'EOF'
#include "stc8h_config.h"
void main(void) {}
EOF
! sdcc -mmcs51 --std-sdcc11 -Icore -c /tmp/stc8h_no_config_check.c
```

Expected: compile fails with `Select exactly one STC8H chip profile.`

- [ ] **Step 8: Run existing checks after explicit profile migration**

Run:

```sh
tools/check_examples.sh
```

Expected: all existing STC8H1K08 examples, Make examples, and temporary compile checks pass after their chip selection is explicit.

- [ ] **Step 9: Commit**

```sh
git add core/stc8h_config.h board/stc8h1k08_tssop20_demo/board_config.h board/stc8h8k64u_lqfp48_base/board_config.h tools/check_examples.sh
git commit -m "feat: add stc8h8k64u chip config"
```

### Task 3: Add Required SFR/XFR Definitions

**Files:**
- Modify: `core/stc8h_sfr.h`

- [ ] **Step 1: Write compile check for new names**

Create `/tmp/h8k64u_sfr_check.c`:

```c
#define STC8H_CONFIG_INCLUDE "board_config.h"
#include "stc8h_config.h"
#include "stc8h_sfr.h"
void main(void)
{
    S2CON = 0u;
    S3CON = 0u;
    S2BUF = 0u;
    S3BUF = 0u;
    T4T3M = 0u;
    T2H = 0u;
    T2L = 0u;
    T3H = 0u;
    T3L = 0u;
}
```

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Iboard/stc8h8k64u_lqfp48_base -c /tmp/h8k64u_sfr_check.c
```

Expected before implementation: fail for missing SFR names.

- [ ] **Step 2: Add SFR names**

Add only official-verified names needed by first modules. Use addresses from `STC8H-en.pdf` and official examples. Add comments for any address that requires re-check before code use.

```c
STC8H_SFR(S2CON, 0x9Au);
STC8H_SFR(S2BUF, 0x9Bu);
STC8H_SFR(S3CON, 0xACu);
STC8H_SFR(S3BUF, 0xADu);
STC8H_SFR(T4T3M, 0xD1u);
STC8H_SFR(T3H, 0xD4u);
STC8H_SFR(T3L, 0xD5u);
STC8H_SFR(T2H, 0xD6u);
STC8H_SFR(T2L, 0xD7u);
```

These addresses are from `STC8H-en.pdf` Timer2, Timer3/4, UART2, and UART3 sections.

- [ ] **Step 3: Run SFR compile check**

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Iboard/stc8h8k64u_lqfp48_base -c /tmp/h8k64u_sfr_check.c
```

Expected: pass after all required SFR names are present.

- [ ] **Step 4: Commit**

```sh
git add core/stc8h_sfr.h
git commit -m "feat: add h8k64u uart sfr definitions"
```

### Task 4: Make EEPROM And ADC Chip Configurable

**Files:**
- Modify: `hal/stc8h_eeprom.h`
- Modify: `hal/stc8h_adc.h`
- Modify: `hal/stc8h_adc.c`

- [ ] **Step 1: Write ADC compile checks**

Create `/tmp/adc_width_check.c`:

```c
#define STC8H_CONFIG_INCLUDE "board_config.h"
#include "stc8h_adc.h"
#if STC8H_ADC_BITS != 12u
#error "H8K64U ADC width not selected"
#endif
void main(void) {}
```

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Ihal -Iboard/stc8h8k64u_lqfp48_base -c /tmp/adc_width_check.c
```

Expected before implementation: fail if `STC8H_ADC_BITS` is not exposed consistently.

- [ ] **Step 2: Make EEPROM size and feature gates overridable**

Change the top of `hal/stc8h_eeprom.h` so size, sector size, and feature gates are all overridable before range checks run:

```c
#ifndef STC8H_EEPROM_SIZE
#define STC8H_EEPROM_SIZE 4096u
#endif

#ifndef STC8H_EEPROM_SECTOR_SIZE
#define STC8H_EEPROM_SECTOR_SIZE 512u
#endif

#ifndef STC8H_EEPROM_ENABLE_READ
#define STC8H_EEPROM_ENABLE_READ 1
#endif

#ifndef STC8H_EEPROM_ENABLE_WRITE
#define STC8H_EEPROM_ENABLE_WRITE 1
#endif

#ifndef STC8H_EEPROM_ENABLE_ERASE
#define STC8H_EEPROM_ENABLE_ERASE 1
#endif

#ifndef STC8H_EEPROM_ENABLE_FIXED_BLOCK
#define STC8H_EEPROM_ENABLE_FIXED_BLOCK 0
#endif

#if (STC8H_EEPROM_SIZE == 0u) && \
    (STC8H_EEPROM_ENABLE_READ || STC8H_EEPROM_ENABLE_WRITE || \
     STC8H_EEPROM_ENABLE_ERASE || STC8H_EEPROM_ENABLE_FIXED_BLOCK)
#error "STC8H_EEPROM_SIZE must be set before enabling EEPROM APIs."
#endif
```

Remove the old unconditional `#define STC8H_EEPROM_SIZE 4096u` and `#define STC8H_EEPROM_SECTOR_SIZE 512u` lines. Keep the existing API declarations under their current feature macros.

- [ ] **Step 3: Add ADC defaults**

In `hal/stc8h_adc.h`, add:

```c
#ifndef STC8H_ADC_BITS
#define STC8H_ADC_BITS 10u
#endif

#ifndef STC8H_ADC_CHANNEL_MASK
#define STC8H_ADC_CHANNEL_MASK 0xFF03u
#endif
```

- [ ] **Step 4: Replace channel check**

In `hal/stc8h_adc.c`, replace the hard-coded channel-valid function with:

```c
static stc8h_u8 stc8h_adc_channel_valid(stc8h_u8 channel)
{
    if (channel > 15u) {
        return 0u;
    }
    return ((STC8H_ADC_CHANNEL_MASK & ((stc8h_u16)1u << channel)) != 0u) ? 1u : 0u;
}
```

- [ ] **Step 5: Replace result assembly**

Replace the return expression with:

```c
#if STC8H_ADC_BITS == 12u
    return (stc8h_u16)(((stc8h_u16)(ADC_RES & 0x0Fu) << 8) | ADC_RESL);
#else
    return (stc8h_u16)(((stc8h_u16)(ADC_RES & 0x03u) << 8) | ADC_RESL);
#endif
```

- [ ] **Step 6: Run checks**

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Ihal -Iboard/stc8h8k64u_lqfp48_base -c /tmp/adc_width_check.c
tools/check_examples.sh
```

Expected: H8K64U compile check passes and existing examples pass.

- [ ] **Step 7: Commit**

```sh
git add hal/stc8h_eeprom.h hal/stc8h_adc.h hal/stc8h_adc.c
git commit -m "feat: make adc and eeprom chip configurable"
```

### Task 5: Add UART2/UART3 Polling Support

**Files:**
- Modify: `hal/stc8h_uart.h`
- Modify: `hal/stc8h_uart.c`

- [ ] **Step 1: Write UART2/UART3 symbol trim check**

Create `/tmp/uart1_only.c`:

```c
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_UART_ASSUME_UART1 1
#include "stc8h_uart.c"
void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
}
```

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Ihal -c -o /tmp/uart1_only.rel /tmp/uart1_only.c
grep -E "S2CON|S3CON|_stc8h_uart2|_stc8h_uart3" /tmp/uart1_only.asm /tmp/uart1_only.sym
```

Expected after implementation: grep finds nothing.

- [ ] **Step 2: Extend UART enum**

Change `hal/stc8h_uart.h` to:

```c
typedef enum {
    STC8H_UART1 = 0,
    STC8H_UART2,
    STC8H_UART3
} stc8h_uart_id_t;
```

- [ ] **Step 3: Add feature defaults**

In `hal/stc8h_uart.c`, add:

```c
#ifndef STC8H_UART_ENABLE_UART2
#define STC8H_UART_ENABLE_UART2 0
#endif

#ifndef STC8H_UART_ENABLE_UART3
#define STC8H_UART_ENABLE_UART3 0
#endif

#ifndef STC8H_UART2_PIN_GROUP
#define STC8H_UART2_PIN_GROUP 0u
#endif

#ifndef STC8H_UART3_PIN_GROUP
#define STC8H_UART3_PIN_GROUP 0u
#endif

#if (STC8H_UART_ENABLE_UART2 || STC8H_UART_ENABLE_UART3) && STC8H_UART_ASSUME_UART1
#error "STC8H_UART_ASSUME_UART1 cannot be used when UART2 or UART3 is enabled."
#endif
```

- [ ] **Step 4: Add UART2 init path**

Implement UART2 only under:

```c
#if STC8H_UART_ENABLE_UART2
/* UART2 init, putc, readable, getc using S2CON/S2BUF and Timer2. */
#endif
```

Use the official 1T baud reload formula:

```c
#define STC8H_UART_RELOAD_VALUE(sysclk, baud) (65536UL - ((sysclk) / (baud) / 4UL))

#if STC8H_UART_ENABLE_UART2
#if STC8H_UART2_BAUD == 0UL
#error "STC8H_UART2_BAUD must be non-zero when UART2 is enabled."
#endif
#ifndef STC8H_UART2_RELOAD
#define STC8H_UART2_RELOAD STC8H_UART_RELOAD_VALUE(STC8H_SYSCLK_HZ, STC8H_UART2_BAUD)
#endif
#if (STC8H_UART2_RELOAD == 0UL) || (STC8H_UART2_RELOAD > 65535UL)
#error "STC8H_UART2_RELOAD is out of 16-bit range."
#endif
#endif
```

Use these control bits:

```c
#define STC8H_AUXR_T2R   0x10u
#define STC8H_AUXR_T2_CT 0x08u
#define STC8H_AUXR_T2X12 0x04u
#define STC8H_UART2_REN  0x10u
#define STC8H_UART2_TI   0x02u
#define STC8H_UART2_RI   0x01u
```

UART2 initialization must preserve unrelated `AUXR` bits:

```c
S2CON = STC8H_UART2_REN;
AUXR &= (stc8h_u8)~(STC8H_AUXR_T2R | STC8H_AUXR_T2_CT | STC8H_AUXR_T2X12);
T2L = (stc8h_u8)STC8H_UART2_RELOAD;
T2H = (stc8h_u8)(STC8H_UART2_RELOAD >> 8);
AUXR = (stc8h_u8)((AUXR & (stc8h_u8)~STC8H_AUXR_T2_CT) |
                  STC8H_AUXR_T2X12 | STC8H_AUXR_T2R);
```

UART2 polling transmit must clear stale `S2TI` before sending and clear it after completion:

```c
S2CON &= (stc8h_u8)~STC8H_UART2_TI;
S2BUF = (stc8h_u8)ch;
while ((S2CON & STC8H_UART2_TI) == 0u) {
}
S2CON &= (stc8h_u8)~STC8H_UART2_TI;
```

UART2 polling receive must read only when `S2RI` is set and clear `S2RI` after reading `S2BUF`.

Use `P_SW2` bit0 to select pin group. Preserve unrelated `P_SW2` bits:

```c
if (STC8H_UART2_PIN_GROUP == 0u) {
    P_SW2 &= (stc8h_u8)~0x01u;
} else {
    P_SW2 |= 0x01u;
}
```

- [ ] **Step 5: Add UART3 init path**

Implement UART3 only under:

```c
#if STC8H_UART_ENABLE_UART3
/* UART3 init, putc, readable, getc using S3CON/S3BUF and Timer3. */
#endif
```

Use the same reload formula. Add UART3 reload validation before the UART3 implementation:

```c
#if STC8H_UART_ENABLE_UART3
#if STC8H_UART3_BAUD == 0UL
#error "STC8H_UART3_BAUD must be non-zero when UART3 is enabled."
#endif
#ifndef STC8H_UART3_RELOAD
#define STC8H_UART3_RELOAD STC8H_UART_RELOAD_VALUE(STC8H_SYSCLK_HZ, STC8H_UART3_BAUD)
#endif
#if (STC8H_UART3_RELOAD == 0UL) || (STC8H_UART3_RELOAD > 65535UL)
#error "STC8H_UART3_RELOAD is out of 16-bit range."
#endif
#endif
```

Use these control bits:

```c
#define STC8H_T4T3M_T3R    0x08u
#define STC8H_T4T3M_T3_CT  0x04u
#define STC8H_T4T3M_T3X12  0x02u
#define STC8H_T4T3M_T3CLKO 0x01u
#define STC8H_UART3_ST3    0x40u
#define STC8H_UART3_REN    0x10u
#define STC8H_UART3_TI     0x02u
#define STC8H_UART3_RI     0x01u
```

UART3 initialization must preserve Timer4 bits in `T4T3M`:

```c
S3CON = (stc8h_u8)(STC8H_UART3_ST3 | STC8H_UART3_REN);
T4T3M &= (stc8h_u8)~(STC8H_T4T3M_T3R | STC8H_T4T3M_T3_CT |
                      STC8H_T4T3M_T3X12 | STC8H_T4T3M_T3CLKO);
T3L = (stc8h_u8)STC8H_UART3_RELOAD;
T3H = (stc8h_u8)(STC8H_UART3_RELOAD >> 8);
T4T3M = (stc8h_u8)(T4T3M | STC8H_T4T3M_T3X12 | STC8H_T4T3M_T3R);
```

UART3 polling transmit must clear stale `S3TI` before sending and clear it after completion:

```c
S3CON &= (stc8h_u8)~STC8H_UART3_TI;
S3BUF = (stc8h_u8)ch;
while ((S3CON & STC8H_UART3_TI) == 0u) {
}
S3CON &= (stc8h_u8)~STC8H_UART3_TI;
```

UART3 polling receive must read only when `S3RI` is set and clear `S3RI` after reading `S3BUF`.

Use `P_SW2` bit1 to select pin group. Preserve unrelated `P_SW2` bits:

```c
if (STC8H_UART3_PIN_GROUP == 0u) {
    P_SW2 &= (stc8h_u8)~0x02u;
} else {
    P_SW2 |= 0x02u;
}
```

- [ ] **Step 6: Run UART checks**

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Ihal -c -o /tmp/uart1_only.rel /tmp/uart1_only.c
! grep -E "S2CON|S3CON|_stc8h_uart2|_stc8h_uart3" /tmp/uart1_only.asm /tmp/uart1_only.sym
tools/check_examples.sh
```

Expected: UART1-only build has no UART2/UART3 symbols and existing examples pass.

- [ ] **Step 7: Commit**

```sh
git add hal/stc8h_uart.h hal/stc8h_uart.c
git commit -m "feat: add optional uart2 uart3 polling support"
```

### Task 6: Add H8K64U Board Pins And Examples

**Files:**
- Create: `board/stc8h8k64u_lqfp48_base/board_pins.h`
- Create: `examples/platformio/boards/STC8H8K64U.json`
- Create: `examples/platformio/h8k64u_uart2_hello/platformio.ini`
- Create: `examples/platformio/h8k64u_uart2_hello/src/main.c`
- Create: `examples/platformio/h8k64u_uart2_hello/src/stc8h_uart_wrap.c`
- Create: `examples/platformio/h8k64u_uart3_hello/platformio.ini`
- Create: `examples/platformio/h8k64u_uart3_hello/src/main.c`
- Create: `examples/platformio/h8k64u_uart3_hello/src/stc8h_uart_wrap.c`
- Create: `examples/platformio/h8k64u_gpio_blink/platformio.ini`
- Create: `examples/platformio/h8k64u_gpio_blink/src/main.c`
- Create: `examples/platformio/h8k64u_gpio_blink/src/stc8h_gpio_wrap.c`
- Create: `examples/platformio/h8k64u_gpio_blink/src/stc8h_gpio_toggle_wrap.c`
- Create: `examples/platformio/h8k64u_gpio_blink/src/stc8h_delay_wrap.c`
- Create: `examples/platformio/h8k64u_adc_read/platformio.ini`
- Create: `examples/platformio/h8k64u_adc_read/src/main.c`
- Create: `examples/platformio/h8k64u_adc_read/src/stc8h_adc_wrap.c`
- Create: `examples/platformio/h8k64u_adc_read/src/stc8h_delay_wrap.c`
- Create: `examples/platformio/h8k64u_adc_read/src/stc8h_uart_wrap.c`
- Create: `examples/platformio/h8k64u_eeprom_safe/platformio.ini`
- Create: `examples/platformio/h8k64u_eeprom_safe/src/main.c`
- Create: `examples/platformio/h8k64u_eeprom_safe/src/stc8h_uart_wrap.c`
- Create: `examples/platformio/h8k64u_wdt_feed/platformio.ini`
- Create: `examples/platformio/h8k64u_wdt_feed/src/main.c`
- Create: `examples/platformio/h8k64u_wdt_feed/src/stc8h_delay_wrap.c`
- Create: `examples/platformio/h8k64u_wdt_feed/src/stc8h_uart_wrap.c`
- Create: `examples/platformio/h8k64u_wdt_feed/src/stc8h_wdt_wrap.c`

- [ ] **Step 1: Create board pins**

Create `board/stc8h8k64u_lqfp48_base/board_pins.h`:

```c
#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include "stc8h_sfr.h"

#define BOARD_RS485_UART STC8H_UART2
#define BOARD_RF433_UART STC8H_UART3

#define BOARD_UART2_RX_PORT 1u
#define BOARD_UART2_RX_PIN 0u
#define BOARD_UART2_TX_PORT 1u
#define BOARD_UART2_TX_PIN 1u

#define BOARD_UART3_RX_PORT 0u
#define BOARD_UART3_RX_PIN 0u
#define BOARD_UART3_TX_PORT 0u
#define BOARD_UART3_TX_PIN 1u

#define BOARD_TEST_GPIO_PORT 1u
#define BOARD_TEST_GPIO_PIN 2u

#endif
```

`BOARD_TEST_GPIO_*` is only for compile and bench examples. Confirm the real board wiring before driving it on hardware.
`BOARD_RS485_UART` and `BOARD_RF433_UART` intentionally expand to UART enum tokens without including `stc8h_uart.h` here; the application source that uses them must include `stc8h_uart.h`.

- [ ] **Step 2: Create PlatformIO board manifest**

Create `examples/platformio/boards/STC8H8K64U.json`:

```json
{
  "build": {
    "core": "8051",
    "cpu": "8051",
    "f_cpu": "11059200L",
    "mcu": "stc8h8k64u"
  },
  "debug": {
    "tools": {}
  },
  "frameworks": [],
  "name": "STC8H8K64U",
  "upload": {
    "maximum_ram_size": 8192,
    "maximum_size": 65536,
    "protocol": "custom"
  },
  "url": "https://www.stcmicro.com/stc/stc8h8k64u.html",
  "vendor": "STC"
}
```

- [ ] **Step 3: Create UART2 example**

Create `examples/platformio/h8k64u_uart2_hello/src/main.c`:

```c
#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART2);
    while (1) {
        stc8h_uart_write_code(STC8H_UART2, "UART2 hello\r\n");
    }
}
```

- [ ] **Step 4: Create UART3 example**

Create `examples/platformio/h8k64u_uart3_hello/src/main.c`:

```c
#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART3);
    while (1) {
        stc8h_uart_write_code(STC8H_UART3, "UART3 hello\r\n");
    }
}
```

- [ ] **Step 5: Create PlatformIO configs**

Each H8K64U example uses:

```ini
[env:STC8H8K64U]
platform = intel_mcs51
boards_dir = ../boards
board = STC8H8K64U
build_flags =
    -I../../../core
    -I../../../hal
    -I../../../board/stc8h8k64u_lqfp48_base
    -DSTC8H_CONFIG_INCLUDE=\"board_config.h\"
    -DSTC8H_PINS_INCLUDE=\"board_pins.h\"
```

Add only the feature flag needed by each example:

```ini
; h8k64u_uart2_hello only
    -DSTC8H_UART_ENABLE_UART2=1

; h8k64u_uart3_hello only
    -DSTC8H_UART_ENABLE_UART3=1
```

Do not enable UART2 or UART3 in the GPIO, ADC, EEPROM safe, or WDT examples unless that example calls the port. This keeps unused UART code out of those builds.

If the installed PlatformIO platform later provides an official `STC8H8K64U` board, compare it with the local manifest before removing the local one. Upload configuration must still be verified before flashing.

- [ ] **Step 6: Create GPIO blink example**

Create `examples/platformio/h8k64u_gpio_blink/src/main.c`:

```c
#include "board_pins.h"
#include "stc8h_delay.h"
#include "stc8h_gpio.h"

void main(void)
{
    stc8h_gpio_set_mode(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN, STC8H_GPIO_MODE_PUSH_PULL);
    stc8h_gpio_write(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN, 0u);

    while (1) {
        stc8h_gpio_toggle(BOARD_TEST_GPIO_PORT, BOARD_TEST_GPIO_PIN);
        stc8h_delay_ms(250u);
    }
}
```

- [ ] **Step 7: Create ADC read example**

Create `examples/platformio/h8k64u_adc_read/src/main.c`:

```c
#include "stc8h_adc.h"
#include "stc8h_uart.h"

static void print_hex16(stc8h_u16 value)
{
    static const STC8H_CODE char hex[] = "0123456789ABCDEF";
    stc8h_uart_putc(STC8H_UART1, hex[(value >> 12) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[(value >> 8) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[(value >> 4) & 0x0Fu]);
    stc8h_uart_putc(STC8H_UART1, hex[value & 0x0Fu]);
}

void main(void)
{
    stc8h_u16 value;
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_adc_init();
    while (1) {
        value = stc8h_adc_read(0u);
        stc8h_uart_write_code(STC8H_UART1, "ADC0=0x");
        print_hex16(value);
        stc8h_uart_write_code(STC8H_UART1, "\r\n");
    }
}
```

- [ ] **Step 8: Create EEPROM safe example**

Create `examples/platformio/h8k64u_eeprom_safe/src/main.c`:

```c
#include "stc8h_uart.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_uart_write_code(STC8H_UART1, "H8K64U EEPROM write disabled\r\n");
    while (1) {
    }
}
```

This safe example must not compile or call `hal/stc8h_eeprom.c`. Add a destructive EEPROM example only after the actual EEPROM allocation is confirmed.

- [ ] **Step 9: Create WDT feed example**

Create `examples/platformio/h8k64u_wdt_feed/src/main.c`:

```c
#include "stc8h_delay.h"
#include "stc8h_uart.h"
#include "stc8h_wdt.h"

void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
    stc8h_uart_write_code(STC8H_UART1, "H8K64U WDT feed\r\n");
    stc8h_wdt_start(STC8H_WDT_PRESCALE_256, 0u);
    while (1) {
        stc8h_wdt_feed();
        stc8h_delay_ms(100u);
    }
}
```

- [ ] **Step 10: Create wrapper source files**

Create the wrapper files that compile library modules into each PlatformIO example.

`examples/platformio/h8k64u_uart2_hello/src/stc8h_uart_wrap.c`:

```c
#include "../../../../hal/stc8h_uart.c"
```

`examples/platformio/h8k64u_uart3_hello/src/stc8h_uart_wrap.c`:

```c
#include "../../../../hal/stc8h_uart.c"
```

`examples/platformio/h8k64u_gpio_blink/src/stc8h_gpio_wrap.c`:

```c
#include "../../../../hal/stc8h_gpio.c"
```

`examples/platformio/h8k64u_gpio_blink/src/stc8h_gpio_toggle_wrap.c`:

```c
#include "../../../../hal/stc8h_gpio_toggle.c"
```

`examples/platformio/h8k64u_gpio_blink/src/stc8h_delay_wrap.c`:

```c
#include "../../../../core/stc8h_delay.c"
```

`examples/platformio/h8k64u_adc_read/src/stc8h_adc_wrap.c`:

```c
#include "../../../../hal/stc8h_adc.c"
```

`examples/platformio/h8k64u_adc_read/src/stc8h_delay_wrap.c`:

```c
#include "../../../../core/stc8h_delay.c"
```

`examples/platformio/h8k64u_adc_read/src/stc8h_uart_wrap.c`:

```c
#include "../../../../hal/stc8h_uart.c"
```

`examples/platformio/h8k64u_eeprom_safe/src/stc8h_uart_wrap.c`:

```c
#include "../../../../hal/stc8h_uart.c"
```

`examples/platformio/h8k64u_wdt_feed/src/stc8h_delay_wrap.c`:

```c
#include "../../../../core/stc8h_delay.c"
```

`examples/platformio/h8k64u_wdt_feed/src/stc8h_uart_wrap.c`:

```c
#include "../../../../hal/stc8h_uart.c"
```

`examples/platformio/h8k64u_wdt_feed/src/stc8h_wdt_wrap.c`:

```c
#include "../../../../hal/stc8h_wdt.c"
```

- [ ] **Step 11: Build examples**

Run:

```sh
(cd examples/platformio/h8k64u_uart2_hello && pio run)
(cd examples/platformio/h8k64u_uart3_hello && pio run)
(cd examples/platformio/h8k64u_gpio_blink && pio run)
(cd examples/platformio/h8k64u_adc_read && pio run)
(cd examples/platformio/h8k64u_eeprom_safe && pio run)
(cd examples/platformio/h8k64u_wdt_feed && pio run)
```

Expected: all compile.

- [ ] **Step 12: Commit**

```sh
git add board/stc8h8k64u_lqfp48_base examples/platformio/boards examples/platformio/h8k64u_uart2_hello examples/platformio/h8k64u_uart3_hello examples/platformio/h8k64u_gpio_blink examples/platformio/h8k64u_adc_read examples/platformio/h8k64u_eeprom_safe examples/platformio/h8k64u_wdt_feed
git commit -m "feat: add h8k64u board examples"
```

### Task 7: Add Automation And Regression Checks

**Files:**
- Modify: `tools/check_examples.sh`
- Modify: `docs/RESOURCE_REPORT.md`

- [ ] **Step 1: Add H8K64U compile calls**

Add to `tools/check_examples.sh`:

```sh
run_platformio_example examples/platformio/h8k64u_uart2_hello
run_platformio_example examples/platformio/h8k64u_uart3_hello
run_platformio_example examples/platformio/h8k64u_gpio_blink
run_platformio_example examples/platformio/h8k64u_adc_read
run_platformio_example examples/platformio/h8k64u_eeprom_safe
run_platformio_example examples/platformio/h8k64u_wdt_feed
```

- [ ] **Step 2: Add UART symbol absence check**

Add a helper that compiles UART1-only and checks no UART2/UART3 symbols are emitted:

```sh
check_uart2_uart3_trim() {
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "${tmp_dir}"' EXIT
    cat > "${tmp_dir}/uart1_only.c" <<EOF
#define STC8H_CHIP_STC8H1K08 1
#define STC8H_CHIP_STC8H8K64U 0
#define STC8H_UART_ASSUME_UART1 1
#include "${ROOT_DIR}/hal/stc8h_uart.c"
void main(void)
{
    (void)stc8h_uart_init(STC8H_UART1);
}
EOF
    sdcc -mmcs51 --std-sdcc11 -I"${ROOT_DIR}/core" -I"${ROOT_DIR}/hal" \
        -c -o "${tmp_dir}/uart1_only.rel" "${tmp_dir}/uart1_only.c"
    if grep -E "S2CON|S3CON|_stc8h_uart2|_stc8h_uart3" \
        "${tmp_dir}/uart1_only.asm" "${tmp_dir}/uart1_only.sym"; then
        echo "UART2/UART3 symbols leaked into UART1-only build" >&2
        exit 1
    fi
}
```

Call `check_uart2_uart3_trim` before the PlatformIO example list. This verifies that the UART1-only fast path still compiles after explicit chip profiles and that UART2/UART3 code is not linked when disabled.

- [ ] **Step 3: Run full verification**

Run:

```sh
tools/check_examples.sh
```

Expected: all existing STC8H1K08 examples and new H8K64U compile examples pass.

- [ ] **Step 4: Commit**

```sh
git add tools/check_examples.sh docs/RESOURCE_REPORT.md
git commit -m "test: add h8k64u regression checks"
```

### Task 8: Hardware Validation Handoff

**Files:**
- Modify: `docs/16_HARDWARE_TEST.md`
- Modify: `docs/15_REMAINING_WORK.md`

- [ ] **Step 1: Add hardware validation checklist**

Add to `docs/16_HARDWARE_TEST.md`:

```markdown
## STC8H8K64U-45I-LQFP48

- Confirm `ADC_VRef+` is tied to a valid reference or VCC.
- Confirm normal default serial/USB download path before testing examples.
- Confirm P3.0/P3.1/P3.2 are not all low during reset if USB download is not used.
- Build and flash UART2 example on selected UART2 pin group.
- Build and flash UART3 example on selected UART3 pin group.
- Verify UART2 and UART3 can run concurrently if both communication ports are needed.
- Run EEPROM write/erase only with an explicitly selected destructive-test environment and a confirmed EEPROM range.
```

- [ ] **Step 2: Add remaining-work note**

Add to `docs/15_REMAINING_WORK.md`:

```markdown
## STC8H8K64U-LQFP48 follow-up

- Hardware validation is required on the actual `STC8H8K64U-45I-LQFP48` board before marking support as hardware-tested.
- Confirm final EEPROM/IAP size from ISP/project configuration before enabling destructive EEPROM examples.
- Decide per board whether UART2 or UART3 is wired to RS485 or 433 MHz modules; the base library must remain role-neutral.
```

- [ ] **Step 3: Commit**

```sh
git add docs/16_HARDWARE_TEST.md docs/15_REMAINING_WORK.md
git commit -m "docs: add h8k64u hardware validation checklist"
```

## Self-review

Spec coverage:

- H8K64U-LQFP48 opt-in support is covered by Tasks 1 and 2.
- UART2/UART3 generic support is covered by Tasks 3, 5, and 6.
- UART roles staying application-level is covered in the design doc and board pin example macros.
- Existing STC8H1K08 compatibility is covered by Tasks 2, 4, 5, and 7.
- Hardware validation and EEPROM uncertainty are covered by Task 8.

Placeholder scan:

- Timer2/Timer3 SFR addresses and UART2/UART3 control bits are now specified from the official manual sections used by the implementation.
- UART2/UART3 baud reload macros, range checks, and `STC8H_UART_ASSUME_UART1` conflict checks are specified before implementation.
- EEPROM size protection now runs after EEPROM feature macros are defined, so `STC8H_EEPROM_SIZE=0` cannot silently bypass disabled-default assumptions.
- No task says to handle errors or tests without concrete commands.

Type consistency:

- UART enum names are consistently `STC8H_UART1`, `STC8H_UART2`, `STC8H_UART3`.
- Board role macros are consistently `BOARD_RS485_UART` and `BOARD_RF433_UART`.
- Feature macros are consistently `STC8H_UART_ENABLE_UART2` and `STC8H_UART_ENABLE_UART3`.
- H8K64U examples follow the existing PlatformIO wrapper-source pattern used by current examples.

Risk review:

- The plan adds a local H8K64U PlatformIO board manifest so examples do not silently inherit STC8H1K08 memory limits.
- Existing temporary SDCC compile checks are migrated in Task 2 before later `tools/check_examples.sh` runs.
- H8K64U board defaults keep UART2/UART3 disabled; each example enables only the port it uses to protect ROM size.
- UART2/UART3 register setup is now grounded in official examples, but hardware timing still must be verified on the target board.
- EEPROM size must be supplied by board/application config before enabling EEPROM APIs or destructive tests.
