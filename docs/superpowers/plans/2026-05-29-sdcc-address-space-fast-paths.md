# SDCC Address Space Fast Paths Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add explicit DATA/XDATA/CODE fixed-path APIs that remove SDCC/MCS51 generic pointer helpers from small STC8H RF, display, and PWM builds.

**Architecture:** Keep existing generic APIs as defaults. Add opt-in address-space-specific APIs with visible names and qualifiers, and make minimal wrappers able to disable generic protocol initialization. Verify behavior on hosted builds and verify SDCC codegen by checking generated assembly for `__gptrget` / `__gptrput`.

**Tech Stack:** C89-style embedded C, SDCC MCS51, hosted `cc` tests, PlatformIO examples, shell verification scripts.

---

## File Structure

- Modify `protocols/proto_rf_link.h` and `protocols/proto_rf_link.c`: add `PROTO_RF_LINK_ENABLE_INIT` plus XDATA init/set-id/fixed send/fixed poll APIs.
- Modify `drivers/drv_nrf24l01.h` and `drivers/drv_nrf24l01.c`: add XDATA fixed payload APIs and CODE fixed address API.
- Modify `drivers/drv_tm1637.h` and `drivers/drv_tm1637.c`: add DATA raw4 display API.
- Modify `hal/stc8h_pwm.c`: make the internal 16-bit XFR writer use XDATA volatile pointers.
- Create `tests/host/test_proto_rf_link_address_space.c`: hosted protocol behavior tests.
- Modify `tests/host/test_drv_nrf24l01_core.c`: hosted nRF24 behavior tests for new APIs.
- Create `tests/host/test_drv_tm1637_address_space.c`: hosted TM1637 event-equivalence tests.
- Modify `tools/check_host_tests.sh`: register new hosted tests and add SDCC codegen/size guards.
- Modify `docs/13_RESOURCE_POLICY.md`, `docs/22_RF_LINK_PROTOCOL.md`, `docs/21_NRF24L01_DESIGN.md`, and `docs/05_USAGE.md`: document when to choose address-space APIs.

---

### Task 1: Protocol XDATA API

**Files:**
- Modify: `protocols/proto_rf_link.h`
- Modify: `protocols/proto_rf_link.c`
- Create: `tests/host/test_proto_rf_link_address_space.c`

- [ ] **Step 1: Write failing hosted protocol tests**

Add a hosted test that enables generic fixed fast path and XDATA fixed path together:

```c
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED 1
#define PROTO_RF_LINK_ENABLE_SEND_DATA_FIXED_FAST_PATH 1
#define PROTO_RF_LINK_ENABLE_POLL_DATA_FIXED 1
#define PROTO_RF_LINK_ENABLE_XDATA_FIXED_API 1
#include "../../protocols/proto_rf_link.c"
```

Test that `proto_rf_link_init_xdata()`, `proto_rf_link_set_ids_xdata()`, `proto_rf_link_send_data_fixed_xdata()`, and `proto_rf_link_poll_data_fixed_xdata()` produce the same packet and payload behavior as the existing generic fixed functions.

- [ ] **Step 2: Run test to verify it fails**

Run:

```sh
sh tools/check_host_tests.sh
```

Expected: compile failure naming missing `proto_rf_link_*_xdata` functions or macro declarations.

- [ ] **Step 3: Implement protocol API**

Add defaults and declarations:

```c
#ifndef PROTO_RF_LINK_ENABLE_INIT
#define PROTO_RF_LINK_ENABLE_INIT 1
#endif

#ifndef PROTO_RF_LINK_ENABLE_XDATA_FIXED_API
#define PROTO_RF_LINK_ENABLE_XDATA_FIXED_API 0
#endif
```

Wrap the existing generic initializer:

```c
#if PROTO_RF_LINK_ENABLE_INIT
void proto_rf_link_init(proto_rf_link_t *link);
#endif
```

Add XDATA implementations that duplicate the fixed-path logic with qualified pointers:

```c
void proto_rf_link_init_xdata(STC8H_XDATA proto_rf_link_t *link);
void proto_rf_link_set_ids_xdata(STC8H_XDATA proto_rf_link_t *link, stc8h_u8 local_id, stc8h_u8 peer_id);
stc8h_status_t proto_rf_link_send_data_fixed_xdata(STC8H_XDATA proto_rf_link_t *link, STC8H_XDATA stc8h_u8 *packet, const STC8H_XDATA stc8h_u8 *data);
stc8h_status_t proto_rf_link_poll_data_fixed_xdata(STC8H_XDATA proto_rf_link_t *link, const STC8H_XDATA stc8h_u8 *packet, STC8H_XDATA stc8h_u8 *data);
```

- [ ] **Step 4: Run hosted tests**

Run:

```sh
sh tools/check_host_tests.sh
```

Expected: protocol address-space test passes.

---

### Task 2: nRF24 XDATA/CODE API

**Files:**
- Modify: `drivers/drv_nrf24l01.h`
- Modify: `drivers/drv_nrf24l01.c`
- Modify: `tests/host/test_drv_nrf24l01_core.c`

- [ ] **Step 1: Write failing nRF24 hosted tests**

Enable the new macros before including the driver in the existing core test:

```c
#define DRV_NRF24L01_ENABLE_FIXED_PAYLOAD_API 1
#define DRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API 1
#define DRV_NRF24L01_ENABLE_CODE_ADDRESS_API 1
```

Add tests for:

```c
drv_nrf24l01_write_payload_fixed_xdata(payload);
drv_nrf24l01_read_payload_fixed_xdata(payload);
drv_nrf24l01_config_pipe0_fixed_code(addr);
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```sh
sh tools/check_host_tests.sh
```

Expected: compile failure naming missing `drv_nrf24l01_*_xdata` or `_code` functions.

- [ ] **Step 3: Implement nRF24 helpers**

Add macro defaults and declarations:

```c
#ifndef DRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API
#define DRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API 0
#endif

#ifndef DRV_NRF24L01_ENABLE_CODE_ADDRESS_API
#define DRV_NRF24L01_ENABLE_CODE_ADDRESS_API 0
#endif
```

Add private helpers with qualified pointers:

```c
static stc8h_u8 drv_nrf24l01_read_buf_xdata(stc8h_u8 cmd, STC8H_XDATA stc8h_u8 *buf, stc8h_u8 len);
static stc8h_u8 drv_nrf24l01_write_buf_xdata(stc8h_u8 cmd, const STC8H_XDATA stc8h_u8 *buf, stc8h_u8 len);
static stc8h_u8 drv_nrf24l01_write_buf_code(stc8h_u8 cmd, const STC8H_CODE stc8h_u8 *buf, stc8h_u8 len);
```

Implement public wrappers around the fixed commands `0xA0`, `0x61`, `0x20 | 0x10`, and `0x20 | 0x0A`.

- [ ] **Step 4: Run hosted tests**

Run:

```sh
sh tools/check_host_tests.sh
```

Expected: nRF24 core tests pass.

---

### Task 3: TM1637 DATA API And PWM Internal Pointer

**Files:**
- Modify: `drivers/drv_tm1637.h`
- Modify: `drivers/drv_tm1637.c`
- Create: `tests/host/test_drv_tm1637_address_space.c`
- Modify: `hal/stc8h_pwm.c`

- [ ] **Step 1: Write failing TM1637 hosted test**

Create a test with fake board macros that append each GPIO transition to an event buffer, then compare:

```c
drv_tm1637_display_raw4(segments);
drv_tm1637_display_raw4_data(segments);
```

Expected behavior: identical return status and identical GPIO event sequence.

- [ ] **Step 2: Run test to verify it fails**

Run:

```sh
sh tools/check_host_tests.sh
```

Expected: compile failure naming missing `drv_tm1637_display_raw4_data`.

- [ ] **Step 3: Implement TM1637 DATA API**

Add:

```c
#ifndef DRV_TM1637_ENABLE_DISPLAY_RAW4_DATA
#define DRV_TM1637_ENABLE_DISPLAY_RAW4_DATA 0
#endif

stc8h_status_t drv_tm1637_display_raw4_data(const STC8H_DATA stc8h_u8 segments[4]);
```

Implement it with a small DATA-specific four-byte write loop using the same command sequence as raw4.

- [ ] **Step 4: Update PWM internal helper**

Change:

```c
static void stc8h_pwm_write16(volatile STC8H_XDATA stc8h_u8 *high,
                              volatile STC8H_XDATA stc8h_u8 *low,
                              stc8h_u16 value);
```

No public PWM header change is needed.

- [ ] **Step 5: Run hosted tests**

Run:

```sh
sh tools/check_host_tests.sh
```

Expected: hosted tests pass.

---

### Task 4: SDCC Codegen And Size Guards

**Files:**
- Modify: `tools/check_host_tests.sh`

- [ ] **Step 1: Add failing SDCC guards**

Add temporary SDCC compilation checks for:

```sh
proto_xdata_fixed.c
nrf24_xdata_code.c
tm1637_data_raw4.c
pwm_fixed_only.c
```

Extract function bodies from generated `.asm` and fail on:

```sh
grep -Eq '__gptr(get|put)'
```

Add a whole-wrapper check with `PROTO_RF_LINK_ENABLE_INIT=0`, `PROTO_RF_LINK_ENABLE_SET_IDS=0`, generic fixed APIs disabled, and `PROTO_RF_LINK_ENABLE_XDATA_FIXED_API=1`; fail if the generated assembly contains any `__gptrget` or `__gptrput`. Add a size check that compares generic protocol fixed sender assembly body lines against XDATA fixed sender assembly body lines and requires the XDATA body to be smaller.

- [ ] **Step 2: Run guard to verify failure before implementation**

Run:

```sh
sh tools/check_host_tests.sh
```

Expected before implementation: missing function failures. Expected after implementation: no generic pointer helper in new function bodies and XDATA sender smaller than generic sender.

- [ ] **Step 3: Keep guards stable**

Use function-body extraction instead of whole-file grep for modules that still intentionally emit generic default APIs. Whole-file grep is valid only for minimal wrappers that explicitly disable generic APIs such as `PROTO_RF_LINK_ENABLE_INIT=0`.

- [ ] **Step 4: Run SDCC guards**

Run:

```sh
sh tools/check_host_tests.sh
```

Expected: all hosted and SDCC trim checks pass.

---

### Task 5: Documentation, Examples, And Full Verification

**Files:**
- Modify: `docs/13_RESOURCE_POLICY.md`
- Modify: `docs/22_RF_LINK_PROTOCOL.md`
- Modify: `docs/21_NRF24L01_DESIGN.md`
- Modify: `docs/05_USAGE.md`
- Optionally modify: `examples/platformio/rf_link_nrf24_small/platformio.ini`
- Optionally modify: `examples/platformio/rf_link_nrf24_small/src/main.c`
- Optionally modify: `examples/platformio/tm1637_number/platformio.ini`

- [ ] **Step 1: Document new macros and APIs**

Add resource policy rows for:

```text
PROTO_RF_LINK_ENABLE_INIT
PROTO_RF_LINK_ENABLE_XDATA_FIXED_API
DRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API
DRV_NRF24L01_ENABLE_CODE_ADDRESS_API
DRV_TM1637_ENABLE_DISPLAY_RAW4_DATA
```

- [ ] **Step 2: Update usage guidance**

Document that small SDCC applications should place radio packets and payloads in XDATA, fixed addresses in CODE, and tiny display buffers in DATA only when they are truly hot-path internal RAM buffers.

- [ ] **Step 3: Update at least one small example**

Update `rf_link_nrf24_small` to use the XDATA/CODE APIs and set:

```ini
-DPROTO_RF_LINK_ENABLE_INIT=0
-DPROTO_RF_LINK_ENABLE_XDATA_FIXED_API=1
-DDRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API=1
-DDRV_NRF24L01_ENABLE_CODE_ADDRESS_API=1
```

- [ ] **Step 4: Run full verification**

Run:

```sh
sh tools/check_host_tests.sh
pio run -d examples/platformio/rf_link_nrf24_small
pio run -d examples/platformio/tm1637_number
pio run -d examples/platformio/pwm_pwma_pwmb_small
```

If time permits, run:

```sh
sh tools/check_examples.sh
```

- [ ] **Step 5: Commit and push**

Run:

```sh
git status --short
git add protocols/proto_rf_link.h protocols/proto_rf_link.c drivers/drv_nrf24l01.h drivers/drv_nrf24l01.c drivers/drv_tm1637.h drivers/drv_tm1637.c hal/stc8h_pwm.c tests/host tools/check_host_tests.sh docs examples
git commit -m "feat: add sdcc address-space fast paths"
git push -u origin codex/sdcc-address-space-fast-paths
```

Expected: branch pushed to `origin/codex/sdcc-address-space-fast-paths`.

---

## Self-Review

- Spec coverage: protocol XDATA, nRF24 XDATA/CODE, TM1637 DATA, PWM XDATA SFR writer, hosted tests, SDCC codegen/size checks, and documentation are all covered.
- Placeholder scan: no unresolved markers are intentionally left in this plan.
- Type consistency: the plan uses `STC8H_XDATA`, `STC8H_DATA`, and `STC8H_CODE` directly, matching the design.
