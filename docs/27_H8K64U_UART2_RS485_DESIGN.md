# H8K64U UART2 中断接收与 RS485 有界发送设计

## 1. 需求与归属

真实 `STC8H8K64U-45I-LQFP48` RS485 项目需要连续接收 UART2 字节，并在发送异常时有界释放半双工总线。现有 UART2 轮询接口只有一个硬件接收标志；主循环阻塞时可能丢失连续帧。现有 `drv_rs485_uart_write()` 对每个字节无限等待 TI，且最后一个字节看到 TI 后可立即撤销 DE。

UART2 寄存器、中断使能、ISR-safe 单字节原语和有界发送等待属于 HAL/Driver 通用能力；Modbus 帧缓冲、静默间隔和业务寄存器仍由应用项目实现。

## 2. 官方硬件事实

依据 `docs/vendor/stc/STC8H-en.pdf` UART2 与 interrupt 章节：

- UART2 中断向量号为 8，入口地址 `0x0043`；
- `IE2` 地址为 `0xAF`，`ES2` 为 bit0；
- UART2 RX/TX 共用一个中断使能和向量，请求条件为 `S2RI || S2TI`；
- `S2RI` 在接收停止位中部由硬件置位，软件读取 S2BUF 后必须清除；
- `S2TI` 在发送停止位开始时由硬件置位，不代表停止位已经完全离开发送引脚；
- UART2 group 0 为 P1.0 RX/P1.1 TX，baud 由 Timer2 产生。

因此，轮询发送与 RX 中断并用时必须临时关闭该 UART 的中断，避免 ISR 清除 S2TI 后轮询发送永久等待；最终字节后必须另行等待停止位和收发器裕量，再撤销 DE。

## 3. HAL API

在 `stc8h_uart` 增加两个独立编译期开关，默认关闭以维持未使用零占用：

```c
STC8H_UART_ENABLE_ISR_API
STC8H_UART_ENABLE_BOUNDED_PUTC
```

启用 ISR API 后提供：

```c
stc8h_status_t stc8h_uart_interrupt_enable(stc8h_uart_id_t uart);
stc8h_status_t stc8h_uart_interrupt_disable(stc8h_uart_id_t uart);
stc8h_u8 stc8h_uart_try_getc(stc8h_uart_id_t uart, stc8h_u8 *value);
void stc8h_uart_clear_tx_flag(stc8h_uart_id_t uart);
```

`try_getc()` 只在 RX flag 已置位时读取一个字节、清 RX flag 并返回 1；无数据或空指针返回 0，不阻塞、不分配缓冲。应用 ISR 自行把字节放入固定缓冲并记录时间。ISR 还必须调用 `clear_tx_flag()` 处理共用向量上的意外 TX flag。

启用 bounded putc 后提供：

```c
stc8h_status_t stc8h_uart_putc_bounded(
    stc8h_uart_id_t uart,
    char ch,
    stc8h_u16 poll_limit);
```

它在写 SBUF 前清 TI，最多检查 `poll_limit` 次；看到 TI 后清除并返回 `STC8H_OK`，参数无效或耗尽返回 `STC8H_ERROR`。`poll_limit` 是 CPU polling 上限，不伪装成精确微秒；应用必须按目标时钟/baud 编译和实测。

旧的阻塞 `stc8h_uart_putc()` 保留给简单日志示例，但可靠 RS485 Driver 不再使用它。

## 4. RS485 Driver 行为

新增配置：

```c
DRV_RS485_UART_ENABLE_RX_INTERRUPT
DRV_RS485_UART_TX_POLL_LIMIT
DRV_RS485_UART_TX_COMPLETE_DELAY_US
```

当 write 被编译时，`TX_POLL_LIMIT` 和 `TX_COMPLETE_DELAY_US` 必须非零。写流程固定为：

1. RX interrupt 模式下先关闭当前 UART 中断；
2. 切到 TX；
3. 每字节调用 bounded putc；
4. 任一字节失败时立即释放到 RX，清 TX flag，恢复 RX interrupt 并返回错误；
5. 最后一个字节成功后等待 `TX_COMPLETE_DELAY_US`；
6. 切回 RX，清 TX flag，恢复 RX interrupt。

`TX_COMPLETE_DELAY_US` 至少覆盖一个停止位、忙等误差和收发器关闭裕量。它由具体 RS485 Build Variant 明确配置：19200 8N1 的一个 bit 约 52.1µs，首个实验项目保守使用 100µs；9600 示例使用 150µs。该延时只保护“最后停止位到撤销 DE”，不是 Modbus `t3.5`；示波器验证后才能缩短。

初始化顺序为 UART init、RX direction、清 TX flag、启用 RX interrupt。未启用 RX interrupt 时，原轮询读取 API 保持可用。

## 5. ISR 所有权

基础库不安装 ISR、不保存全局 ring buffer：

```c
STC8H_INTERRUPT(app_uart2_isr, STC8H_VECTOR_UART2)
{
    stc8h_u8 value;

    if (stc8h_uart_try_getc(STC8H_UART2, &value) != 0u) {
        /* app-owned bounded buffer */
    }
    stc8h_uart_clear_tx_flag(STC8H_UART2);
}
```

这样保持中断向量、缓冲地址空间、容量、overflow 策略和时间戳均由应用决定。模块没有函数指针、动态注册、隐藏中断或固定 RAM。

## 6. 资源与冲突

- 目标：`STC8H8K64U` UART2；UART1/UART3 API 只复用已存在的 flag/register 结构，不自动启用；
- UART2 占用 Timer2 作为 baud generator；
- UART2 interrupt vector 8 由应用独占；
- Driver 不占用 Timer；停止位延时复用 `stc8h_delay_us()` 忙等；
- HAL/Driver 不分配固定 RAM；
- 同一 UART 不允许在 RX interrupt 开启时绕过 Driver 做轮询 TX；
- 不包含 ring buffer、RTU/Modbus、DMA、任务调度、自动方向收发器或多主总线仲裁。

## 7. 验收

无硬件验证：

- host test 覆盖初始化使能中断、成功发送顺序、字节等待失败后释放总线、空写和轮询 RX；
- H8K64U PlatformIO 示例绑定 vector 8 并编译 ISR-safe API；
- SDCC map/listing 确认向量、IE2/S2CON 路径和无固定驱动缓冲；
- `tools/check_host_tests.sh`、`tools/check_examples.sh`、`tools/check_examples_full.sh` 和 `tools/prepare_h8k64u_validation.sh` 通过；
- 未引用新 API 的代表性构建不出现对应符号。

真实硬件仍需：

- 连续字节不丢失；
- 示波器确认最后停止位结束后才撤销 DE；
- 强制 TX flag 不到达时能返回错误并释放总线；
- UART2 RX 中断、主循环和 Watchdog 长稳。
