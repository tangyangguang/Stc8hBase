# 剩余工作

本文档记录当前尚未完成的工作。每次回复用户时，应同步说明剩余工作。

## 1. Milestone 1

- 已完成 `gpio_blink` 最小资源基线。
- 已完成 `milestone1_demo` 综合演示资源报告。
- 已完成 `i2c_scan` 调试示例和资源报告。
- 已补充并验证 PlatformIO `gpio_blink` 最小示例。
- 已完成一次 SDCC map 符号检查，确认未编译模块零占用。
- 已核对 STC 官方手册中的端口模式 SFR 地址。
- 已补充 Milestone 1 硬件测试清单。
- 已按当前核心板把 LED 改为 P1.2 高电平点亮。
- 已尝试通过 `/dev/cu.usbserial-110` 烧录，已识别 STC8H1K08，失败点为切换到 115200 后串口读超时。
- 已为 PlatformIO `gpio_blink` 增加 9600 下载速率配置；第二次已切换到 9600，但擦除前协议帧错误。
- 已取消上传阶段的 `-t 11059` 频率修调，等待再次烧录验证。
- 已将 PlatformIO `gpio_blink` 上传协议从平台默认 `stc8` 改为 `stc8g`。
- 已确认 `stc8g + 9600` 在两块 STC8H1K08 核心板上烧录成功。
- 已确认 `stc8g + 38400` 烧录成功，并作为当前默认下载速度。
- 已记录 `stc8g` 是 stcgal 下载协议名，不是芯片型号。
- 已确认 `gpio_blink` 硬件实测通过：P1.2 LED 快闪/慢闪正常。
- 已将 `gpio_blink` 改为快闪/慢闪组合，便于肉眼区分延时行为。
- 已补充并验证 PlatformIO `i2c_scan` 示例。
- 已补充并验证 PlatformIO `lcd1602_text` 示例。
- 已按当前接线更新板级宏：LCD1602 SDA=P3.2、SCL=P1.7，EC11=P1.0/P1.1/P5.4，电位器=P3.3。
- 已把 PlatformIO 示例上传配置改为 `stc8g + 38400 + -t 11059.2`，确保芯片运行频率与库默认 `11.0592MHz` 一致。
- 已把 PlatformIO `i2c_scan` 改为每约 2 秒重复输出扫描结果，避免打开串口监视器后错过启动输出。
- 复测 PlatformIO `i2c_scan`，确认 UART1 115200 输出和 LCD1602 I2C 地址。
- Keil C51 最小编译验证：本机无 Keil 工具，已记录为待人工验证项。
- 进行 STC8H1K08 TSSOP20 后续硬件实测：UART1、软件 I2C、LCD1602。

## 2. 待优化项

- LCD1602 写入暂不返回 I2C ACK 错误；硬件调试优先使用 `i2c_scan` 确认地址。
