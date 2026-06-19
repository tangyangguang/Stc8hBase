# STC8H8K64U OTA/IAP Bootloader 设计方案

## 1. 目标

本文档用于持续评审 `STC8H8K64U-45I-LQFP48` 专用 OTA 基础能力。当前阶段只定义方案、边界和验收标准，不进入代码实现。

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
- 升级失败后 bootloader 可重新接收完整固件。

第一版明确不支持：

- 不支持 `STC8H1K08` 程序 OTA。
- 不支持 STC8H 全系列自动适配。
- 不支持 STC 内部 A/B 双应用区。
- 不支持 STC 侧 HTTPS、证书、JSON、压缩或复杂签名验签。
- 不支持 bootloader 自升级。
- 不支持普通应用在未编译 OTA 模块时产生任何 ROM/RAM/外设占用。

## 5. 能力边界

基础库只提供 STC 侧可复用基础能力和最小参考实现，不承接完整产品 OTA 平台。

### 5.1 基础库负责

- `STC8H8K64U` IAP 擦页、写入、读回、错误码。
- 软件复位和进入 bootloader/ISP 相关的最小封装。
- boot metadata 结构定义：应用地址、长度、版本、CRC、状态和有效标志。
- 地址范围检查，禁止擦写 bootloader、boot stub 和参数保留区。
- 分块写入状态机。
- CRC16、CRC32 小工具。
- RS485 bootloader 最小示例和验证文档。

### 5.2 ESP32 或应用项目负责

- 云端版本查询。
- 固件下载和完整固件暂存。
- 签名、hash、产品版本策略和灰度策略。
- RS485 从机地址分配和总线仲裁。
- 控制 STC 进入升级模式。
- 超时重试、断点恢复和升级结果上报。
- 业务升级窗口选择。

## 6. Flash/IAP 规划

前置要求：生产烧录 bootloader 时必须确认 `STC8H8K64U` 的 IAP/EEPROM 规划允许 IAP 改写应用程序区。若仍使用类似 `0.5KB EEPROM` 的配置，只能写参数区，不能实现应用 OTA。

建议逻辑分区：

| 区域 | 建议范围 | 用途 |
|---|---:|---|
| Boot stub / 向量区 | `0x0000..0x01FF` | 复位入口、中断跳转表、进入 bootloader 或 app |
| Application 区 | `0x0200..0xEFFF` | 可 OTA 更新的业务固件 |
| Bootloader 区 | `0xF000..0xFBFF` | RS485 升级协议、IAP 写入、CRC、状态机 |
| Boot 参数区 | `0xFC00..0xFFFF` | 双份升级状态、版本、长度、CRC、有效标志、失败原因 |

以上地址为第一版设计基线。实现前必须用实际 bootloader 链接结果确认 `Bootloader 区` 是否足够；若不足，优先扩大 bootloader 区并缩小应用区，不允许让 bootloader 溢出到参数区。

## 7. 启动流程

上电后固定进入 boot stub，再进入 bootloader 决策：

```text
上电/复位
  -> boot stub
  -> bootloader 检查参数区
      -> update_pending=1：停留 bootloader，等待 ESP32
      -> app_valid=1 且 app_crc 匹配：跳转应用
      -> 其他情况：停留 bootloader，等待 ESP32 恢复升级
```

应用要请求 OTA 时，通过业务协议收到 ESP32 升级命令后写 `update_pending=1`，然后软件复位。复位后 bootloader 不再跳转应用。

## 8. 应用链接和中断策略

应用从 `0x0200` 开始链接，不占用低地址 `0x0000..0x01FF`。

boot stub 固定持有复位和中断向量。中断处理有两种可选实现：

- 第一版最小验证：示例应用尽量不依赖中断，只验证启动、版本输出和主循环行为。
- 正式应用支持：boot stub 的固定中断向量跳转到应用中断跳转表，应用构建必须生成固定格式的跳转表。

第一版设计文档先保留中断跳转表要求；具体链接脚本、Keil C51 配置和 SDCC 配置在实现计划中展开。

## 9. RS485 升级协议

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
| `COMMIT` | ESP32 -> STC | CRC 通过后标记新应用有效 |
| `REBOOT` | ESP32 -> STC | 重启运行新应用 |
| `ABORT` | ESP32 -> STC | 取消升级，保持 bootloader 可恢复 |

## 10. 固件包格式

ESP32 下载的固件包应包含 manifest，不应只传裸二进制。

建议字段：

```text
magic
format_version
target_chip = STC8H8K64U
app_base = 0x0200
app_size
app_crc32
version_major
version_minor
version_patch
build_id
min_bootloader_version
payload
signature
```

ESP32 必须先校验 `signature` 或 hash。STC 侧只检查：

- `magic`
- `target_chip`
- `app_base`
- `app_size`
- `app_crc32`
- `min_bootloader_version`

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
| `app_base` | 应用起始地址 |
| `app_size` | 应用长度 |
| `app_crc32` | 应用整体 CRC |
| `version` | 应用版本 |
| `write_offset` | 已写入进度 |
| `fail_reason` | 最近失败原因 |
| `param_crc` | 参数自身 CRC |

参数区必须采用双份或等价的掉电保护结构。写新状态时先写新记录，校验通过后用 `sequence` 选择最新有效记录。不得只有单份状态记录。

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
  -> REBOOT                 写 app_valid 成功后复位

BOOT_ERROR
  -> BOOT_WAIT_BEGIN        ESP32 重新 BEGIN
```

`COMMIT` 之前不得设置 `app_valid=1`。

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
| COMMIT 后应用启动失败 | 应用未完成健康确认时，下次 bootloader 回到升级等待 |
| 低电压或电源不稳 | 拒绝擦写，返回错误 |

由于第一版不做内部 A/B，升级期间旧应用不可用。恢复依赖 bootloader 永久保留和 ESP32 持有完整固件包。

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

## 16. 基础库落点

建议后续实现时拆为：

| 位置 | 内容 |
|---|---|
| `hal/` | `STC8H8K64U` IAP 程序区擦写原语 |
| `utils/` | CRC16、CRC32 |
| `protocols/` | 可选分块传输状态机，不直接绑定具体 UART |
| `examples/` | `h8k64u_rs485_bootloader` 最小示例 |
| `docs/` | 本设计、协议、验证计划、硬件记录 |

模块必须遵守基础库原则：

- 未编译时零占用。
- 不隐藏初始化外设。
- 不自动占用 UART、Timer、中断或全局缓冲。
- 地址、分区、从机地址和引脚必须编译期显式配置。
- 公共 API 不暴露 Keil/SDCC 专属类型。

## 17. 验收计划

第一阶段：PC 模拟 ESP32。

- 能烧入 bootloader。
- 能通过串口/RS485 写入最小 app。
- app 能启动并输出版本。
- 擦除后断电能停留 bootloader。
- 写入中断后能重新升级。
- CRC 错误不会 commit。
- app 超过分区会拒绝。
- 目标芯片不匹配会拒绝。
- bootloader 区不会被擦写。

第二阶段：ESP32 集成。

- ESP32 下载并暂存固件。
- ESP32 验签或 hash。
- RS485 分块发送。
- 超时重试。
- STC 验证 CRC32。
- COMMIT 后 STC 重启运行新版本。
- ESP32 读取新版本并上报结果。

第三阶段：硬件压力。

- 连续升级不少于 100 次。
- 升级中复位。
- 升级中断电。
- RS485 干扰下重试。
- 低电压拒绝升级。
- 不同固件大小边界测试。
- 参数区双份记录掉电测试。

## 18. 当前待确认问题

实现前仍需确认：

- ESP32 是否一定能暂存完整 STC 固件包。
- STC reset 线是否由 ESP32 控制。
- 现场 RS485 总线是一主一从还是一主多从。
- 应用是否需要中断；若需要，必须先确认中断跳转表方案。
- 应用最大可接受代码空间是多少，是否允许 bootloader 占用 4KB 到 8KB。
- 生产烧录工具是否能固定配置 IAP/EEPROM 覆盖程序空间。

## 19. 评审结论

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

第一版应坚持范围收敛，先完成可恢复、可验证、可维护的最小 OTA 基础能力。
