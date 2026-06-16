# Explicit Chip Profile Migration

## Why this changed

The base library should no longer silently default to `STC8H1K08` when no chip is selected.

Every firmware must select exactly one chip profile. This prevents a project for one chip from compiling under another chip's register, ADC, EEPROM, UART, timer, and package assumptions.

## Who is affected

Existing projects are affected only if they include the base library without a board configuration or without defining `STC8H_CHIP_STC8H1K08`.

Existing projects that already include `board/stc8h1k08_tssop20_demo/board_config.h` and define `STC8H_CHIP_STC8H1K08 1` need only add the explicit negative chip macro when the new one-hot check is implemented.

## STC8H1K08 migration

Add an explicit board config or compile flags:

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

## STC8H8K64U-LQFP48 migration

Use a board config that selects:

```c
#define STC8H_CHIP_STC8H1K08 0
#define STC8H_CHIP_STC8H8K64U 1
```

Then set board-specific UART, ADC, GPIO, and EEPROM choices. Do not enable EEPROM APIs or destructive EEPROM tests until the actual ISP EEPROM allocation is confirmed.

## Expected breakage

A build that includes `stc8h_config.h` without selecting a chip should fail at compile time. This is intentional.

Fix the build by adding a board config and passing it through:

```sh
-DSTC8H_CONFIG_INCLUDE=\"board_config.h\"
```
