# STC8H8K64U OTA/IAP Bootloader 设计方案

## 1. 目标

本文档用于持续评审 `STC8H8K64U-45I-LQFP48` 专用 OTA 基础能力，并记录当前实现边界。

目标场景：

- ESP32 作为主控板，负责联网获取 STC 固件。
- ESP32 与 `STC8H8K64U` 通过 RS485 通信。
- ESP32 将新固件通过 RS485 分块传给 `STC8H8K64U`。
- `STC8H8K64U` 通过 IAP 擦写自身应用程序区，实现现场固件更新。

本方案只支持 `STC8H8K64U-45I-LQFP48`，不扩展为 STC8H 全系列通用 OTA 框架。

## 2. 资料依据

本方案依据以下资料和实践结论：

- STC 官方 `STC8H8K64U` 资料确认：芯片有最多 64KB Flash，EEPROM 大小可配置，擦除页为 512 字节，ISP 可更新应用代码。
- STC8H 官方手册附录确认：`STC8H8K64U` 属于可自定义 EEPROM 大小的 IAP 系列，用户可开发自己的 ISP 程序；若要更新程序空间，需要在 ISP 下载时把可 IAP 改写区域规划为覆盖程序空间。
- STC 官方示例以 `STC8H8K64U` 展示了用户程序区、用户 ISP 区、复位入口映射区和用户参数区的分区思想。
- `stcgal`、`stc8prog` 等开源工具证明 STC 内置 BSL/ISP 可通过 UART/USB 包协议烧写 code memory 和 IAP/EEPROM，但 STC ISP 协议并非完整公开文档化，不适合作为产品 OTA 的唯一基础。
- NXP 二级 bootloader 应用笔记和 MCUboot/Memfault DFU 资料给出通用最佳实践：bootloader 与应用分离，使用 metadata 描述镜像，升级状态必须可恢复，bootloader 区不能被普通应用升级擦写。

来源记录见 `docs/10_REFERENCES.md`。

## 3. 设计结论

推荐方案：

```text
ESP32 云端下载/验签/暂存完整固件
    -> RS485 分块传输
        -> STC8H8K64U 专用 bootloader 接收
            -> IAP 擦写应用区
                -> CRC 校验
                    -> commit 有效标志
                        -> 重启运行新应用
```

不推荐让 ESP32 模拟 STC-ISP 完成产品 OTA。原因：

- 官方 ISP/BSL 更适合生产烧录和开发下载，不适合产品协议、RS485 方向控制和现场恢复状态机。
- STC ISP 协议与工具链耦合较强，公开资料不足。
- 产品 OTA 需要明确的版本、长度、CRC、commit 和失败恢复机制，这些更适合自定义 bootloader。

## 4. 范围

第一版必须支持：

- 仅支持 `STC8H8K64U-45I-LQFP48`。
- 仅支持单应用区 OTA。
- ESP32 必须能暂存完整 STC 固件包。
- STC 侧使用 IAP 擦写应用区。
- STC 侧做分帧 CRC16 和整包 CRC32。
- ESP32 侧做固件签名或 hash 校验。
- RS485 作为第一版参考传输链路。
- OTA 核心按 payload 级 API 设计，不绑定 RS485、Modbus、433 串口透传或业务协议。
- 升级失败后 bootloader 可重新接收完整固件。

第一版明确不支持：

- 不支持 `STC8H1K08` 程序 OTA。
- 不支持 STC8H 全系列自动适配。
- 不支持 STC 内部 A/B 双应用区。
- 不支持 STC 侧 HTTPS、证书、JSON、压缩或复杂签名验签。
- 不支持 bootloader 自升级。
- 不支持单应用区上的真正 rollback；第一版只提供 recovery/retry。
- 不支持普通应用在未编译 OTA 模块时产生任何 ROM/RAM/外设占用。

## 5. 能力边界

基础库只提供 STC 侧可复用基础能力和最小参考实现，不承接完整产品 OTA 平台。

### 5.1 基础库负责

- `STC8H8K64U` IAP 擦页、写入、读回、错误码。
- 软件复位和进入 bootloader/ISP 相关的最小封装。
- boot stub、应用链接基址和中断跳转表的最小约定。
- boot metadata 结构定义：应用地址、长度、版本、CRC、状态和有效标志。
- manifest、frame 和 boot metadata 的固定字节序编解码。
- 地址范围检查，禁止擦写 bootloader、boot stub 和参数保留区。
- 分块写入状态机。
- CRC16、CRC32 小工具。
- RS485 bootloader 最小示例和验证文档。
- 进入 OTA/bootloader 前的板级安全关闭钩子声明和调用点。

### 5.2 ESP32 或应用项目负责

- 云端版本查询。
- 固件下载和完整固件暂存。
- 签名、hash、产品版本策略和灰度策略。
- RS485 从机地址分配和总线仲裁。
- 控制 STC 进入升级模式。
- 超时重试、断点恢复和升级结果上报。
- 业务升级窗口选择。
- 具体业务输出关闭实现，例如电磁阀、水泵、继电器、PWM 或电机输出。

## 6. Flash/IAP 规划

前置要求：生产烧录 bootloader 时必须确认 `STC8H8K64U` 的 IAP/EEPROM 规划允许 IAP 改写应用程序区。若仍使用类似 `0.5KB EEPROM` 的配置，只能写参数区，不能实现应用 OTA。

建议逻辑分区：

| 区域 | 建议范围 | 用途 |
|---|---:|---|
| Boot stub / 向量区 | `0x0000..0x01FF` | 复位入口、中断跳转表、进入 bootloader 或 app |
| Application 区 | `0x0200..0xB7FF` | 可 OTA 更新的业务固件 |
| Bootloader 区 | `0xB800..0xFBFF` | RS485 升级协议、IAP 写入、CRC、状态机 |
| Boot 参数区 | `0xFC00..0xFFFF` | 双份升级状态、版本、长度、CRC、有效标志、失败原因 |

以上地址为当前已编译验证的实现基线。最初评审曾尝试保留应用区到 `0xEFFF`，但带 RS485 帧、CRC32、IAP、双参数记录和状态回包的 bootloader 不能可靠放入 `0xF000..0xFBFF`。当前 `h8k64u_rs485_ota_bootloader` 通过 `--code-loc 0xB800` 链接，`tools/check_examples.sh` 会检查 reset stub 在 `0x0000`、bootloader `s_HOME` 在 `0xB800`，并禁止代码符号进入 `0xFC00..0xFFFF` 参数区。

应用镜像大小上限相应调整为 `0xB600` 字节，即 `0x0200..0xB7FF`。应用项目生成 OTA manifest 时必须使用相同的 `app_base/app_size` 边界；超过该范围的镜像应在 ESP32 侧拒绝下发，并会被 STC 侧 manifest/IAP 边界检查拒绝。

## 7. 启动流程

上电后固定进入 boot stub，再进入 bootloader 决策：

```text
上电/复位
  -> boot stub
  -> bootloader 检查参数区
      -> update_pending=1：停留 bootloader，等待 ESP32
      -> app_valid=1 且 app_crc 匹配：跳转应用
      -> state=BOOT_COMMITTED 且 boot_attempted=0 且 app_crc 匹配：记录 boot_attempted=1 后试启动应用
      -> 其他情况：停留 bootloader，等待 ESP32 恢复升级
```

应用要请求 OTA 时，通过业务协议收到 ESP32 升级命令后写 `update_pending=1`，然后软件复位。复位后 bootloader 不再跳转应用。

## 8. 应用链接和中断策略

应用从 `0x0200` 开始链接，不占用低地址 `0x0000..0x01FF`。这不是建议项，而是 OTA bootloader 可工作的硬约束；默认从 `0x0000` 链接的应用不能作为 OTA 镜像使用。

boot stub 固定持有复位和中断向量。中断处理有两种可选实现：

- 第一版最小验证：示例应用尽量不依赖中断，只验证启动、版本输出和主循环行为。
- 正式应用支持：boot stub 的固定中断向量跳转到应用中断跳转表，应用构建必须生成固定格式的跳转表。

第一版必须先实现和验证以下启动链路：

- boot stub 固定编译到 `0x0000..0x01FF`。
- boot stub 复位入口只做最小跳转，进入高地址 bootloader 决策函数。
- bootloader 判断可以启动应用时，跳转到 `STC8H_OTA_APP_BASE`。
- OTA 应用示例必须通过 SDCC/PlatformIO 明确从 `0x0200` 链接。
- Keil C51 必须记录等价链接配置，即使第一阶段只能做 Windows 环境人工验证。
- 示例应用若使用中断，必须通过应用中断跳转表进入，不得重新占用低地址硬向量。

第一版验收时，如果不能证明应用镜像确实从 `0x0200` 链接并能被 bootloader 跳转启动，则不得宣称 bootloader OTA 已完成。

## 9. RS485 升级协议

OTA 核心 API 必须传输无关。RS485 只作为第一版示例传输，基础库不实现灌溉业务协议、不实现 Modbus，也不假设帧来自某一个 UART。应用项目可以把 RS485、普通 UART 或 433 串口透传收到的 payload 喂给 OTA API。

### 9.1 当前基础库相关能力

当前基础库已有能力：

- `STC8H8K64U-45I-LQFP48` 显式芯片 profile。
- UART1、UART2、UART3 轮询初始化、发送、接收。
- H8K64U 板级配置默认定义 `BOARD_RS485_UART = STC8H_UART2`、`BOARD_RF433_UART = STC8H_UART3`。
- UART2 默认引脚组为 P1.0/P1.1，UART3 默认引脚组为 P0.0/P0.1，均可通过板级宏调整。
- `h8k64u_uart2_hello`、`h8k64u_uart3_hello` 已覆盖 H8K64U UART2/UART3 最小发送验证。
- `drv_nrf24l01` 和 `proto_rf_link` 已覆盖 nRF24L01/2.4GHz 小包射频链路，但它们不是 433MHz 串口透传模块能力，也不是 OTA 专用协议。

当前已实现：

- `drivers/drv_rs485_uart.*` 提供薄半双工 UART 方向控制 wrapper，通过 `BOARD_RS485_TX_ENABLE()` / `BOARD_RS485_RX_ENABLE()` 由板级控制 DE/RE。
- `protocols/proto_ota_frame.*` 提供传输无关 OTA 帧 build/parse、CRC16、地址过滤、重复序号上报和 STATUS 回包命令。
- `examples/platformio/h8k64u_rs485_ota_bootloader` 接入 UART2/RS485、应用区 IAP、参数区 IAP、OTA 状态机和高地址 bootloader 布局。
- `examples/platformio/h8k64u_uart3_ota_passthrough` 使用 UART3 验证 433 透明串口链路可承载同一 OTA frame，并返回 STATUS 帧。
- UART 当前仍是轮询 API，OTA bootloader 第一版继续使用轮询，避免中断和应用向量表耦合。

当前缺口：

- 还没有真实 433 模块空口丢包、延时和重试参数验证。
- 还没有真实硬件上的 RS485 方向时序、IAP 写入和断电恢复验证。

### 9.2 传输适配层边界

OTA 传输适配层只负责把外部串口类链路上的字节流转换为 OTA payload 操作。它不能理解业务命令，也不能直接擦写 Flash。

建议传输适配层能力：

- 从链路接收 OTA 帧。
- 校验帧头、长度、序号和 CRC16。
- 提取 `BEGIN`、`WRITE_BLOCK`、`VERIFY`、`COMMIT` 等 OTA payload。
- 调用 `stc8h_ota_begin()`、`stc8h_ota_write_chunk()` 等 OTA 核心 API。
- 发送 ACK、NAK、状态和错误码。
- 对半双工链路控制 TX/RX 方向。

传输适配层明确不做：

- 不下载云端固件。
- 不做签名验签。
- 不解释灌溉业务命令。
- 不实现 Modbus 业务寄存器表。
- 不私有实现 IAP 写入。
- 不假设底层一定是 RS485；UART 透传和 433 串口透传必须复用同一套 payload 帧和 OTA 核心。

### 9.3 RS485 第一阶段方案

RS485 是第一版参考链路。建议新增一个很薄的半双工 UART/RS485 适配能力，而不是把 RS485 逻辑写进 OTA 核心。

RS485 适配层应支持：

- 编译期选择 UART：默认使用 `BOARD_RS485_UART`。
- 板级宏控制 DE/RE：
  - `BOARD_RS485_TX_ENABLE()`
  - `BOARD_RS485_RX_ENABLE()`
- 发送前切 TX，发送完成后切 RX。
- 方向切换前后允许板级短延时，避免收发器尾字节被截断。
- 可选从机地址字段，支持一主多从。
- 不占用 Timer、中断或隐藏全局缓冲。

第一版 RS485 OTA bootloader 使用 `BOARD_RS485_UART`，当前 H8K64U 基板默认是 UART2。示例中的方向脚仅用于构建参考，真实项目必须在自己的 `board_pins.h` 中绑定实际 DE/RE 引脚，并实测发送完成后的方向切回时序。

RS485 示例必须覆盖：

- 单从机升级。
- 错地址帧忽略。
- CRC 错误 NAK。
- 半双工方向切换。
- 连续 chunk 写入。
- 超时后仍能继续接收新 `BEGIN`。

### 9.4 433MHz 串口透传支持范围

433MHz 只支持透明串口类模块，即模块对 STC 暴露 UART RX/TX，基础库把它视为串口透传链路。

明确不支持：

- SPI/寄存器型 433 射频芯片。
- 433 频道、空中速率、发射功率、前导码、同步字、FEC 等射频参数配置。
- 433 模块私有 AT 命令配置。
- 433 自组网、绑定、路由或频道扫描。

支持范围：

- 默认使用 `BOARD_RF433_UART`，当前 H8K64U 板级配置为 UART3。
- 复用与 RS485 相同的 OTA 帧格式和 OTA 核心 API。
- 不需要 DE/RE 控制。
- `h8k64u_uart3_ota_passthrough` 只验证 UART3 透明串口收发 OTA frame 和 STATUS 回包，不执行 IAP 写擦。
- 因无线链路丢包更高，建议更小 chunk、更长超时、更保守重试。
- 433 串口透传不复用 `proto_rf_link`；它只复用 OTA payload 帧。

如果后续项目使用 SPI/寄存器型 433 芯片，该需求不属于本 OTA 基础能力范围，也不作为当前路线的后续计划。

RS485 使用主从模式。ESP32 是主站，STC 是从站。STC 只响应目标地址匹配的帧，不主动发起升级。

建议帧格式：

```text
SOF | proto_ver | dst_addr | src_addr | cmd | seq | offset | len | payload | crc16
```

字段：

| 字段 | 说明 |
|---|---|
| `SOF` | 固定帧头，用于同步 |
| `proto_ver` | 协议版本 |
| `dst_addr` | STC 从机地址 |
| `src_addr` | ESP32 主站地址 |
| `cmd` | 命令 |
| `seq` | 包序号，防重复包和乱序 |
| `offset` | 相对 Application 区的偏移 |
| `len` | payload 长度 |
| `payload` | 命令数据 |
| `crc16` | 单帧校验 |

第一版 payload 建议为 64 或 128 字节。STC 侧不得依赖大缓冲。

第一版 chunk 策略：

- 只支持顺序写入。
- `offset` 必须等于当前 `write_offset`，否则拒绝。
- 允许重复发送上一块已接受 chunk；重复 chunk 必须满足 `offset`、`len` 和 flash 读回数据全部一致，才返回 ACK。
- 小于 `write_offset` 但不是上一块完整重复的 chunk 一律 NAK。
- 大于 `write_offset` 的未来 chunk 一律 NAK。
- 不支持乱序写入。
- 断点恢复由 ESP32 通过 `READ_STATUS` 获取 `write_offset` 后继续发送。

最小命令集：

| 命令 | 方向 | 作用 |
|---|---|---|
| `PING` | ESP32 -> STC | 查询 bootloader 版本、芯片、状态 |
| `ENTER_UPDATE` | ESP32 -> STC | 请求进入升级模式 |
| `BEGIN` | ESP32 -> STC | 下发固件大小、版本、CRC32、目标芯片 |
| `ERASE_RANGE` | ESP32 -> STC | 擦除应用区指定范围 |
| `WRITE_BLOCK` | ESP32 -> STC | 写入一块固件数据 |
| `READ_STATUS` | ESP32 -> STC | 查询状态、offset、错误码 |
| `VERIFY` | ESP32 -> STC | 触发 STC 计算应用区 CRC32 |
| `COMMIT` | ESP32 -> STC | CRC 通过后提交为待试启动应用 |
| `REBOOT` | ESP32 -> STC | 重启运行新应用 |
| `ABORT` | ESP32 -> STC | 取消升级，保持 bootloader 可恢复 |

## 10. 固件包格式

ESP32 下载的固件包应包含 manifest，不应只传裸二进制。

STC 侧 manifest 字段分为强制字段和可裁剪字段。强制字段必须出现在第一版实现中。manifest 在线上传输和持久化时必须使用固定字节序，不允许直接对 C struct 内存做 CRC 或作为协议格式。

第一版统一采用 little-endian 字节序。多字节字段按低字节在前编码，字段顺序必须由协议文档固定。`manifest_crc` 计算范围是从 `magic` 到 `flags` 的规范化字节序内容，不包含 `manifest_crc` 字段本身。

强制字段：

```text
magic
format_version
target_chip = STC8H8K64U
board_id
hw_revision
app_id
app_base = 0x0200
app_size
app_crc32
version_major
version_minor
version_patch
min_bootloader_version
flags
manifest_crc
```

可裁剪字段：

```text
build_id
release_channel
compat_flags
image_hash
signature
```

ESP32 必须先校验 `signature` 或 hash。STC 侧只检查：

- `magic`
- `target_chip`
- `board_id`
- `hw_revision`
- `app_id`
- `app_base`
- `app_size`
- `app_crc32`
- `min_bootloader_version`
- `manifest_crc`

`image_hash` 和 `signature` 默认由 ESP32 校验。第一版不要求 STC 侧计算 hash 或验证签名。

## 11. Boot 参数区

参数区必须至少保存：

| 字段 | 作用 |
|---|---|
| `param_magic` | 参数记录有效标志 |
| `param_version` | 参数结构版本 |
| `sequence` | 双份参数的新旧判断 |
| `state` | 当前升级状态 |
| `app_valid` | 应用是否有效 |
| `update_pending` | 是否请求进入升级 |
| `boot_attempted` | commit 后是否已试启动新应用 |
| `app_base` | 应用起始地址 |
| `app_size` | 应用长度 |
| `app_crc32` | 应用整体 CRC |
| `version` | 应用版本 |
| `write_offset` | 已写入进度 |
| `fail_reason` | 最近失败原因 |
| `param_crc` | 参数自身 CRC |

参数区必须采用双份或等价的掉电保护结构。写新状态时先写新记录，校验通过后用 `sequence` 选择最新有效记录。不得只有单份状态记录。

第一版固定参数区布局：

| 记录 | 范围 | 说明 |
|---|---:|---|
| Param A | `0xFC00..0xFDFF` | 512 字节参数记录 A |
| Param B | `0xFE00..0xFFFF` | 512 字节参数记录 B |

写入规则：

- 每条记录单独占用一个 512 字节擦除页。
- 启动时读取 A/B，CRC 正确且 `param_magic`/`param_version` 匹配的记录才有效。
- A/B 都有效时选择 `sequence` 更新的一条；`sequence` 相同视为参数损坏，进入 recovery。
- 写新状态时选择非当前记录所在页，先擦除，再写完整规范化字节序记录，再读回校验。
- 如果写新记录期间断电，旧记录仍可被选择。
- `param_crc` 计算规范化字节序内容，不对 C struct 裸内存计算。
- 参数区写入失败不得跳转应用。

## 12. Bootloader 状态机

STC bootloader 状态：

```text
BOOT_IDLE
BOOT_WAIT_BEGIN
BOOT_ERASING
BOOT_RECEIVING
BOOT_VERIFYING
BOOT_COMMITTED
BOOT_ERROR
```

状态转换：

```text
BOOT_IDLE
  -> BOOT_WAIT_BEGIN        收到 ENTER_UPDATE 或参数区已有 update_pending
  -> 应用跳转               app_valid=1 且无 update_pending
  -> 试启动应用             state=BOOT_COMMITTED 且 boot_attempted=0 且 CRC 匹配

BOOT_WAIT_BEGIN
  -> BOOT_ERASING           BEGIN 校验通过
  -> BOOT_ERROR             固件目标、大小、版本不合法

BOOT_ERASING
  -> BOOT_RECEIVING         应用区擦除完成
  -> BOOT_ERROR             擦除失败或地址越界

BOOT_RECEIVING
  -> BOOT_VERIFYING         所有块写入完成
  -> BOOT_ERROR             写入失败、CRC16 错误、offset 越界

BOOT_VERIFYING
  -> BOOT_COMMITTED         CRC32 匹配
  -> BOOT_ERROR             CRC32 不匹配

BOOT_COMMITTED
  -> REBOOT                 写 trial 状态成功后复位
  -> BOOT_WAIT_BEGIN        trial 已尝试但 app_valid 仍为 0

BOOT_ERROR
  -> BOOT_WAIT_BEGIN        ESP32 重新 BEGIN
```

`COMMIT` 之前不得设置 `app_valid=1`。

`COMMIT` 也不得直接设置 `app_valid=1`。第一版采用 trial boot 语义：

- `COMMIT` 只写入 `state=BOOT_COMMITTED`、`app_valid=0`、`boot_attempted=0`、`update_pending=0`。
- bootloader 首次看到该状态且 CRC32 匹配时，先把 `boot_attempted=1` 持久化，再跳转应用。
- 新应用完成早期自检和输出安全初始化后，调用应用有效标记能力，把 `app_valid=1` 写入参数区。
- 若试启动后复位且 `app_valid` 仍为 0，bootloader 不再继续跳转该应用，进入 recovery 等待 ESP32 重新升级。

当前 `h8k64u_ota_min_app` 默认构建只保留 mark-valid 调用点，不写参数区；`STC8H8K64U_mark_valid_iap` 构建环境会编译真实 `hal/stc8h_ota_params_store + hal/stc8h_iap_ota_params` 接入路径。真实运行该环境会写擦 `0xFC00/0xFE00` OTA 参数扇区，必须在硬件验证阶段确认测试板可接受该操作后再烧录。

应用启动健康确认：

- `COMMIT` 后 bootloader 只允许一次试启动新应用，新应用必须在完成早期自检和输出安全初始化后调用应用有效标记能力。
- 若新应用未在约定条件下标记有效，下一次复位时 bootloader 应进入 recovery 等待 ESP32 重新升级。
- 第一版不恢复旧应用，因为单应用区已覆盖旧固件；这里的恢复语义是重新进入 bootloader 接收固件。

## 13. 失败恢复

必须覆盖：

| 场景 | 行为 |
|---|---|
| 传输中断 | ESP32 通过 `READ_STATUS` 查询进度后重发 |
| 单帧 CRC 错 | STC 拒绝该帧，ESP32 重发 |
| 擦除后断电 | 下次上电发现 `app_valid=0`，停留 bootloader |
| 写入中断电 | 下次上电停留 bootloader，ESP32 重新升级 |
| 写完但 CRC32 错 | 不提交，保持 bootloader |
| COMMIT 前断电 | 不认为应用有效，保持 bootloader |
| COMMIT 后、试启动前断电 | 下次继续按 trial 状态试启动一次 |
| COMMIT 后应用启动失败 | 应用未完成健康确认时，下次 bootloader 进入 recovery 等待 |
| 低电压或电源不稳 | 拒绝擦写，返回错误 |

由于第一版不做内部 A/B，升级期间旧应用不可用。恢复依赖 bootloader 永久保留和 ESP32 持有完整固件包。文档和 API 中不得把该能力描述为 rollback，应描述为 recovery 或 retry。

## 14. 安全策略

第一版安全分工：

- ESP32 做签名验签、hash、版本策略和云端认证。
- STC 做目标芯片、地址范围、长度、CRC32 和 bootloader 版本检查。
- STC 不做公钥签名验签。

原因：

- `STC8H8K64U` 资源有限，复杂验签会显著增加 ROM/RAM 和验证成本。
- ESP32 已具备联网和更充足资源，适合承接安全策略。
- STC 侧必须保持 bootloader 小、稳定、可审计。

## 15. 硬件要求

建议硬件设计满足：

- ESP32 能控制 STC reset，或至少能通过业务协议让 STC 软件复位。
- RS485 收发器 DE/RE 方向可由固件明确控制。
- 多从机总线必须有唯一地址。
- 升级期间普通业务通信暂停。
- 供电检测由 ESP32 或 STC 侧低电压检测承担；电源不稳时不得 IAP 写擦。
- 下载串口/USB ISP 仍应保留为生产和救援通道。

## 16. 候选最小 API

候选 API 只表达能力边界，最终命名和参数类型在实现计划中确认。API 必须保持小、稳定、可裁剪。

```c
stc8h_ota_init()
stc8h_ota_should_enter_bootloader()
stc8h_ota_get_boot_action()
stc8h_ota_manifest_decode()
stc8h_ota_begin(manifest)
stc8h_ota_write_chunk(offset, data, len)
stc8h_ota_verify()
stc8h_ota_commit()
stc8h_ota_abort()
stc8h_ota_get_status()
stc8h_ota_mark_boot_attempted()
stc8h_ota_mark_app_valid()
```

API 边界：

- `stc8h_ota_write_chunk()` 接收 payload，不关心 payload 来自 RS485、普通 UART 还是 433 串口透传。
- `stc8h_ota_manifest_decode()` 把规范化 manifest 字节流解码为内部结构，并验证 `manifest_crc`。
- `stc8h_ota_begin()` 只接受已由上层完成传输帧校验和 manifest 解码后的 manifest。
- `stc8h_ota_commit()` 只在整包 CRC32 通过后成功。
- `stc8h_ota_abort()` 清理接收状态，但不得擦写 bootloader、boot stub 或参数保留区。
- `stc8h_ota_get_boot_action()` 返回留在 bootloader、正常跳转应用或 trial 跳转应用三类启动决策。
- `stc8h_ota_mark_boot_attempted()` 只能在 trial 跳转应用前调用，用于持久化 `boot_attempted=1`。
- `stc8h_ota_mark_app_valid()` 由新应用在早期健康确认后调用。
- 未启用 OTA 构建时，上述 API 不声明、不编译。

## 17. 业务安全输出边界

OTA 期间从站必须拒绝业务运行命令，并保证输出处于安全状态。基础库只定义机制，不包含灌溉业务逻辑。

基础库提供：

- bootloader 进入 OTA 前调用板级安全关闭钩子的固定时机。
- OTA 状态查询能力，应用可据此拒绝业务命令。
- 示例中演示如何在进入 bootloader 前关闭 GPIO/PWM 输出。

业务项目负责：

- 定义哪些输出需要关闭。
- 实现板级安全关闭函数。
- 在 ESP32 主控侧暂停对目标从站的普通控制命令。
- 在新应用启动早期先初始化输出到安全态，再标记 app valid。

## 18. 基础库落点

建议后续实现时拆为：

| 位置 | 内容 |
|---|---|
| `hal/` | `STC8H8K64U` IAP 程序区擦写原语 |
| `hal/` | OTA 参数区双记录读写原语 |
| `core/` 或 `hal/` | boot stub、跳转入口和应用基址约定 |
| `utils/` | CRC16、CRC32 |
| `protocols/` | manifest/frame/参数规范化编解码和分块传输状态机，不直接绑定具体 UART |
| `protocols/` 或 `drivers/` | RS485/半双工 UART 传输适配，边界只到 payload 帧 |
| `examples/` | `h8k64u_rs485_bootloader` 最小示例 |
| `examples/` | `h8k64u_uart_passthrough_ota` 或 `h8k64u_433_uart_ota` 后续串口透传示例 |
| `docs/` | 本设计、协议、验证计划、硬件记录 |

模块必须遵守基础库原则：

- 未编译时零占用。
- 不隐藏初始化外设。
- 不自动占用 UART、Timer、中断或全局缓冲。
- 地址、分区、从机地址和引脚必须编译期显式配置。
- 公共 API 不暴露 Keil/SDCC 专属类型。

## 19. 验收计划

第一阶段：PC 模拟 ESP32。

- 能烧入 bootloader。
- boot stub、bootloader 和应用镜像地址分区正确。
- 应用镜像从 `0x0200` 链接并能被 bootloader 跳转启动。
- 能通过串口/RS485 写入最小 app。
- app 能启动并输出版本。
- 擦除后断电能停留 bootloader。
- 写入中断后能重新升级。
- CRC 错误不会 commit。
- app 超过分区会拒绝。
- 目标芯片不匹配会拒绝。
- `board_id`、`hw_revision` 或 `app_id` 不匹配会拒绝。
- chunk 丢失、重复、乱序均能检测或按设计处理。
- bootloader 区不会被擦写。
- 参数区 A/B 单页掉电损坏时能选择另一条有效记录。
- OTA 关闭编译时不引入 OTA API、CRC、IAP 程序区写入符号或全局缓冲。
- RS485 DE/RE 方向切换不会截断最后一个字节。
- UART3 透明串口链路能复用同一 OTA payload API。

第二阶段：ESP32 集成。

- ESP32 下载并暂存固件。
- ESP32 验签或 hash。
- RS485 分块发送。
- 超时重试。
- STC 验证 CRC32。
- COMMIT 后 STC 试启动新版本，新应用自检通过后再标记有效。
- ESP32 读取新版本并上报结果。

第三阶段：硬件压力。

- 连续升级不少于 100 次。
- 升级中复位。
- 升级中断电。
- RS485 干扰下重试。
- 低电压拒绝升级。
- 不同固件大小边界测试。
- 参数区双份记录掉电测试。
- 433 透明串口模块接入前，先用有线 UART3 透传模拟完成同一 OTA 流程。

## 20. 应用方需求映射

应用方提出的合理需求应吸收为基础库边界：

- 传输无关 OTA 核心。
- 上层喂入 manifest 和 chunk。
- 基础库负责状态机、镜像校验、IAP 写入、commit 和 recovery。
- OTA 期间业务输出保持安全态。
- 未启用 OTA 时零占用。

应用方提出但基础库不承诺的能力：

- 不承诺 STC8H 全系列通用 OTA。
- 不承诺单应用区 rollback 到旧应用。
- 不承诺 STC 侧签名验签、HTTPS、hash 或云端版本策略。
- 不承诺基础库实现 RS485/Modbus/灌溉业务协议。

应用项目必须调整自己的 OTA 设计：

- 把 ESP32 作为下载、验签、暂存、重试和上报主体。
- 把 STC 从站 OTA 视为 payload 级写入和 recovery 机制。
- 在业务协议中只负责把 OTA payload 可靠送到 STC OTA API。
- 在进入 OTA 前停止普通控制命令，并调用板级安全关闭逻辑。

## 21. 当前待确认问题

实现前仍需确认：

- ESP32 是否一定能暂存完整 STC 固件包。
- STC reset 线是否由 ESP32 控制。
- 现场 RS485 总线是一主一从还是一主多从。
- 应用是否需要中断；若需要，必须先确认中断跳转表方案。
- 应用最大可接受代码空间是多少，是否允许 bootloader 占用 4KB 到 8KB。
- 生产烧录工具是否能固定配置 IAP/EEPROM 覆盖程序空间。
- H8K64U 实物验证时允许使用哪一段应用区做首次破坏性擦写测试。

## 22. 评审结论

从专业角度看，`ESP32 暂存完整固件 + STC8H8K64U 专用 IAP bootloader + 单应用区可恢复升级` 是当前场景的推荐方案。

该方案的优势：

- STC 侧小而稳定。
- ESP32 承担复杂网络和安全逻辑。
- 失败后可通过永久 bootloader 恢复。
- 不强行在 64KB STC 内部做双镜像。
- 能作为基础库能力沉淀，但不会演变成通用 OTA 框架。

主要风险：

- IAP/EEPROM 生产配置错误会导致 OTA 不可用。
- 低地址向量和应用链接地址必须一次设计清楚。
- bootloader 体积必须严格控制。
- 单应用区方案升级失败时旧应用不可继续运行。
- 参数区必须做双份掉电保护。
- manifest、frame 和参数区必须使用规范化字节序，不能依赖 C struct 布局。
- 应用方如果需要真正 rollback，必须增加外部暂存区或改为双应用区，不能由当前单区方案隐式满足。

第一版应坚持范围收敛，先完成可恢复、可验证、可维护的最小 OTA 基础能力。
