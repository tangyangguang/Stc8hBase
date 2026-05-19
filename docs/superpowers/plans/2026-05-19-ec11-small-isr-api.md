# EC11 Small ISR API Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add an explicit EC11 small scanner API that is safe to call from SDCC mcs51 Timer ISRs.

**Architecture:** Keep the ordinary small API unchanged and document it as polling/main-loop only. Add a disabled-by-default ISR API that requires a `STC8H_DATA drv_ec11_small_t *` state object and uses a reentrant compiler marker on SDCC. Verify the generated SDCC path avoids generic pointer helpers and the old transition helper parameter overlay.

**Tech Stack:** C11-style embedded C, SDCC mcs51, Keil C51 compatibility macros, host C tests, PlatformIO example checks.

---

### Task 1: Compiler Marker

**Files:**
- Modify: `core/stc8h_compiler.h`

- [ ] Add a `STC8H_REENTRANT` macro:

```c
#if defined(__SDCC)
#define STC8H_REENTRANT __reentrant
#elif defined(__C51__) || defined(__CX51__)
#define STC8H_REENTRANT reentrant
#else
#define STC8H_REENTRANT
#endif
```

### Task 2: Failing Behavior And Codegen Tests

**Files:**
- Create: `tests/host/test_drv_ec11_small_isr.c`
- Modify: `tools/check_host_tests.sh`
- Modify: `tools/check_examples.sh`

- [ ] Add a host test that includes `drivers/drv_ec11.c` with `DRV_EC11_ENABLE_SMALL_ISR_API=1` and calls `drv_ec11_scan_delta_small_isr()`.
- [ ] Add the host test to `tools/check_host_tests.sh`.
- [ ] Add `check_ec11_small_isr_api()` to `tools/check_examples.sh`. It must compile a temporary SDCC file that includes only the ISR small API, then fail if `__gptrget`, `__gptrput`, or `_drv_ec11_transition_PARM_2` appears in the generated files.
- [ ] Run `sh tools/check_host_tests.sh` and confirm it fails because `drv_ec11_scan_delta_small_isr()` is not declared or defined.

### Task 3: Driver API

**Files:**
- Modify: `drivers/drv_ec11.h`
- Modify: `drivers/drv_ec11.c`

- [ ] Add `DRV_EC11_ENABLE_SMALL_ISR_API`, default `0`.
- [ ] Declare `drv_ec11_small_init_isr()` and `drv_ec11_scan_delta_small_isr()` when the ISR API is enabled.
- [ ] Implement both functions under `#if DRV_EC11_ENABLE_SMALL_ISR_API`.
- [ ] Use `STC8H_DATA drv_ec11_small_t *` in the signatures.
- [ ] Use `STC8H_REENTRANT` in both signatures.
- [ ] Inline the same 16-entry transition table logic inside the ISR scanner and do not call `drv_ec11_transition()`.

### Task 4: Documentation

**Files:**
- Modify: `docs/05_USAGE.md`
- Modify: `docs/13_RESOURCE_POLICY.md`
- Modify: `docs/10_REFERENCES.md`

- [ ] Document that ordinary EC11 small scan is not ISR-safe on SDCC mcs51 small model.
- [ ] Document the ISR API usage and `static STC8H_DATA drv_ec11_small_t encoder;` requirement.
- [ ] Add `DRV_EC11_ENABLE_SMALL_ISR_API` to the resource policy table.
- [ ] Record the SDCC reentrancy/overlay reference in `docs/10_REFERENCES.md`.

### Task 5: Verification And Delivery

**Files:**
- Inspect all modified files.

- [ ] Run `sh tools/check_host_tests.sh`.
- [ ] Run `sh tools/check_examples.sh`.
- [ ] Run `git status --short`.
- [ ] Commit only related files.
- [ ] Push branch `codex/ec11-small-isr-api`.
