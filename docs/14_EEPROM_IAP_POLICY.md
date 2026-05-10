# EEPROM/IAP 策略

## 1. 目标

EEPROM/IAP 涉及片内非易失数据，必须先明确地址边界，再写实现。

本项目不兼容老项目历史格式，不做旧数据迁移。

但示例和测试不能静默擦写未知区域。

## 2. 实现前必须确认

实现 `stc8h_eeprom` 前必须从 STC8H1K08 官方资料确认：

- EEPROM/IAP 可用地址范围。
- 页大小或扇区大小。
- 擦除粒度。
- 写入粒度。
- IAP 使能、触发、等待和关闭流程。
- 写擦期间对中断和时序任务的影响。

确认结果记录到本文件或实现注释中。

## 3. 测试专用区域

第一版示例必须使用测试专用地址范围。

测试地址范围由应用项目显式定义：

```c
#define BOARD_EEPROM_TEST_ADDR
#define BOARD_EEPROM_TEST_SIZE
```

未定义测试地址范围时，`eeprom_rw` 示例不得编译通过。

实现前确认页/扇区大小后，示例必须检查：

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
