# H8K64U UART2 RS485 interrupt echo

最小验证目标：在 `STC8H8K64U-45I-LQFP48` 上编译 UART2 vector 8 接收、应用自有 32-byte XDATA ring buffer，以及关闭 RX interrupt 后的有界轮询发送。

硬件接口：

- UART2 group 0：P1.0 RX、P1.1 TX；
- P4.4：RS485 DE 与 `/RE` 共接，高=发送、低=接收；
- 9600 baud、8N1；
- 最终停止位保守等待：150µs，示波器验证前不缩短。

收到的每个字节由 UART2 ISR 放入 ring buffer，主循环逐字节回显。overflow 只置应用标志；基础库不拥有缓冲或 overflow 策略。

```sh
pio run -d examples/platformio/h8k64u_rs485_uart2_irq_echo
```

该构建不自动上传。真实验收需要示波器确认 P4.4 只在完整停止位之后拉低，并用连续字节、缓冲溢出和故障注入验证。
