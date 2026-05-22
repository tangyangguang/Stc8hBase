# PWM XFR Access Design

## Goal

Fix `stc8h_pwm` so every public API that touches `PWMA` or `PWMB` XFR registers enables `P_SW2.EAXFR` before the access. PWM must not depend on another module leaving EAXFR enabled.

## Evidence

The STC8H reference notes in this repo state that XFR registers require `P_SW2.EAXFR=1` before access. The PWM HAL uses `PWMA/PWMB` registers under the `0xFE..` XFR range. `stc8h_adc_init()` temporarily enables EAXFR for `ADCTIM`, then clears it, which exposes PWM runtime paths that write CCR/ENO/CCER/CR1 without reopening EAXFR.

## Affected Paths

- `stc8h_pwm_set_prescaler()` and `stc8h_pwm_set_period()` may read `PWMA_ENO/PWMB_ENO` through the running guard before their existing EAXFR set.
- `stc8h_pwm_init_channel()` already enables EAXFR before writing `PWMA_PS/PWMB_PS` and before calling mode/duty helpers.
- `stc8h_pwm_set_duty()` writes `PWMA_CCRx/PWMB_CCRx` and must enable EAXFR itself because it is a runtime public API.
- `stc8h_pwm_enable()` and `stc8h_pwm_disable()` write `PWMA/PWMB` output, compare-enable, and counter-control registers and must enable EAXFR themselves.

## Implementation

Add a tiny local helper for opening PWM XFR access and call it in each public PWM function before the first PWM XFR read or write. This keeps the API small, preserves the existing "leave EAXFR enabled after PWM access" behavior, and avoids save/restore overhead in hot paths.

## Verification

Add an SDCC codegen guard to `tools/check_examples.sh` that compiles `hal/stc8h_pwm.c` and fails unless the generated `stc8h_pwm_set_duty`, `stc8h_pwm_enable`, and `stc8h_pwm_disable` function bodies contain `orl _P_SW2,#0x80`. Then run the full project check script.
