# STC8H8K64U OTA 复用接入指南

本文面向需要在新项目中复用 `Stc8hBase` OTA 能力的开发者，给出从可行性判断、Bootloader/Application 接入、首次工厂安装，到升级、恢复和验收的最短可靠路径。

技术原理、状态机和 wire format 见 [25 H8K64U OTA 设计](25_H8K64U_OTA_DESIGN.md)；通用源码引用方式见 [20 应用项目引用模式](20_APPLICATION_INTEGRATION.md)。

## 1. 先区分两种“下载”

本库涉及两条不同链路：

1. **UART1/STC ISP 工厂下载**：首次安装或救援 Bootloader，必须接触 UART1；不能远程更新 Bootloader。
2. **UART2/RS485 OTA**：Bootloader 已安装后，通过可靠双向链路更新高地址 Application。

OTA 的 BEGIN/DATA/VERIFY/ACTIVATE 流程可以在操作者确认后自动执行，但**不允许**因上电、联网、广播、串口活动、`probe` 或发现新版本而自动擦写。产品若称“自动下载”，必须明确它是“人工授权一次后自动完成传输与校验”，不是无人授权自动升级。

## 2. 当前实现状态

| 能力 | 基础库状态 | 新产品还要做什么 |
|---|---|---|
| 低地址 Bootloader、reset/vector 转发 | 已实现并通过 SDCC/真实芯片 | 配置板级安全输出、地址、baud 和目标 ID |
| Application 高地址链接和控制 API | 已实现并通过 Trial/mark-valid 实测 | 迁移业务 Application 和产品数据布局 |
| Frame/Manifest/UID/CRC/readback | 已实现并通过 Host + RS485 实测 | 选择产品 Transport 和授权入口 |
| 双槽 Params、checkpoint/resume | 已实现并完成真实中断续传 | 在目标电源条件下复测 Brownout |
| PC pack/inspect/factory/probe/update/resume/activate | 已实现并完成真实链路实测 | 纳入项目构建/发布流程 |
| ESP32 永久 Sender、固件暂存、Web/API | 不属于本基础库，尚未产品化 | 由 Controller 项目实现 |
| 数字签名/Secure Boot | 未实现 | 有攻击防护要求时另行设计 |
| Keil C51 | 有 compile-check wrapper，未真实编译 | Windows + Keil 完成链接和硬件验证 |

基础库闭环完成不等于某个业务产品已经上线 OTA。产品只有完成本文第 11 节全部下游检查和板级验收后，才能宣称 OTA 集成完成。

## 3. 适用条件与不适用条件

可以直接复用的前提：

- MCU 是 `STC8H8K64U-45I-LQFP48`；
- 接受固定 Flash 契约和最大 33792-byte Application；
- UART2 链路可双向收发、ACK、超时和重试；
- 首次安装和 Bootloader 救援可使用 UART1/STC ISP；
- 业务能在 Application 启动最早阶段关闭危险输出并接管 Watchdog；
- 接受单 Application：失败后由外部 Sender 重传，不存在本地 A/B 回滚。

不能直接使用的情况：

- 其他 STC 型号或其他 Flash 容量；
- Application 超过 33792 bytes；
- 只有单向 433 发射，没有可靠 ACK/重传；
- 要求远程更新 Bootloader；
- 要求密码学签名、安全启动或抵御总线主动攻击者；
- 无法保证升级时总线上只有一个主站。

## 4. 不可修改的布局契约

```text
0x0000..0x6BFF  27 KiB  常驻 Bootloader，远程不可写
0x6C00..0xEFFF  33 KiB  Application，远程可写
0xF000..0xFBFF   3 KiB  产品持久化数据，OTA 不访问
0xFC00..0xFDFF 512 B    OTA Params A
0xFE00..0xFFFF 512 B    OTA Params B
```

必须同时满足：

- STC ISP `program_eeprom_split=27648`；
- Bootloader HOME 为 `0x0200`；
- reset vector 为 `LJMP 0x0200`；
- Application HOME 为 `0x6C00`；
- Application、产品数据、Params 不能相互重叠；
- 产品项目不得把 `0xFC00..0xFFFF` 当作普通 EEPROM 使用。

不迁移旧布局和旧 Params。接入该布局时应把旧数据视为一次性清除并重新初始化。

## 5. 推荐目录和引用方式

不要复制或维护 OTA 核心私有副本。应用项目应引用独立、唯一的 `Stc8hBase` Git 工作区，并只创建薄 wrapper；正式构建在应用项目自己的记录中写明实际使用的提交：

```text
workspace/
  foundation/Stc8hBase/
  devices/project/
    firmware/bootloader/
      platformio.ini
      link_bootloader.py
      src/boot_config.h
      src/vector_table.c
      src/*_wrap.c
      src/main.c                 # 板级 Transport Adapter
    firmware/application/
      platformio.ini
      link_app_base.py
      src/base/*_wrap.c
      src/main.c
```

可直接对照：

- `examples/platformio/h8k64u_rs485_ota_bootloader/`
- `examples/platformio/h8k64u_ota_min_app/`

通用修复回到 `Stc8hBase`；产品项目只保留引脚、方向控制、安全输出、地址/波特率、目标 ID 和业务命令映射。

## 6. Bootloader 接入

### 6.1 从已验证示例开始

复制示例的工程结构和 wrapper 列表，不复制基础库 `.c` 内容。PlatformIO 至少需要：

```ini
board_upload.maximum_size = 27648
extra_scripts =
    pre:link_bootloader.py
build_flags =
    --stack-auto
    -DSTC8H_UART_ENABLE_UART2=1
    -DSTC8H_UART_ENABLE_BOUNDED_PUTC=1
    -DSTC8H_UART_ENABLE_WRITE_RAM=0
    -DSTC8H_UART_ENABLE_WRITE_CODE=0
    -DSTC8H_IAP_PROGRAM_ENABLE=1
    -DSTC8H_IAP_OTA_PARAMS_ENABLE=1
```

`link_bootloader.py` 必须包含：

```python
Import("env")
env.Append(LINKFLAGS=["--code-loc", "0x0200"])
```

### 6.2 必须配置的产品参数

在 `boot_config.h` 或构建参数中明确：

```c
#define STC8H_OTA_EXPECTED_BOARD_ID       1u
#define STC8H_OTA_EXPECTED_HW_REVISION    1u
#define STC8H_OTA_EXPECTED_APP_ID         1u
#define H8K64U_OTA_LOCAL_ADDR             0x11u
#define H8K64U_OTA_SAFE_OUTPUTS_OFF()     product_outputs_force_safe()
```

同时在板级 `board_config.h` 配置 `STC8H_SYSCLK_HZ`、`STC8H_UART2_BAUD` 和 UART2 pin group。

`H8K64U_OTA_SAFE_OUTPUTS_OFF()` 是产品 BSP 强制契约：

- 必须无动态分配；
- 必须不依赖调度器；
- 必须不启用中断；
- 在任何 Application 判断和总线处理之前关闭电机、继电器、PWM 等危险输出；
- 只有确实没有受控负载的核心板示例才能使用 no-op。

### 6.3 RS485 Transport Adapter

复用 Core/Receiver 时，板级 Adapter 负责：

- UART2 初始化；
- 本机单播地址；
- 原始帧 collector；
- ACK/status 回发；
- duplicate response cache；
- RS485 TX/RX 方向；
- 发送完成后释放总线；
- ACK 发完后再执行受控复位。

自动收发模块可把方向宏定义为空操作。手动 DE/RE 必须实现：

```c
#define BOARD_RS485_TX_ENABLE()  /* DE active, /RE inactive */
#define BOARD_RS485_RX_ENABLE()  /* DE inactive, /RE active */
```

Bootloader 使用 polling，不占 UART2 中断。不要在 Bootloader 内加入业务协议、动态注册、日志框架或固件下载源管理。

### 6.4 向量表

必须保留 `vector_table.c`，将低地址中断槽 0..44 转发到：

```text
0x6C00 + vector_offset
```

UART2 示例：

```text
0x0043 -> 0x6C43
```

SDCC 对大于 31 的 ISR 编号不能直接依赖普通 C 声明，产品若使用这些扩展向量，必须按芯片手册提供汇编入口并检查最终 HEX。

## 7. Application 接入

### 7.1 链接地址

PlatformIO 配置：

```ini
board_upload.maximum_size = 33792
extra_scripts =
    pre:link_app_base.py
```

`link_app_base.py`：

```python
Import("env")
env.Append(LINKFLAGS=["--code-loc", "0x6C00"])
```

发布前必须从 map/HEX 检查 HOME 和 `main` 均不低于 `0x6C00`，最高地址不超过 `0xEFFF`。

### 7.2 只编译必要模块

启用 Application OTA 控制时创建 wrapper，引入：

```text
hal/stc8h_boot_control_iap.c
hal/stc8h_iap_ota_params.c
hal/stc8h_ota_params_store.c
protocols/stc8h_ota_format.c
```

并配置：

```ini
-DSTC8H_IAP_OTA_PARAMS_ENABLE=1
```

不使用 OTA 的构建不要创建这些 wrapper，也不要启用对应宏，从而保持零 OTA ROM/RAM、零额外外设和零中断占用。

### 7.3 启动和健康确认顺序

推荐骨架：

```c
void main(void)
{
    product_outputs_force_safe();
    product_minimum_hardware_init();

    /* Bootloader 留下的 Watchdog 仍在运行；耗时自检期间也要有界喂狗。 */
    if (product_minimum_health_check() != STC8H_OK) {
        product_outputs_force_safe();
        stc8h_boot_controlled_reset();
    }

    /* TRIAL_STARTED 时提交 APP_VALID；普通 APP_VALID 启动时不改写。 */
    (void)stc8h_boot_mark_app_valid();

    while (1) {
        product_poll();
        stc8h_wdt_feed();
    }
}
```

`stc8h_boot_mark_app_valid()` 只在 `TRIAL_STARTED` 时提交；普通 `APP_VALID` 启动调用会返回错误且不改写 Params，因此推荐像上例一样在健康检查通过后调用并由 Bootloader/Watchdog 保证未提交 trial 的后续恢复，不要把普通启动的 no-op 当成健康失败。不要为了尽快启动而在 GPIO 安全态、关键依赖和最小自检之前调用。

### 7.4 产品升级请求

经过认证/授权的业务命令处理流程：

```c
stc8h_u32 session_id = product_generate_nonzero_session();

if (stc8h_boot_request_update(session_id) == STC8H_OK) {
    if (product_send_update_ack(session_id) == STC8H_OK) {
        stc8h_boot_controlled_reset();
    } else {
        (void)stc8h_boot_cancel_update_request(session_id);
    }
}
```

规则：

- `session_id` 必须非零且由本次明确操作生成；
- 先持久化请求，再发送 ACK，最后复位；
- ACK 失败且尚未复位时才允许 cancel；
- 擦除开始后不能 cancel，只能 resume 或显式 restart；
- 普通查询、广播、启动、联网、版本发现不得调用 request API。

## 8. 包、工厂镜像和首次安装

### 8.1 打包

```sh
python3 tools/stc8h_ota.py pack \
  --hex build/application.hex \
  --output build/application.stcota \
  --board-id 1 --hardware-revision 1 --app-id 1 \
  --version 1.2.3 --build 42
python3 tools/stc8h_ota.py inspect build/application.stcota
```

打包 ID 必须与 Bootloader 构建期目标完全一致。

### 8.2 生成工厂产物

```sh
python3 tools/stc8h_ota.py factory \
  --boot-hex build/bootloader.hex \
  --package build/application.stcota \
  --output build/factory.hex
```

输出：

```text
factory.hex          CPU 地址视图，供检查或明确支持分区地址空间的烧录器
factory.code.bin     精确 27 KiB Bootloader code segment
factory.eeprom.bin   精确 37 KiB Application + data/Params segment
```

### 8.3 stcgal 首次 split 迁移

stcgal 1.10 不得把 `factory.hex` 作为单一 code image。已验证流程：

```sh
# 第一次：安装 code 并让下一次 ISP 可擦写完整 EEPROM segment。
stcgal.py -P stc8g -p <STC-UART1> -t 11059.2 -a -b 19200 \
  -o program_eeprom_split=27648 -o eeprom_erase_enabled=true \
  build/factory.code.bin

# 第二次：分别写 code/eeprom，结束后恢复后续 ISP 保留 EEPROM。
stcgal.py -P stc8g -p <STC-UART1> -t 11059.2 -a -b 19200 \
  -o program_eeprom_split=27648 -o eeprom_erase_enabled=false \
  build/factory.code.bin build/factory.eeprom.bin
```

若板卡没有 DTR/RTS 复位或受控电源，`-a` 不能替代人工冷启动；应按板卡 ISP 设计进入 STC BSL。不要用 Application 业务串口承载首次 Bootloader 安装。

## 9. Sender 操作手册

Sender 可直接使用 USB-RS485，也可使用无日志、字节透明的串口中转。中转两侧 baud 必须一致，且总线上不能同时运行第二个主站。

```sh
# 查询，不擦写。
python3 tools/stc8h_ota.py probe \
  --port <sender-serial> --baud 9600 --address 17

# 正常更新；默认要求交互输入 UPDATE <address>。
python3 tools/stc8h_ota.py update \
  --port <sender-serial> --baud 9600 --address 17 \
  --file build/application.stcota

# 从目标持久化 checkpoint 继续。
python3 tools/stc8h_ota.py resume \
  --port <sender-serial> --baud 9600 --address 17 \
  --file build/application.stcota

# update --no-activate 后，单独明确激活已经 VERIFIED 的镜像。
python3 tools/stc8h_ota.py activate \
  --port <sender-serial> --baud 9600 --address 17
```

显式选项：

| 选项 | 使用条件 |
|---|---|
| `--yes` | 包和目标已由外部流程审核，允许非交互执行 |
| `--restart` | 明确放弃当前断点，重新擦除 Application |
| `--allow-downgrade` | 明确允许 package build 小于目标记录 build |
| `--no-activate` | 只传输并 VERIFY，停在 VERIFIED；稍后必须明确运行 `activate` |

不要把 `--yes --restart --allow-downgrade` 固化成无条件默认参数。

## 10. 状态与运维动作

| INFO 状态 | 含义 | 推荐动作 |
|---|---|---|
| `EMPTY` | 没有可启动记录 | 明确执行 `update` |
| `APP_VALID` | Application 已确认 | 正常启动；升级前先由 Application request |
| `UPDATE_REQUESTED` | Application 已授权升级 | 用同一 session 执行 `update` |
| `PREPARING` | 擦除准备中被打断 | 进入恢复，按 INFO/session 显式 restart |
| `RECEIVING` | 有可恢复传输 | 包匹配时 `resume`；放弃时 `--restart` |
| `VERIFIED` | 镜像已完整验证 | 明确 ACTIVATE，或显式 restart/abort |
| `TRIAL_PENDING` | 等待一次 Trial | 复位后 Bootloader 会先留证再启动 |
| `TRIAL_STARTED` | Trial 已启动但未确认 | 不再次启动；显式重传可用镜像 |
| `RECOVERY` | 参数/传输需要恢复 | `probe` 后按 session 和包状态恢复 |
| `FAILED` | Manifest、写入、读回或 CRC 等失败 | 修复原因后用同一 session 显式 `--restart` |

关键保证：

- UID 不匹配在擦除前拒绝；
- 每帧 CRC16、每块写后读回、最终 CRC32；
- 每 2048 bytes 和镜像末尾提交 checkpoint；
- Trial 未 mark-valid 就复位时不会无限启动；
- Bootloader 保留不代表上一版 Application 仍存在。

## 11. 验收清单

### 11.1 每次集成必须自动检查

```sh
tools/check_host_tests_full.sh
tools/check_examples_full.sh
```

下游项目还必须检查：

- Bootloader 不越过 `0x6BFF`；
- Application HOME=`0x6C00`，不越过 `0xEFFF`；
- reset/vector 转发完整；
- package ID 与 Bootloader ID 一致；
- 产品数据只使用 `0xF000..0xFBFF`；
- 未启用 OTA 的构建不出现 OTA 控制符号。

### 11.2 新板至少完成一次真实硬件验收

- Factory code/eeprom 两阶段安装；
- 正常 update/VERIFY/ACTIVATE；
- Application mark-valid 后重启；
- 传输中断并从 2048-byte checkpoint resume；
- 错误 UID 和错误目标；
- 重复 DATA；
- 故意 CRC 失败和 FAILED restart；
- Trial 未确认后复位停留 Bootloader；
- 自动和手动 DE/RE 对应板型的总线释放；
- Brownout、负载安全态和 Watchdog；
- Bootloader 救援 UART1 路径。

## 12. 常见错误

- **Application 仍从 `0x0000` 链接**：会破坏向量和跳转契约，不能打包。
- **把组合 `factory.hex` 直接作为 stcgal code image**：跨 split 写入会失败；使用两个 `.bin`。
- **改了 split 但只烧一次**：当前 BSL 仍按旧 EEPROM 选项工作；按两阶段流程迁移。
- **过早 mark-valid**：坏固件会被永久确认；必须先建立安全态并完成最小健康检查。
- **发现版本后直接 request/update**：违反明确授权约束。
- **升级时 Controller 和 PC 同时做主站**：会争用半双工总线。
- **自动收发模块仍错误驱动不存在的 DE**：方向策略必须属于产品 BSP。
- **手动 DE/RE 发送完立即拉低 DE**：必须覆盖最后停止位和收发器裕量。
- **错误包失败后换随机 session restart**：目标已有 session 时必须复用；当前 PC 工具已自动处理。
- **把 Trial 当作 A/B 回滚**：本方案没有旧 Application 副本。

## 13. 能力边界和后续扩展点

已完成并可复用：

- 低地址常驻 Bootloader；
- 高地址单 Application；
- UID/Manifest 目标约束；
- stop-and-wait、重试、重复块处理；
- 双槽 Params 和掉电 resume；
- 写后读回和整镜像 CRC32；
- Trial/mark-valid/Watchdog；
- PC pack/inspect/factory/probe/update/resume/activate 工具。

产品项目仍需自行实现：

- 固件从云或本地进入 Sender 的可信来源；
- Controller 固件暂存；
- 业务授权、维护窗口、进度和告警 UI；
- 产品 RS485 地址/baud 和板级方向控制；
- 电机/继电器等真实安全输出；
- 数字签名和可信发布链；
- 可靠双向 433 Transport Adapter（如需要）；
- 完整 PCB、电源、Brownout、EMC 和 Keil C51 真实验证。

完成上述产品适配前，应称为“OTA Foundation 已接入”，不能宣称完整产品 OTA 工作流已经上线。
