# STC8H8K64U LQFP48 Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add explicit, opt-in `STC8H8K64U-45I-LQFP48` base-library support with generic UART2/UART3 communication ports and no regression to existing `STC8H1K08` projects.

**Architecture:** Keep the current thin HAL and compile-time configuration model. Add H8K64U defaults and board files separately, guard UART2/UART3 code behind feature macros, and keep RS485/433 role binding in application or board configuration.

**Tech Stack:** C for SDCC/Keil C51, STC8H official documentation, PlatformIO/Makefile examples, shell verification scripts.

---

## File Structure

- Modify `core/stc8h_config.h`: add chip-default selection and H8K64U defaults without changing STC8H1K08 fallback behavior.
- Modify `core/stc8h_sfr.h`: add only SFR/XFR definitions needed by H8K64U GPIO, ADC, EEPROM, UART2, UART3, Timer2, and Timer3.
- Modify `hal/stc8h_gpio.c`: add guarded P6/P7 support only if needed by selected package/board masks.
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
- Create: `examples/platformio/boards/STC8H8K64U.json` if PlatformIO does not already provide a suitable board definition in the installed platform.
- Modify `tools/check_examples.sh`: add compile-only H8K64U checks and symbol absence checks.
- Modify `docs/03_CHIP_SUPPORT.md`, `docs/10_REFERENCES.md`, `docs/vendor/stc/README.md`, and `docs/13_RESOURCE_POLICY.md`: document support level, source, and resource rules.

### Task 1: Document Source And Support Level

**Files:**
- Modify: `docs/03_CHIP_SUPPORT.md`
- Modify: `docs/10_REFERENCES.md`
- Modify: `docs/vendor/stc/README.md`
- Modify: `docs/13_RESOURCE_POLICY.md`

- [ ] **Step 1: Add support level text**

Add a subsection to `docs/03_CHIP_SUPPORT.md`:

```markdown
### STC8H8K64U-45I-LQFP48 opt-in support

`STC8H8K64U-45I-LQFP48` is supported as an explicit opt-in chip profile.
It is not the default target and does not change `STC8H1K08` behavior.

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

- [ ] **Step 3: Run documentation grep**

Run:

```sh
rg -n "STC8H8K64U|UART2|UART3|LQFP48" docs/03_CHIP_SUPPORT.md docs/10_REFERENCES.md docs/13_RESOURCE_POLICY.md docs/vendor/stc/README.md
```

Expected: the new support level and source notes appear.

- [ ] **Step 4: Commit docs**

```sh
git add docs/03_CHIP_SUPPORT.md docs/10_REFERENCES.md docs/vendor/stc/README.md docs/13_RESOURCE_POLICY.md
git commit -m "docs: plan stc8h8k64u lqfp48 support"
```

### Task 2: Add Chip Defaults Without Changing STC8H1K08

**Files:**
- Modify: `core/stc8h_config.h`
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

#define STC8H_UART_ENABLE_UART2 1
#define STC8H_UART_ENABLE_UART3 1
#define STC8H_UART2_PIN_GROUP 0u
#define STC8H_UART3_PIN_GROUP 0u

#define STC8H_ADC_BITS 12u
#define STC8H_ADC_CHANNEL_MASK 0xFFFFu

#define STC8H_EEPROM_SECTOR_SIZE 512u

#endif
```

- [ ] **Step 3: Update chip default logic**

In `core/stc8h_config.h`, replace the current default chip block with:

```c
#ifndef STC8H_CHIP_STC8H8K64U
#define STC8H_CHIP_STC8H8K64U 0
#endif

#ifndef STC8H_CHIP_STC8H1K08
#define STC8H_CHIP_STC8H1K08 1
#endif
```

- [ ] **Step 4: Run config checks**

Run:

```sh
sdcc -mmcs51 --std-sdcc11 -Icore -Iboard/stc8h8k64u_lqfp48_base -c /tmp/h8k64u_config_check.c
sdcc -mmcs51 --std-sdcc11 -Icore -c - <<'EOF'
#include "stc8h_config.h"
#if !STC8H_CHIP_STC8H1K08
#error "default chip changed"
#endif
void main(void) {}
EOF
```

Expected: both commands pass.

- [ ] **Step 5: Commit**

```sh
git add core/stc8h_config.h board/stc8h8k64u_lqfp48_base/board_config.h
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
```

Add Timer2/Timer3 registers only after verifying exact addresses from the official manual section used by the implementation.

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

- [ ] **Step 2: Make EEPROM size overridable**

Change `hal/stc8h_eeprom.h` constants to:

```c
#ifndef STC8H_EEPROM_SIZE
#define STC8H_EEPROM_SIZE 4096u
#endif

#ifndef STC8H_EEPROM_SECTOR_SIZE
#define STC8H_EEPROM_SECTOR_SIZE 512u
#endif
```

- [ ] **Step 3: Add ADC defaults**

In `hal/stc8h_adc.h`, add:

```c
#ifndef STC8H_ADC_BITS
#define STC8H_ADC_BITS 10u
#endif

#ifndef STC8H_ADC_CHANNEL_MASK
#define STC8H_ADC_CHANNEL_MASK 0xC103u
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
```

- [ ] **Step 4: Add UART2 init path**

Implement UART2 only under:

```c
#if STC8H_UART_ENABLE_UART2
/* UART2 init, putc, readable, getc using S2CON/S2BUF and Timer2. */
#endif
```

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
- Create: `examples/platformio/h8k64u_uart3_hello/platformio.ini`
- Create: `examples/platformio/h8k64u_uart3_hello/src/main.c`
- Create: `examples/platformio/h8k64u_gpio_blink/platformio.ini`
- Create: `examples/platformio/h8k64u_gpio_blink/src/main.c`

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

#endif
```

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
    -I../../..//core
    -I../../..//hal
    -I../../..//board/stc8h8k64u_lqfp48_base
    -DSTC8H_CONFIG_INCLUDE=\"board_config.h\"
    -DSTC8H_PINS_INCLUDE=\"board_pins.h\"
```

If the installed PlatformIO platform later provides an official `STC8H8K64U` board, compare it with the local manifest before removing the local one. Upload configuration must still be verified before flashing.

- [ ] **Step 6: Build examples**

Run:

```sh
(cd examples/platformio/h8k64u_uart2_hello && pio run)
(cd examples/platformio/h8k64u_uart3_hello && pio run)
```

Expected: both compile.

- [ ] **Step 7: Commit**

```sh
git add board/stc8h8k64u_lqfp48_base examples/platformio/boards examples/platformio/h8k64u_uart2_hello examples/platformio/h8k64u_uart3_hello
git commit -m "feat: add h8k64u uart examples"
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
```

- [ ] **Step 2: Add UART symbol absence check**

Add a helper that compiles UART1-only and checks no UART2/UART3 symbols are emitted:

```sh
check_uart2_uart3_trim() {
    tmp_dir=$(mktemp -d)
    trap 'rm -rf "${tmp_dir}"' EXIT
    cat > "${tmp_dir}/uart1_only.c" <<EOF
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

Call `check_uart2_uart3_trim` before the PlatformIO example list.

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

- The plan intentionally leaves exact Timer2/Timer3 SFR addresses to be verified in Task 3 before implementation because the code must not be written from memory. This is not an implementation placeholder; it is a required evidence gate.
- No task says to handle errors or tests without concrete commands.

Type consistency:

- UART enum names are consistently `STC8H_UART1`, `STC8H_UART2`, `STC8H_UART3`.
- Board role macros are consistently `BOARD_RS485_UART` and `BOARD_RF433_UART`.
- Feature macros are consistently `STC8H_UART_ENABLE_UART2` and `STC8H_UART_ENABLE_UART3`.

Risk review:

- The plan adds a local H8K64U PlatformIO board manifest so examples do not silently inherit STC8H1K08 memory limits.
- UART3 on Timer3 must be verified from the official manual before code is written. This is the main technical risk.
- EEPROM size must be supplied by board/application config before destructive tests.
