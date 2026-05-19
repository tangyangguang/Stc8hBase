# EC11 Small ISR API Design

## Goal

Provide an explicit ISR-safe way to scan an EC11 encoder from a Timer ISR under SDCC mcs51 small model, without asking application projects to copy EC11 decoding logic or protect broad main-loop code with disabled interrupts.

## Evidence

SDCC mcs51 small model uses statically allocated internal RAM for ordinary function parameters and local variables, and the linker overlays compatible lifetimes. A Timer ISR can interrupt a main-loop call chain while those overlay slots hold live values. The ToyRemote build showed `__divuint_PARM_2` / `__moduint_PARM_2` sharing OSEG addresses with `drv_ec11_transition()` parameters, which explains TM1637 dynamic display corruption when the ISR calls the ordinary EC11 small scanner.

Local SDCC experiments showed that adding `__reentrant` to the existing small scanner is not enough when the API accepts an unqualified object pointer. SDCC emits generic pointer helpers such as `__gptrget` and `__gptrput`; those helpers still use ordinary parameter storage and can collide with interrupted main-loop code.

## Design

Keep `drv_ec11_scan_delta_small()` as the ordinary polling/main-loop API. It remains small and portable, but documentation must state that it is not ISR-safe on SDCC mcs51 small model.

Add an optional ISR-specific small API guarded by `DRV_EC11_ENABLE_SMALL_ISR_API`, default `0`:

```c
void drv_ec11_small_init_isr(STC8H_DATA drv_ec11_small_t *ec11);
stc8h_s8 drv_ec11_scan_delta_small_isr(STC8H_DATA drv_ec11_small_t *ec11,
                                       stc8h_u8 a_level,
                                       stc8h_u8 b_level);
```

On SDCC, both functions use `__reentrant`. The state pointer is `STC8H_DATA` so SDCC generates direct internal-RAM accesses instead of generic pointer helper calls. The scanner does not call `drv_ec11_transition()`; it performs the 16-entry transition table lookup inline so the ISR path has no nested ordinary helper. On Keil and hosted builds, the compiler abstraction maps the reentrant marker away or to the compiler equivalent.

`drv_ec11_small_t` stays unchanged. Applications that need ISR scanning must place the object in internal data memory:

```c
static STC8H_DATA drv_ec11_small_t encoder;
```

The Timer ISR may call `drv_ec11_scan_delta_small_isr()` and accumulate its return value into an application-owned `volatile` counter. Reading and clearing that counter from the main loop remains an application responsibility because the foundation library does not own the scheduler or event queue.

## Tradeoffs

Conditional `__reentrant` on the existing API was rejected because it still leaves generic pointer helpers on the path unless the public pointer type changes. Changing the existing API to `STC8H_DATA` would break useful hosted and generic usage and would make the ordinary API less flexible.

Duplicating the small scanner body in an ISR-specific function is deliberate. It costs a small amount of ROM only when enabled, but it keeps the ISR path independent from SDCC overlay and generic pointer helpers. This is preferable to broad interrupt masking in application code and keeps EC11 decoding inside the reusable driver.

## Verification

Add a host behavior test for the ISR API using the same EC11 sequences as the ordinary small API tests.

Add an SDCC codegen guard that compiles an ISR-only small EC11 translation unit and fails if its generated map/RST contains `__gptrget`, `__gptrput`, or `_drv_ec11_transition_PARM_2`. The guard also checks that the generated map has no OSEG contribution for the ISR-only path.

Run:

```sh
sh tools/check_host_tests.sh
sh tools/check_examples.sh
```
