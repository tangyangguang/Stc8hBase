# Driver API Trimming Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add default-on API trimming macros for nRF24L01, TM1637, and the known PWM small-memory blocker.

**Architecture:** Follow the existing `proto_rf_link` pattern: define defaults in public headers, wrap declarations and definitions with the same conditions, and compile helper functions only when enabled APIs need them. Keep PWM trimming narrow and retain existing group/channel masks.

**Tech Stack:** C11 for SDCC/mcs51, PlatformIO examples, shell map-symbol checks.

---

### Task 1: Add Failing Symbol Checks

**Files:**
- Modify: `tools/check_examples.sh`
- Modify: `examples/platformio/rf_link_nrf24_small/platformio.ini`
- Modify: `examples/platformio/tm1637_number/platformio.ini`
- Modify: `examples/platformio/pwm_pwma_pwmb_small/platformio.ini`

- [ ] **Step 1: Configure existing examples to disable unused APIs**

Add build flags that disable observed unused nRF24 diagnostic/feature APIs in `rf_link_nrf24_small`, high-level TM1637 formatting APIs in `tm1637_number`, and PWM disable in `pwm_pwma_pwmb_small`.

- [ ] **Step 2: Add map checks**

Extend `tools/check_examples.sh` with `check_map_absent` calls for the disabled symbols:

```sh
_drv_nrf24l01_read_fifo_status
_drv_nrf24l01_read_observe_tx
_drv_nrf24l01_enable_dynamic_payload
_drv_nrf24l01_disable_dynamic_payload
_drv_nrf24l01_disable_ack_payload
_drv_tm1637_display_digits
_drv_tm1637_display_number
_stc8h_pwm_disable
```

- [ ] **Step 3: Run the focused examples and confirm failure before implementation**

Run:

```sh
tools/check_examples.sh
```

Expected before implementation: the build or symbol checks fail because the new macros are not implemented or disabled symbols still exist.

### Task 2: Implement nRF24L01 Trimming

**Files:**
- Modify: `drivers/drv_nrf24l01.h`
- Modify: `drivers/drv_nrf24l01.c`

- [ ] **Step 1: Add default-on macros in the header**

Define `DRV_NRF24L01_ENABLE_*` macros before declarations.

- [ ] **Step 2: Wrap declarations and definitions**

Guard optional public functions and their private helper dependencies with matching `#if` conditions.

- [ ] **Step 3: Re-run focused checks**

Run:

```sh
tools/check_examples.sh
```

Expected after this task: nRF24 disabled symbols are absent; any remaining failures are from TM1637/PWM tasks.

### Task 3: Implement TM1637 Trimming

**Files:**
- Modify: `drivers/drv_tm1637.h`
- Modify: `drivers/drv_tm1637.c`

- [ ] **Step 1: Add default-on macros and dependency checks**

Define formatting-layer switches and compile-time errors for invalid combinations.

- [ ] **Step 2: Wrap declarations, definitions, and helpers**

Guard `display_digits`, `display_number`, `encode_digit`, `clear`, and `drv_tm1637_divmod10`.

- [ ] **Step 3: Re-run focused checks**

Run:

```sh
tools/check_examples.sh
```

Expected after this task: TM1637 disabled symbols are absent; any remaining failures are from PWM.

### Task 4: Implement Narrow PWM Trimming

**Files:**
- Modify: `hal/stc8h_pwm.h`
- Modify: `hal/stc8h_pwm.c`

- [ ] **Step 1: Add `STC8H_PWM_ENABLE_DISABLE`**

Default it to `1`, guard the declaration and definition of `stc8h_pwm_disable`.

- [ ] **Step 2: Compile state per enabled group**

Guard the `PWMA` and `PWMB` period/prescaler state variables and accessors so unused groups do not allocate internal RAM.

- [ ] **Step 3: Re-run focused checks**

Run:

```sh
tools/check_examples.sh
```

Expected after this task: PWM disabled symbol is absent and all examples pass.

### Task 5: Validate Application Reproduction

**Files:**
- No intended foundation-library source changes.

- [ ] **Step 1: Run ToyRemote checks**

Run:

```sh
/Users/tyg/dir/codex_dir/Stc8hToyRemote/tools/check_all.sh
pio run -d /Users/tyg/dir/codex_dir/Stc8hToyRemote/receiver
```

Expected: both commands exit 0, controller is under 8192 bytes, receiver has no DSEG/OSEG overflow.

- [ ] **Step 2: Commit and push**

Run:

```sh
git status --short
git add docs/superpowers/specs/2026-05-16-driver-api-trimming-design.md docs/superpowers/plans/2026-05-16-driver-api-trimming.md drivers/drv_nrf24l01.h drivers/drv_nrf24l01.c drivers/drv_tm1637.h drivers/drv_tm1637.c hal/stc8h_pwm.h hal/stc8h_pwm.c tools/check_examples.sh examples/platformio/rf_link_nrf24_small/platformio.ini examples/platformio/tm1637_number/platformio.ini examples/platformio/pwm_pwma_pwmb_small/platformio.ini
git commit -m "Trim driver APIs for small targets"
git push
```
