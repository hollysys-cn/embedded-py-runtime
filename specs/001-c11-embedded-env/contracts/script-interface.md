# 脚本接口契约

**日期**: 2026-01-18
**关联**: [spec.md](../spec.md) | [plan.md](../plan.md)

本文档定义开发环境提供的脚本接口和命令行工具的契约规范。

## 1. 工具链安装脚本

### install-toolchain.sh

**用途**: 自动下载、验证并安装 ARM GCC 工具链

**位置**: `.specify/scripts/setup/install-toolchain.sh`

**接口**:

```bash
install-toolchain.sh [OPTIONS]
```

**选项**:
- `--platform <PLATFORM>`: 指定平台 (linux-x86_64|darwin-x86_64|win32)，默认自动检测
- `--version <VERSION>`: 指定工具链版本，默认从 toolchain.json 读取
- `--mirror <MIRROR>`: 使用国内镜像源 (tsinghua|aliyun|ustc)，默认官方源
- `--force`: 强制重新下载，即使已安装
- `--dry-run`: 仅显示将要执行的操作，不实际下载

**退出码**:
- `0`: 成功
- `1`: 参数错误
- `2`: 下载失败
- `3`: 校验失败
- `4`: 解压失败

**标准输出**:
```
[INFO] 检测到平台: linux-x86_64
[INFO] 工具链版本: 10.3-2021.10
[INFO] 下载中... [===========          ] 45%
[INFO] 下载完成
[INFO] 验证 SHA256 校验和...
[INFO] 校验成功
[INFO] 解压中...
[INFO] 安装完成: .toolchain/gcc-arm-none-eabi
```

**标准错误**:
```
[ERROR] SHA256 校验失败
  期望: 97dbb4f019ad1650b732faffcc881689cedc14e2b7ee863d390e0a41ef16c9a3
  实际: 00000000000000000000000000000000000000000000000000000000000000000
```

**前置条件**:
- 需要 `wget` 或 `curl` 命令可用
- 需要 `sha256sum` 或 `shasum` 命令可用
- 需要 `tar` 命令可用（Linux/macOS）或 `unzip`（Windows）

**后置条件**:
- 工具链安装在 `.toolchain/gcc-arm-none-eabi/`
- 创建 `.toolchain/.installed` 标记文件
- 环境变量 `PATH` 更新（通过 source 环境配置脚本）

---

## 2. 工具链验证脚本

### verify-toolchain.sh

**用途**: 验证工具链是否正确安装并可用

**位置**: `.specify/scripts/setup/verify-toolchain.sh`

**接口**:

```bash
verify-toolchain.sh [--verbose]
```

**选项**:
- `--verbose`: 显示详细的验证信息

**退出码**:
- `0`: 验证通过
- `1`: 工具链未安装
- `2`: 工具链版本不匹配
- `3`: 必要工具缺失

**标准输出**:
```
[OK] arm-none-eabi-gcc: 版本 10.3.1
[OK] arm-none-eabi-g++: 版本 10.3.1
[OK] arm-none-eabi-gdb: 版本 10.2
[OK] C11 支持: 已启用
[OK] Cortex-M4 支持: 已启用
验证通过 ✓
```

**验证项**:
1. 工具链可执行文件是否存在
2. 版本号是否正确
3. C11 标准支持（编译测试程序）
4. Cortex-M4 目标支持（检查 `--target-help` 输出）

---

## 3. 环境配置脚本

### configure-environment.sh

**用途**: 配置开发环境变量和路径

**位置**: `.specify/scripts/setup/configure-environment.sh`

**接口**:

```bash
source configure-environment.sh
```

**注意**: 必须使用 `source` 或 `.` 执行，以便修改当前 Shell 环境

**行为**:
- 将工具链的 `bin` 目录添加到 `PATH`
- 设置 `ARM_TOOLCHAIN_PATH` 环境变量
- 设置 `CMAKE_TOOLCHAIN_FILE` 指向 CMake 工具链文件
- 创建便捷别名（如 `arm-gcc`, `arm-gdb`）

**标准输出**:
```
[INFO] 工具链路径: /path/to/.toolchain/gcc-arm-none-eabi
[INFO] 已添加到 PATH
[INFO] CMAKE_TOOLCHAIN_FILE=/path/to/.specify/templates/cmake/toolchain-arm-cortex-m4.cmake
[INFO] 环境配置完成
```

---

## 4. 构建脚本

### build.sh

**用途**: 执行项目构建

**位置**: `.specify/scripts/build/build.sh`

**接口**:

```bash
build.sh [OPTIONS] [TARGET]
```

**选项**:
- `--config <CONFIG>`: 构建配置 (Debug|Release|MinSizeRel)，默认 Debug
- `--clean`: 构建前清理
- `--verbose`: 显示详细的构建输出
- `--jobs <N>`: 并行构建作业数，默认 CPU 核心数

**TARGET**: CMake 目标名称，默认 `all`

**退出码**:
- `0`: 构建成功
- `1`: 配置失败
- `2`: 编译失败
- `3`: 链接失败
- `10`: 构建锁冲突（另一个构建进程正在运行）

**标准输出**:
```
[INFO] 构建配置: Debug
[INFO] 平台: ARM Cortex-M4
[INFO] 并行作业: 8
[INFO] CMake 配置中...
[INFO] 编译中... [1/25]
[INFO] 编译中... [25/25]
[INFO] 链接中...
[INFO] 生成 HEX 文件...
[INFO] 构建成功
[INFO]
  代码段大小: 12288 字节
  数据段大小: 512 字节
  BSS 段大小: 2048 字节
  总计: 14848 字节
[INFO]
  输出文件: build/bin/firmware.elf
            build/bin/firmware.hex
            build/bin/firmware.bin
```

**构建锁机制**:
- 检查 `build/.build.lock` 文件
- 如果存在且进程仍在运行，拒绝启动新构建
- 构建完成或失败时自动删除锁文件
- 支持 `--force` 选项强制删除过期锁

---

## 5. 清理脚本

### clean.sh

**用途**: 清理构建产物

**位置**: `.specify/scripts/build/clean.sh`

**接口**:

```bash
clean.sh [OPTIONS]
```

**选项**:
- `--all`: 清理所有内容（包括 CMake 缓存）
- `--obj`: 仅清理目标文件
- `--artifacts`: 仅清理最终产物（.elf, .hex, .bin）

**默认行为**: 清理所有构建产物但保留 CMake 缓存

**退出码**:
- `0`: 清理成功
- `1`: 权限错误

---

## 6. 调试服务器脚本

### start-debug-server.sh

**用途**: 启动 OpenOCD 调试服务器

**位置**: `.specify/scripts/debug/start-debug-server.sh`

**接口**:

```bash
start-debug-server.sh [OPTIONS]
```

**选项**:
- `--interface <INTERFACE>`: 调试接口 (stlink-v2|jlink|cmsis-dap)，默认 stlink-v2
- `--target <TARGET>`: 目标 MCU (stm32f4x|stm32f7x)，默认 stm32f4x
- `--port <PORT>`: GDB 服务器端口，默认 3333
- `--daemon`: 以守护进程模式运行

**退出码**:
- `0`: 服务器启动成功
- `1`: OpenOCD 未安装
- `2`: 调试器未连接
- `3`: 目标设备未找到

**标准输出**:
```
[INFO] 启动 OpenOCD...
[INFO] 接口: ST-Link/V2
[INFO] 目标: STM32F4xx
[INFO] GDB 端口: 3333
[INFO] OpenOCD 服务器运行中
[INFO] 等待 GDB 连接...
```

---

## 7. GDB 连接脚本

### connect-gdb.sh

**用途**: 启动 GDB 并连接到调试服务器

**位置**: `.specify/scripts/debug/connect-gdb.sh`

**接口**:

```bash
connect-gdb.sh <ELF_FILE> [OPTIONS]
```

**选项**:
- `--port <PORT>`: OpenOCD GDB 端口，默认 3333
- `--tui`: 使用 TUI 模式（文本用户界面）
- `--load`: 连接后自动加载程序到目标设备

**退出码**:
- `0`: 正常退出
- `1`: ELF 文件不存在
- `2`: 无法连接到调试服务器

---

## 脚本通用规范

### 1. 错误处理

所有脚本必须：
- 使用 `set -e` 在命令失败时立即退出
- 使用 `set -u` 检测未定义变量
- 使用 `set -o pipefail` 捕获管道中的错误
- 提供清晰的错误消息到 stderr

### 2. 日志格式

```bash
# 信息
echo "[INFO] 消息内容"

# 警告
echo "[WARN] 警告内容" >&2

# 错误
echo "[ERROR] 错误内容" >&2

# 成功
echo "[OK] 成功消息"
```

### 3. 颜色输出（可选）

支持通过 `--no-color` 选项禁用：

```bash
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}[OK]${NC} 成功"
```

### 4. 帮助信息

所有脚本必须支持 `--help` 选项，输出使用说明。

---

## 集成测试契约

每个脚本都应有对应的 bats 测试文件，位于 `tests/` 目录：

```
tests/
├── toolchain/
│   ├── test_install.bats      # 测试 install-toolchain.sh
│   └── test_verify.bats       # 测试 verify-toolchain.sh
├── build/
│   ├── test_build.bats        # 测试 build.sh
│   └── test_clean.bats        # 测试 clean.sh
└── debug/
    └── test_debug_server.bats # 测试 start-debug-server.sh
```

**测试覆盖率目标**: >80%
