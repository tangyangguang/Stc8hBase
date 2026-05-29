# SDCC Address Space Fast Paths Design

## Goal

Reduce SDCC/MCS51 ROM overhead from generic pointer helpers in the fixed-path RF, display, and PWM code used by small STC8H applications, while keeping the default generic APIs available for existing users.

## Evidence

- SDCC's MCS51 documentation states that unqualified pointers are 3-byte generic pointers and generic pointer dereference emits support routines; explicit address-space pointers generate more efficient code.
- Current minimal SDCC builds still emit generic pointer helpers in the optimized fixed paths:
  - `proto_rf_link_send_data_fixed()` emits `__gptrget` / `__gptrput` because `link`, `packet`, and `data` are unqualified.
  - `proto_rf_link_poll_data_fixed()` has the same issue for packet and payload copies.
  - `proto_rf_link_init()` also writes through an unqualified `link` pointer and is always emitted today, so a wrapper can keep generic pointer helpers even if all other generic protocol APIs are disabled.
  - `drv_nrf24l01_write_payload_fixed()`, `drv_nrf24l01_read_payload_fixed()`, and `drv_nrf24l01_config_pipe0_fixed()` still route through generic buffer helpers.
  - `drv_tm1637_display_raw4()` still reads a generic `segments` pointer.
  - `stc8h_pwm_write16()` writes through unqualified volatile pointers even though all callers pass XFR/SFRX addresses.

## Design Choice

Use explicit address-space-specific APIs instead of changing existing function signatures behind macros.

The default APIs stay source-compatible and remain useful for broad reuse. Small SDCC/8051 applications opt into named fast paths whose signatures show the address-space contract at the call site. This avoids hidden ABI changes across translation units and makes codegen tests straightforward.

Rejected alternatives:

- Macro-switching the same function name between generic and address-specific signatures. This saves call-site edits but creates hidden type/ABI changes and is fragile when different files use different macro sets.
- Header-only macro fast paths. They can be smaller in isolated cases, but they scatter implementation details into applications and make hosted behavior tests harder.

## API Additions

### Common Address-Space Types

Do not add new pointer typedefs. Use the existing `STC8H_XDATA`, `STC8H_DATA`, and `STC8H_CODE` macros directly in public signatures and private helpers. This keeps the address-space contract visible and avoids another naming layer.

Hosted builds map these qualifiers to ordinary C types, so tests can call the same APIs without compiler extensions.

### `proto_rf_link`

Add disabled-by-default fixed-path APIs:

```c
void proto_rf_link_init_xdata(STC8H_XDATA proto_rf_link_t *link);

#if PROTO_RF_LINK_ENABLE_SET_IDS
void proto_rf_link_set_ids_xdata(STC8H_XDATA proto_rf_link_t *link,
                                 stc8h_u8 local_id,
                                 stc8h_u8 peer_id);
#endif

stc8h_status_t proto_rf_link_send_data_fixed_xdata(
    STC8H_XDATA proto_rf_link_t *link,
    STC8H_XDATA stc8h_u8 *packet,
    const STC8H_XDATA stc8h_u8 *data);

stc8h_status_t proto_rf_link_poll_data_fixed_xdata(
    STC8H_XDATA proto_rf_link_t *link,
    const STC8H_XDATA stc8h_u8 *packet,
    STC8H_XDATA stc8h_u8 *data);
```

The new APIs preserve the fixed packet format and the same optional tracking macros as the generic fixed APIs. They do not call the generic packet builder and must compile without `__gptrget` or `__gptrput` in a minimal SDCC fixed-path build.

Add `PROTO_RF_LINK_ENABLE_INIT`, defaulting to `1`, around the existing generic `proto_rf_link_init()`. Small XDATA-only wrappers can set it to `0` and use `proto_rf_link_init_xdata()` instead.

### `drv_nrf24l01`

Add disabled-by-default address-space-specific helpers:

```c
stc8h_u8 drv_nrf24l01_write_payload_fixed_xdata(const STC8H_XDATA stc8h_u8 *data);
stc8h_u8 drv_nrf24l01_read_payload_fixed_xdata(STC8H_XDATA stc8h_u8 *data);
stc8h_status_t drv_nrf24l01_config_pipe0_fixed_code(const STC8H_CODE stc8h_u8 *addr);
```

Internally, add static `read_buf_xdata`, `write_buf_xdata`, and `write_buf_code` helpers as needed. Generic raw buffer APIs remain unchanged.

`config_pipe0_fixed_code()` is intended for fixed radio addresses stored in flash/code space. XDATA payload helpers are intended for packet buffers and payload buffers held outside scarce internal RAM.

### `drv_tm1637`

Add a disabled-by-default raw4 API for display segment buffers in internal DATA:

```c
stc8h_status_t drv_tm1637_display_raw4_data(const STC8H_DATA stc8h_u8 segments[4]);
```

This mirrors the existing raw4 behavior but reads the four segment bytes through DATA-specific addressing. It should share the same byte-writing sequence and display-control behavior as the existing raw path.

### `stc8h_pwm`

Keep the public fixed-channel PWM API unchanged. Change the internal 16-bit XFR writer to accept XDATA volatile byte pointers:

```c
static void stc8h_pwm_write16(volatile STC8H_XDATA stc8h_u8 *high,
                              volatile STC8H_XDATA stc8h_u8 *low,
                              stc8h_u16 value);
```

All current callers pass `STC8H_SFRX()` addresses, so this is an internal correctness and codegen tightening rather than a public API expansion.

## Configuration

Each new address-space-specific public API is controlled by an explicit macro defaulting to `0`:

- `PROTO_RF_LINK_ENABLE_XDATA_FIXED_API`
- `DRV_NRF24L01_ENABLE_XDATA_PAYLOAD_API`
- `DRV_NRF24L01_ENABLE_CODE_ADDRESS_API`
- `DRV_TM1637_ENABLE_DISPLAY_RAW4_DATA`

The existing generic protocol initializer is controlled by `PROTO_RF_LINK_ENABLE_INIT`, defaulting to `1`.

Existing feature macros still decide whether the generic APIs are emitted. The new macros do not require the generic APIs to be enabled.

## Compatibility

Default builds remain source-compatible. The only default implementation change is the internal PWM pointer qualifier, which does not alter the public header.

Keil C51 compatibility is preserved through `STC8H_XDATA`, `STC8H_CODE`, and `STC8H_DATA`. Hosted tests continue to compile because these macros collapse to ordinary C qualifiers or no-ops.

## Verification

Add hosted tests for behavioral equivalence:

- `proto_rf_link_send_data_fixed_xdata()` creates the same 32-byte packet as the generic fixed fast path.
- `proto_rf_link_poll_data_fixed_xdata()` accepts and copies the same fixed DATA packet.
- nRF24 XDATA fixed payload helpers issue the same SPI commands and byte counts as the generic fixed helpers.
- nRF24 CODE pipe0 helper writes SETUP_AW, TX_ADDR, RX_ADDR_P0, and RX_PW_P0 with the expected bytes.
- TM1637 DATA raw4 emits the same command and data sequence as raw4.

Add SDCC codegen guards:

- Compile minimal `proto_rf_link` XDATA fixed sender/poller and fail if `__gptrget` or `__gptrput` appears.
- Compile minimal nRF24 XDATA/CODE fast path and fail if the fast-path functions emit generic pointer helpers.
- Compile minimal TM1637 DATA raw4 path and fail if it emits `__gptrget`.
- Compile PWM fixed-only path and fail if `stc8h_pwm_write16` emits generic pointer helpers.

Add at least one size guard comparing a generic fixed-path build to the new address-space-specific build. The guard should fail if the specialized build is not smaller in generated assembly/code size or if helper calls remain.

## Documentation

Update the resource policy and RF/nRF24/TM1637 usage notes to explain when to choose the address-space-specific APIs. The docs must state that these are not application-specific shortcuts; they are public fixed-path APIs for small SDCC/8051 builds where buffer placement is known.

## Out of Scope

- No application-project bypass around `proto_rf_link` or `drv_nrf24l01`.
- No change to RF packet format, nRF24 SPI command semantics, TM1637 command sequence, or PWM register addresses.
- No persistent data migration.
