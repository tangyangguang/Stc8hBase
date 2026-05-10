# EEPROM/IAP 策略

## 1. 目标

EEPROM/IAP 涉及片内非易失数据，必须先明确地址边界，再写实现。

本项目不兼容老项目历史格式，不做旧数据迁移。

但示例和测试不能静默擦写未知区域。

## 2. 官方确认结果

`stc8h_eeprom` 实现已按 STC8H1K08 官方资料和 STC 官方 EEPROM/IAP 库核对：

- STC8H1K08 EEPROM/IAP 容量为 4KB。
- EEPROM/IAP 地址范围为 `0x0000..0x0FFF`。
- 擦除粒度为 512 字节扇区。
- 写入粒度按字节写入。
- IAP 寄存器为 `IAP_DATA=0xC2`、`IAP_ADDRH=0xC3`、`IAP_ADDRL=0xC4`、`IAP_CMD=0xC5`、`IAP_TRIG=0xC6`、`IAP_CONTR=0xC7`、`IAP_TPS=0xF5`。
- 命令值为读 `1`、写 `2`、擦除 `3`。
- 触发流程为写 `IAP_TRIG=0x5A` 后写 `IAP_TRIG=0xA5`。
- 官方库在触发窗口前保存并关闭全局中断，触发后执行两个 `NOP` 并恢复中断。
- 操作结束后关闭 IAP，清理命令和触发寄存器，并把地址寄存器置为 `0xFFFF`。

资料来源见 `docs/vendor/stc/` 和 `docs/17_OFFICIAL_STC_LIBRARY_REVIEW.md`。

## 3. 测试专用区域

第一版示例必须使用测试专用地址范围。

测试地址范围由应用项目显式定义：

```c
#define BOARD_EEPROM_TEST_ADDR
#define BOARD_EEPROM_TEST_SIZE
```

未定义测试地址范围时，`eeprom_rw` 示例不得编译通过。

`eeprom_rw` 的 `platformio.ini` 必须设置 `default_envs = STC8H1K08`，保证普通 `pio run` 和 `pio run -t upload` 只构建/上传禁用写擦的安全环境。

真实写擦测试必须显式选择 `STC8H1K08_write_test` 环境，例如 `pio run -e STC8H1K08_write_test -t upload`，并在烧录前确认测试扇区可以被擦除。示例默认测试地址使用最后一个 EEPROM 扇区 `0x0E00..0x0FFF`，应用项目不得把该示例地址视为通用参数区约定。

示例必须检查：

- `BOARD_EEPROM_TEST_ADDR` 是否按擦除粒度对齐。
- `BOARD_EEPROM_TEST_SIZE` 是否为擦除粒度整数倍，或写明实际擦除覆盖范围。
- 检查失败时拒绝执行写擦。

## 4. 示例保护规则

EEPROM/IAP 示例必须：

- 在源码顶部写明会擦写的地址范围。
- 只访问 `BOARD_EEPROM_TEST_ADDR` 到 `BOARD_EEPROM_TEST_ADDR + BOARD_EEPROM_TEST_SIZE`。
- 不扫描、不擦写、不格式化未知区域。
- 不假设应用参数区存在。
- 不做旧格式迁移。
- 默认环境不得执行写擦；写擦环境必须有显式启用宏。
- PlatformIO 示例必须设置 `default_envs`，避免写擦环境被普通命令隐式构建或上传。

## 5. 应用项目规则

应用项目如果使用 EEPROM/IAP 保存参数，必须自己定义参数区：

```c
#define BOARD_EEPROM_PARAM_ADDR
#define BOARD_EEPROM_PARAM_SIZE
```

基础库只提供读、写、擦除基础能力，不定义应用参数格式。

## 6. 中断和时序

EEPROM/IAP 写擦期间可能影响中断和时序。

第一版要求：

- 写擦函数文档说明是否需要临时关闭中断。
- 示例不在红外收发期间执行 EEPROM/IAP 写擦。
- 资源报告记录 EEPROM/IAP 示例是否使用中断保护。
- 触发 IAP 时只关闭极短触发窗口的全局中断；第一版不在写擦期间运行红外等严格时序任务。
