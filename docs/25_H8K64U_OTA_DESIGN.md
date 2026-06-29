# STC8H8K64U OTA/IAP Bootloader 设计

本文只记录当前设计基线、边界和验证入口，不记录逐次调试日志。

## 目标

目标场景：

- ESP32 负责联网下载、验签或 hash 校验、暂存完整 STC 固件。
- ESP32 通过 RS485 或串口透传链路把 OTA payload 分块发送给 `STC8H8K64U-45I-LQFP48`。
- STC8H8K64U 专用 bootloader 使用 IAP 擦写单应用区。
- 写入完成后由 STC 侧做整包 CRC32 校验，commit 后试启动新应用。

本能力只面向 `STC8H8K64U-45I-LQFP48`。不做 STC8H 全系列自动适配，不支持 `STC8H1K08` 程序 OTA。

## 资料依据

依据记录见 `docs/10_REFERENCES.md`。当前设计依赖以下事实：

- STC8H8K64U 有最多 64KB Flash，擦除页为 512 字节，IAP/EEPROM 边界可在生产烧录配置中改变。
- 若要由用户 bootloader IAP 改写应用程序区，生产烧录时必须把可 IAP 改写区域规划为覆盖应用区。
- STC 内置 ISP/BSL 更适合生产烧录和救援，不适合作为产品 OTA 主协议。
- bootloader 与应用必须分区，metadata/参数区必须可恢复，普通应用不得擦写 bootloader 区。

## 推荐方案

```text
ESP32 下载/验签/暂存完整固件
  -> RS485 或串口透传分块发送 OTA frame
    -> STC8H8K64U bootloader 接收 payload
      -> IAP 擦写应用区
        -> CRC32 校验
          -> COMMIT 写参数区
            -> 试启动应用
              -> 应用自检后 mark_app_valid
```

ESP32 不应在产品升级流程中模拟 STC-ISP 协议。原因是 STC ISP 协议公开资料不足，且产品 OTA 需要地址、版本、CRC、重试、commit 和恢复状态机。

## 范围

基础库负责：

- H8K64U 程序区 IAP 擦页、写入、读回和边界检查。
- OTA manifest、frame、参数区记录的规范化编解码。
- 单应用区写入状态机、CRC32 校验、commit、trial boot、mark-valid 和 recovery。
- boot stub、应用链接基址、bootloader 高地址布局和最小示例。
- RS485/UART 参考适配，不绑定业务协议。

ESP32 或应用项目负责：

- 云端版本查询、固件下载、验签或 hash 校验。
- 完整固件暂存、重试、超时、升级窗口、结果上报。
- RS485 地址分配、总线仲裁和业务协议封装。
- 升级前停止业务命令并关闭危险输出。
- 生产烧录时固定正确的 code/EEPROM split。

明确不支持：

- STC 内部 A/B 双应用区和真正 rollback。
- bootloader 自升级。
- STC 侧 HTTPS、证书、JSON、压缩或复杂签名验签。
- SPI/寄存器型 433 模块配置。
- Modbus 或灌溉业务协议。
- 未编译 OTA 模块时产生任何 ROM/RAM/外设占用。

## Flash 布局

当前实现基线：

| 区域 | 范围 | 用途 |
|---|---:|---|
| Boot stub / 向量区 | `0x0000..0x01FF` | 复位入口和中断跳转表 |
| Application | `0x0200..0xB3FF` | OTA 应用镜像 |
| Bootloader | `0xB400..0xFBFF` | 接收、IAP、CRC、状态机 |
| Boot 参数区 | `0xFC00..0xFFFF` | 双份参数记录 |

应用镜像大小上限为 `0xB200` 字节。应用必须从 `0x0200` 链接；默认从 `0x0000` 链接的固件不能作为 OTA 镜像。

生产前必须确认 STC8H8K64U 的 IAP/EEPROM split 允许 bootloader IAP 覆盖应用区。若仍只保留顶部 512 字节 EEPROM/IAP 区，OTA 会在擦写应用区时失败。

`tools/check_examples_full.sh` 会检查 bootloader reset stub、`0xB400` 高地址入口和 `0xFC00..0xFFFF` 参数区边界。

## 启动流程

```text
上电/复位
  -> boot stub
  -> bootloader 读取参数区
    -> update_pending=1：留在 bootloader
    -> app_valid=1 且 CRC32 匹配：跳转应用
    -> BOOT_COMMITTED 且 boot_attempted=0 且 CRC32 匹配：记录 boot_attempted=1 后试启动应用
    -> 其他情况：留在 bootloader 等待重新升级
```

COMMIT 不直接设置 `app_valid=1`。新应用完成早期自检并把输出初始化到安全态后，才调用 mark-valid 能力。

若试启动后复位但 `app_valid` 仍为 0，bootloader 不再继续跳转该应用，而是进入 recovery 等待 ESP32 重新下发固件。

## OTA Frame

OTA frame 是传输无关协议，可运行在 RS485、普通 UART 或透明串口 433 模块上。推荐帧格式：

```text
SOF | proto_ver | dst_addr | src_addr | cmd | seq | offset | len | payload | crc16
```

第一版 payload 建议为 64 或 128 字节。STC 侧不得依赖大缓冲。

最小命令集：

| 命令 | 作用 |
|---|---|
| `BEGIN` | 下发 manifest 并准备擦写 |
| `WRITE_BLOCK` | 顺序写入应用镜像块 |
| `VERIFY` | 计算应用区 CRC32 |
| `COMMIT` | CRC 通过后提交 trial boot |
| `ABORT` | 取消当前升级 |
| `STATUS` | 回报状态、offset 和错误原因 |

写入策略：

- 只支持顺序写入。
- `offset` 必须等于当前 `write_offset`。
- 允许重复发送上一块已接受 chunk，但内容必须与 flash 读回一致。
- 未来 offset、乱序 chunk、内容不一致的重复 chunk 都拒绝。
- 断点恢复由 ESP32 根据 STATUS 中的 `write_offset` 决定是否重发。

## Manifest

manifest 在线上传输时必须使用固定 little-endian 字节序，不允许直接发送 C struct 内存。

STC 侧检查字段：

- `magic`
- `format_version`
- `target_chip`
- `board_id`
- `hw_revision`
- `app_id`
- `app_base`
- `app_size`
- `app_crc32`
- `min_bootloader_version`
- `flags`
- `manifest_crc`

固件签名、hash、灰度策略和云端认证由 ESP32 负责。STC 侧只做资源可承受的目标、范围、版本和 CRC 检查。

## 参数区

参数区使用 A/B 双记录，每条记录占用一个 512 字节擦除页：

| 记录 | 范围 |
|---|---:|
| Param A | `0xFC00..0xFDFF` |
| Param B | `0xFE00..0xFFFF` |

记录必须包含：

- 参数 magic、版本、sequence。
- OTA state、`app_valid`、`update_pending`、`boot_attempted`。
- `app_base`、`app_size`、`app_crc32`、版本号、`write_offset`。
- 最近失败原因。
- 参数记录 CRC。

写入规则：

- 写新状态时选择非当前记录页。
- 先擦除，再写完整规范化记录，再读回校验。
- A/B 都有效时选择 sequence 更新的一条。
- sequence 相同或 CRC 错误时进入 recovery。
- 参数区写入失败不得跳转应用。

## 状态机

核心状态：

```text
BOOT_IDLE
BOOT_WAIT_BEGIN
BOOT_ERASING
BOOT_RECEIVING
BOOT_VERIFYING
BOOT_COMMITTED
BOOT_ERROR
```

关键规则：

- `BEGIN` 校验 manifest 后才允许擦除应用区。
- `WRITE_BLOCK` 只能写应用区范围内的顺序 chunk。
- `VERIFY` 必须读取应用区并计算 CRC32。
- `COMMIT` 只能在 CRC32 匹配后成功。
- `ABORT` 只清理接收状态，不擦写 bootloader、boot stub 或参数保留区。
- 单应用区方案不承诺回滚旧应用，只承诺停留 bootloader 后重新升级。

## 硬件边界

硬件设计必须满足：

- ESP32 能控制 STC reset，或能通过业务协议让 STC 软件复位。
- RS485 DE/RE 方向可由固件控制。
- 多从机总线必须有唯一地址。
- 升级期间普通业务通信暂停。
- 电源不稳或低电压时不得执行 IAP 写擦。
- ISP 下载通道必须保留为生产和救援通道。

真实项目必须实测 RS485 DE/RE 时序、总线冲突、断电中断、参数区 A/B 恢复和不同镜像大小边界。

## 验证入口

日常快检：

```sh
tools/check_host_tests.sh
tools/check_examples.sh
```

发布前完整验证：

```sh
tools/check_host_tests_full.sh
tools/check_examples_full.sh
tools/prepare_h8k64u_validation.sh
```

可选硬件脚本：

- `tools/h8k64u_uart1_ota_smoke.py`：UART1 最小 OTA 闭环。
- `tools/h8k64u_uart1_ota_faults.py`：UART1 故障和恢复场景。

这些脚本会构建、上传或触发 IAP 写擦，必须在确认端口、芯片和测试板可接受风险后手动运行。

## 已验证结论

稳定结论只保留影响设计的事实：

- UART1 bootloader reset stub 能进入高地址 bootloader。
- `program_eeprom_split=512` 这类允许 IAP 覆盖应用区的配置下，单应用区 IAP 写入、CRC32 校验、COMMIT、trial boot 和 mark-valid 路径已通过 UART1 硬件闭环。
- `0.5KB EEPROM` split 只能写顶部参数区，不能 IAP 擦写 `0x0200` 应用区。
- CPU 地址到 STC IAP 地址必须显式转换，不能把 `0x0200` 直接当作 IAP 地址寄存器值。
- OTA manifest 保存不能依赖结构体整体赋值；8051/SDCC 场景下应逐字段复制，避免 generic memcpy 和地址空间问题。
- bootloader 跳转应用前必须按参数区记录重新读取应用区并计算 CRC32。

## 剩余风险

- 生产烧录流程必须能稳定设置正确的 code/EEPROM split。
- 真实 RS485 收发器 DE/RE 时序和多从机场景仍需实测。
- 真实断电中断和参数区 A/B 单页损坏恢复仍需实测。
- 单应用区 OTA 失败时旧应用不可继续运行；需要真正 rollback 时必须增加外部暂存区或改为双应用区。
- bootloader 体积必须持续检查，不能进入 `0xFC00..0xFFFF` 参数区。
