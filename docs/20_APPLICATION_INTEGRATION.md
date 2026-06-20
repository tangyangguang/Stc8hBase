# 应用项目引用模式

本文记录真实应用项目接入 `Stc8hBase` 的推荐方式。基础库只提供芯片级能力、薄 HAL、可复用外设协议和工具；应用项目继续负责板级接线、电平、低功耗前后 IO 状态、业务命令映射和产品状态机。

## 1. Makefile 直接编译源文件

适合小型 SDCC/Make 项目。应用项目在 `CFLAGS` 中加入基础库 `core/hal/drivers/utils` include path，并只把实际使用的 `.c` 文件放入 `SRCS`。

这种方式的优点是构建关系直观、未使用模块天然零占用。项目侧仍应用 `STC8H_CONFIG_INCLUDE` 和 `STC8H_PINS_INCLUDE` 指定自己的 `board_config.h`、`board_pins.h`。

## 2. PlatformIO wrapper include

适合 PlatformIO 项目。应用项目在 `src/base/*.c` 中用 wrapper 引入基础库实现，例如：

```c
#define STC8H_CONFIG_INCLUDE "board_config.h"
#define STC8H_PINS_INCLUDE "board_pins.h"
#include "../../../Stc8hBase/hal/stc8h_gpio.c"
```

wrapper 方式避免复制基础库源码，也能让 PlatformIO 自动编译项目 `src/` 下的入口。只为实际使用的基础库源文件创建 wrapper；例如普通 GPIO 只需要 `stc8h_gpio.c`，需要 `P1/P3/P5` 上拉或数字输入 mask 操作时才额外加入 `stc8h_gpio_xfr.c`。

## 3. 边界原则

- 基础库负责 GPIO、EXTI、Timer、PWM、UART、power-down 入口，以及 NEC `mark/space` 编码和解码状态机。
- 板级代码负责引脚组、电平有效性、上拉/数字输入、未用脚低功耗收敛、唤醒源和外设资源分配。
- 应用代码负责产品命令表、repeat 策略、灯光状态机、按键优先级和调试日志。
- Timer0 模式由单个固件统一分配；不要同时把 Timer0 用作 1T 阻塞延时和 12T free-run 接收计时。
- `PWMA` 1..4 共用周期；同一固件需要多个 PWM 通道时，必须选择同一个频率。

## 4. 来自真实项目的结论

红外夜灯项目使用 Makefile 直接编译基础库源文件，接收端用 Timer0 12T free-run + INT0 边沿捕获解码 NEC。红外遥控器项目使用 PlatformIO wrapper include，发送端用 PWM 产生 38kHz 载波，用 Timer0 1T 阻塞延时输出 NEC 帧。

这两个项目验证了当前基础库边界是合适的：不需要把遥控器或夜灯业务沉淀进基础库；需要沉淀的是 GPIO XFR mask、EXTI 批量清标志、PWM 共享周期语义和 Timer0 资源说明这类小而明确的基础能力。

## 5. 8KB 项目的裁剪构建示例

STC8H1K08 红外夜灯这类 8KB flash 项目可以继续编译通用 HAL 源文件，但通过构建参数声明实际使用的芯片资源：

```sh
TRIM_FLAGS='-DSTC8H_GPIO_PORT_MASK=0x0A \
-DSTC8H_PWM_GROUP_MASK=0x01 \
-DSTC8H_PWM_A_CHANNEL_MASK=0x01 \
-DSTC8H_PWM_B_CHANNEL_MASK=0x00 \
-DSTC8H_TIMER_ENABLE_1MS=0 \
-DSTC8H_TIMER_ENABLE_INTERRUPT_CONTROL=0 \
-DSTC8H_UART_ASSUME_UART1=1 \
-DSTC8H_UART_ENABLE_WRITE_RAM=0 \
-DSTC8H_UART_ENABLE_RX=0 \
-DDRV_IR_RX_ENABLE_FALLING=0'
```

该配置表示：只访问 `P1/P3`，只使用 `PWMA channel 1` 并关闭 `PWMB` 分支，Timer0 用 12T free-run/read/reset/start/stop，不使用 Timer 中断 API，UART 只保留 UART1 TX code-string 输出，IR RX 只保留 NEC pulse 解码。生产版若关闭 UART 日志，应让 Makefile 不编译 `hal/stc8h_uart.c`。

验证方式：

```sh
BASE_FLAGS="-mmcs51 --std-sdcc11 -Iboard -Isrc -I${BASE}/core -I${BASE}/hal -I${BASE}/drivers -DSTC8H_CONFIG_INCLUDE=\\\"board_config.h\\\" -DSTC8H_PINS_INCLUDE=\\\"board_pins.h\\\" -DAPP_POWER_DOWN_ENABLE=1"
make clean
make IR_UART_DEBUG=1 IR_VERBOSE_DEBUG=0 POWER_DOWN_ENABLE=1
make clean
make CFLAGS="$BASE_FLAGS -DAPP_IR_UART_DEBUG=1 -DAPP_IR_VERBOSE_DEBUG=0 $TRIM_FLAGS" IR_UART_DEBUG=1 IR_VERBOSE_DEBUG=0 POWER_DOWN_ENABLE=1
make clean
make CFLAGS="$BASE_FLAGS -DAPP_IR_UART_DEBUG=0 -DAPP_IR_VERBOSE_DEBUG=0 $TRIM_FLAGS" IR_UART_DEBUG=0 IR_VERBOSE_DEBUG=0 POWER_DOWN_ENABLE=1
```

比较 `build/*.mem` 里的 `ROM/EPROM/FLASH` 和 `Stack starts`，并检查 `build/*.map` 中是否还有已关闭路径的符号前缀。

## 6. PlatformIO RF 链路裁剪

STC8H1K08 项目通过 PlatformIO wrapper 接入 `proto_rf_link` 时，不要在小内存固件里默认拉入完整链路 API。SDCC/mcs51 会给未调用的外部函数参数分配内部 RAM，整单元 wrapper 容易和 `drv_nrf24l01`、业务协议 wrapper 一起耗尽 DSEG/OSEG 连续空间。

推荐做法是在 `platformio.ini` 里声明当前阶段实际需要的链路 API。阶段 2 只编译 radio 驱动和应用 radio 骨架时，可关闭全部 `PROTO_RF_LINK_ENABLE_*`；真实接入发送、接收、心跳或状态回传时，再逐项打开 `CONNECT`、`SEND_DATA`、`POLL`、`SEND_STATUS`、`SEND_HEARTBEAT`、`TICK`、`GET_STATE`。

基础库示例 `examples/platformio/rf_link_nrf24_small` 只记录 `drv_nrf24l01 + stc8h_spi + proto_rf_link` 在 STC8H1K08 上的裁剪接入和资源边界；它不是链路可靠性运行参考。需要真实 nRF24 TX 完成、`MAX_RT` 恢复或 ACK payload 判定时，以 `nrf24_pair_diag`、`nrf24_fixed_ping` 和 `nrf24_ack_payload` 的 wait + `drv_nrf24l01_complete_tx()` 流程为准。

## 7. H8K64U OTA 接入边界

当前 `STC8H8K64U` OTA 示例采用单应用区布局：

```text
0x0000..0x01FF  boot stub / reset vector
0x0200..0xB3FF  OTA application
0xB400..0xFBFF  bootloader
0xFC00..0xFFFF  dual OTA parameter records
```

应用项目生成 OTA 镜像时必须从 `0x0200` 链接，且 manifest 的 `app_size` 不能超过 `0xB200` 字节。ESP32 侧负责云端下载、验签、暂存完整镜像、总线重试和业务升级窗口；基础库侧提供 manifest/frame/params 编解码、状态机、IAP backend、RS485 bootloader 示例和边界检查。

OTA 应用启动后必须先把业务输出初始化到安全态，再标记 app valid。基础库提供两类参考构建：

- `h8k64u_ota_min_app` 默认环境只保留 `stc8h_boot_mark_app_valid()` 调用点，不写参数区，适合做链接和调用顺序验证。
- `h8k64u_ota_min_app` 的 `STC8H8K64U_mark_valid_iap` 环境会把 `stc8h_boot_mark_app_valid()` 接到 `hal/stc8h_ota_params_store` 和 `hal/stc8h_iap_ota_params`，用于编译验证真实参数区 mark-valid 路径。实际烧录运行前必须确认允许写擦 `0xFC00/0xFE00` 参数扇区。

RS485 项目使用 `drivers/drv_rs485_uart` 时，需要在板级定义：

```c
#define BOARD_RS485_TX_ENABLE()  /* DE/RE 切到发送 */
#define BOARD_RS485_RX_ENABLE()  /* DE/RE 切到接收 */
```

基础库不实现 Modbus 或灌溉业务寄存器；业务层收到升级命令后应关闭所有输出、拒绝普通业务命令，并让 ESP32 通过 OTA 帧协议驱动 bootloader。
