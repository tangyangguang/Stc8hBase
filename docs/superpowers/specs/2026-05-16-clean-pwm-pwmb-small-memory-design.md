# Clean PWM and Small-Memory Trimming Design

## Goal

Support STC8H1K08 SDCC builds that use PWMA and PWMB at different frequencies without application-local PWM register code, and keep small-memory driver builds from pulling unused APIs into internal RAM.

## PWM API

`stc8h_pwm` is redesigned as a group-aware HAL. The old single-argument channel API is removed because it encoded PWMA-only behavior and cannot express independent PWMA/PWMB periods cleanly.

The public API is:

```c
stc8h_status_t stc8h_pwm_set_prescaler(stc8h_u8 group, stc8h_u16 prescaler);
stc8h_status_t stc8h_pwm_set_period(stc8h_u8 group, stc8h_u16 period);
stc8h_status_t stc8h_pwm_init_channel(stc8h_u8 group, stc8h_u8 channel, stc8h_u8 pin_select);
stc8h_status_t stc8h_pwm_set_duty(stc8h_u8 group, stc8h_u8 channel, stc8h_u16 duty);
stc8h_status_t stc8h_pwm_enable(stc8h_u8 group, stc8h_u8 channel);
stc8h_status_t stc8h_pwm_disable(stc8h_u8 group, stc8h_u8 channel);
```

Groups are `STC8H_PWM_GROUP_A` and `STC8H_PWM_GROUP_B`. Channels are the hardware channel numbers: `1..4` for PWMA and `5..8` for PWMB. Pin selection is explicit and uses small constants that map directly to `PWMA_PS` or `PWMB_PS` bit fields. No automatic pin lookup is added.

Each group owns one saved period and prescaler. `stc8h_pwm_set_duty()` clamps duty against that group's period. `stc8h_pwm_set_period()` and `stc8h_pwm_set_prescaler()` reject changing a running group while any output in that group is enabled.

Only PWM mode 1, edge-aligned up-counting, P outputs are implemented. Complementary outputs, dead time, brake behavior beyond main-output enable, interrupts, capture, and sync features stay out of scope.

## Required Receiver Case

At `STC8H_SYSCLK_HZ=11059200`, the motor outputs use prescaler `0`, so period counts are direct timer counts minus one:

- PWMA1 servo on `P1.0/PWM1P`: 50Hz period `221183`, which exceeds 16-bit. The example therefore uses prescaler `11` and period `18431` ticks. Servo duty `500..2500us` maps to `461..2304`.
- PWMB6 MOS on `P5.4/PWM6_2`: 10kHz period `1105`.
- PWMB7 AT8236 IN1 on `P3.3/PWM7_2`: 10kHz period `1105`.
- PWMB8 AT8236 IN2 on `P3.4/PWM8_2`: 10kHz period `1105`.

The HAL accepts raw period and prescaler counts. It does not hide frequency math behind helper APIs, so applications keep clock-rate choices explicit and small-memory builds avoid extra division code.

## Small-Memory Trimming

`drv_ec11` gains compile-time API gates for setters that are often unused in fixed hardware builds:

- `DRV_EC11_ENABLE_SET_FAST`
- `DRV_EC11_ENABLE_SET_REVERSE`
- `DRV_EC11_ENABLE_SET_STEPS_PER_DETENT`

All default to `1`. Small-memory PlatformIO wrappers can set them to `0` before including `drv_ec11.c`; the header hides disabled declarations and the implementation omits the functions.

`drv_button` and `stc8h_adc` are not trimmed until map/mem evidence shows they are part of the DSEG/OSEG failure. This avoids adding knobs that are not yet justified by the current problem.

## Verification

Add a PlatformIO example for STC8H1K08/SDCC that builds PWMA1 plus PWMB6/7/8 together and checks `firmware.mem` and `firmware.map` for small-memory regressions. `tools/check_examples.sh` must build it and fail if disabled EC11 setter symbols appear in a small-memory map.
