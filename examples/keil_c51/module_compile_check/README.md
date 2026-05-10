# Keil C51 模块编译验证

本目录用于在 Windows + Keil C51 环境中验证基础库源码是否能被 C51 编译器接受。

## 1. 验证目标

- 只做编译验证，不烧录，不链接业务固件。
- 覆盖当前已实现的 core、HAL、drivers、utils 模块。
- 使用 wrapper 源文件定义 `STC8H_CONFIG_INCLUDE` 和 `STC8H_PINS_INCLUDE`，避免 Keil 命令行对带引号宏的转义差异。

## 2. 使用方式

在 Windows 命令行中进入本目录：

```bat
cd examples\keil_c51\module_compile_check
build_c51.bat
```

前提：

- 已安装 Keil C51。
- `C51.exe` 在 `PATH` 中；或者先运行 Keil 安装目录下的环境配置脚本。

如果没有配置 `PATH`，可临时执行：

```bat
set PATH=C:\Keil_v5\C51\BIN;%PATH%
```

实际路径按本机安装位置调整。

## 3. 通过标准

- 所有 wrapper 均编译成功。
- `build\*.lst` 中没有 C51 语法错误。
- 如果只有未使用函数、优化相关 warning，需要单独记录；如果是 memory qualifier、SFR、interrupt、code 字符串相关 warning，应视为需要修正。

## 4. 结果记录

验证后把结果记录到：

```text
docs/RESOURCE_REPORT.md
docs/15_REMAINING_WORK.md
```

当前仓库无法在 macOS 上自动完成 Keil C51 验证，因此此目录是人工验证入口。
