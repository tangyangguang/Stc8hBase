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
- 已复测 PlatformIO `i2c_scan`，上传时已修调到约 `11.054MHz`，但 UART monitor 仍无输出。
- 已按 STC8H 官方 UART 示例口径修正 UART1：Timer1 使用 16 位自动重装方式。
- 已再次核对官方 UART1 公式，修正 11.0592MHz / 115200 reload 为 `0xFFE8`，并补齐 Timer1 波特率源、UART1 默认引脚组和 P3.0/P3.1 模式设置。
- 已新增 PlatformIO `uart_hello` 最小示例。
- 已复测 PlatformIO `uart_hello`，确认 UART1 115200 输出。
- 已下载必要 STC 官方 PDF 到 `docs/vendor/stc/`，用于离线复核寄存器和芯片资源事实。
- 已补充项目规则：芯片、寄存器和外设实现/调试必须先参考官方或权威资料，并维护参考资料列表。
- 已补充本地资料总索引 `docs/vendor/README.md`，并归档 PCF8574 官方资料、HD44780 权威镜像资料，用于 LCD1602 I2C 背包和字符屏驱动核对。
- 已用 `i2c_lines` 诊断确认：当前板级 I2C 开漏模式需要启用 P1.7/P3.2 内部上拉和数字输入使能。
- 已确认 LCD1602 背包芯片为 8574T / PCF8574T 兼容。
- 已复测 PlatformIO `i2c_scan`，确认 LCD1602 I2C 地址为 `0x27`。
- 已复测 PlatformIO `lcd1602_text`，确认 LCD 显示 `Stc8hBase` / `LCD1602 OK`。
- 已新增 EC11 和按键驱动，并归档 Alps Alpine EC11E 官方资料作为两相 A/B 输出参考。
- 已新增 PlatformIO `ec11_counter` 示例。
- 已确认 EC11 慢速顺时针为 `+1`、慢速逆时针为 `-1`，快速顺时针可触发 `+10` 加速步进。
- 已确认 EC11 SW 短按输出 `SW press` / `SW short`。
- 已根据实测反馈优化按钮长按判定，并把 EC11 测试例程改为 1ms 扫描、100ms 汇总打印，避免串口打印拖慢扫描。
- 已复测 EC11 SW 长按输出 `SW long` / `SW release`。
- 已复测 EC11 慢速旋转和快速旋转，30ms 快速阈值和 10 步进加速生效。
- 已新增 ADC HAL 和 PlatformIO `adc_pot` 示例，读取 P3.3/ADC11。
- 已复测 P3.3/ADC11 电位器输入，ADC 原始值可覆盖 0..1023 范围。
- 已新增 Timer0 1ms tick HAL 和 PlatformIO `timer_tick` 示例。
- 已烧录 `timer_tick` 并验证串口输出：启动行 `Timer0 1ms tick`，之后每约 1 秒输出 `tick`。
- 已新增 `util_soft_timer`，使用 16-bit tick，单个对象占 4 字节 RAM。
- 已新增 PlatformIO `soft_timer_tick` 示例，并完成 SDCC 编译和 16-bit 回绕宿主机测试。
- 已新增 `util_ring_buffer`，使用 DATA RAM 缓冲，保留一个空位区分空/满。
- 已新增 PlatformIO `ring_buffer_demo` 示例，并完成 SDCC 编译和回绕宿主机测试。
- 已新增 PlatformIO `uart_echo_buffered` 示例，使用 UART 轮询接收 + `util_ring_buffer` 回显，不启用 UART 中断。
- 已新增 `util_crc`，包含 `checksum8` 和逐位 `crc16_modbus`。
- 已新增 PlatformIO `crc_demo` 示例，并完成 SDCC 编译和宿主机标准向量测试。
- 已新增 `util_filter`，包含限幅和移位式 IIR 平滑，不使用除法。
- 已新增 PlatformIO `filter_demo` 示例，并完成 SDCC 编译和宿主机边界测试。
- Keil C51 最小编译验证：本机无 Keil 工具，已记录为待人工验证项。
- 进行 STC8H1K08 TSSOP20 后续硬件实测：`filter_demo` 串口输出、`crc_demo` 串口输出、`uart_echo_buffered` 回显、`ring_buffer_demo` 串口输出、`soft_timer_tick` 串口输出和 P1.2 LED 250ms 翻转；`timer_tick` 的 P1.2 LED 500ms 翻转需人工目视确认。

## 2. 待优化项

- LCD1602 写入暂不返回 I2C ACK 错误；硬件调试优先使用 `i2c_scan` 确认地址。
