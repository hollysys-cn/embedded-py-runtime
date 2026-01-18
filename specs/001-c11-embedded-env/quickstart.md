# 快速入门指南: C11 嵌入式开发环境

**目标平台**: ARM Cortex-M4 (STM32F4 系列)
**工具链**: ARM GCC Embedded 10.3+
**构建系统**: CMake 3.20+

---

## 前置要求

### 软件依赖

根据您的操作系统安装以下工具：

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y \
    wget \
    cmake \
    build-essential \
    git \
    python3
```

#### macOS
```bash
# 安装 Homebrew (如果还未安装)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install wget cmake git python3
```

#### Windows
1. 安装 [Git for Windows](https://git-scm.com/download/win)（包含 Git Bash）
2. 安装 [CMake](https://cmake.org/download/)（添加到 PATH）
3. 安装 [Python 3](https://www.python.org/downloads/)（添加到 PATH）

推荐使用 Git Bash 作为命令行环境。

---

## 第一步：设置开发环境

### 1.1 克隆项目（如果还没有）

```bash
git clone https://github.com/hollysys-cn/embedded-py-runtime.git
cd embedded-py-runtime
```

### 1.2 安装 ARM GCC 工具链

使用自动化安装脚本：

```bash
# 使用官方源（国外网络）
.specify/scripts/setup/install-toolchain.sh

# 或使用国内镜像源（推荐）
.specify/scripts/setup/install-toolchain.sh --mirror tsinghua
```

安装过程约需 5-10 分钟（取决于网络速度）。

**验证安装**:

```bash
.specify/scripts/setup/verify-toolchain.sh
```

预期输出：
```
[OK] arm-none-eabi-gcc: 版本 10.3.1
[OK] arm-none-eabi-g++: 版本 10.3.1
[OK] arm-none-eabi-gdb: 版本 10.2
[OK] C11 支持: 已启用
[OK] Cortex-M4 支持: 已启用
验证通过 ✓
```

### 1.3 配置环境变量

```bash
source .specify/scripts/setup/configure-environment.sh
```

**提示**: 将此命令添加到 `~/.bashrc` 或 `~/.zshrc` 以便每次自动加载：

```bash
echo "source $(pwd)/.specify/scripts/setup/configure-environment.sh" >> ~/.bashrc
```

---

## 第二步：创建第一个 C11 项目

### 2.1 使用示例项目

项目已包含示例代码，位于 `.specify/templates/examples/hello-c11/`：

```bash
# 复制示例到工作目录
cp -r .specify/templates/examples/hello-c11 ./my-first-project
cd my-first-project
```

### 2.2 查看项目结构

```
my-first-project/
├── CMakeLists.txt       # CMake 配置文件
├── main.c               # 主程序源码
└── README.md            # 项目说明
```

**main.c** 内容示例：

```c
#include <stdint.h>
#include <stdio.h>

// 使用 C11 特性: _Static_assert
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

// 使用 C11 特性: 匿名结构体
typedef struct {
    struct {
        uint8_t r, g, b;
    };  // 匿名结构体
    uint8_t alpha;
} Color;

int main(void) {
    // 使用 C11 特性: 复合字面量和指定初始化
    Color red = { .r = 255, .g = 0, .b = 0, .alpha = 255 };

    printf("Hello from C11!\n");
    printf("Color RGB: (%d, %d, %d)\n", red.r, red.g, red.b);

    return 0;
}
```

### 2.3 配置 CMake

**CMakeLists.txt** 内容示例：

```cmake
cmake_minimum_required(VERSION 3.20)

# 设置工具链文件（必须在 project() 之前）
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/../.specify/templates/cmake/toolchain-arm-cortex-m4.cmake)

# 定义项目
project(hello-c11 C ASM)

# C11 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 源文件
add_executable(${PROJECT_NAME} main.c)

# 链接器脚本（针对 STM32F407VG）
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F407VGTx_FLASH.ld)
target_link_options(${PROJECT_NAME} PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
)

# 生成 HEX 和 BIN 文件
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "生成 HEX 和 BIN 文件"
)

# 显示代码大小
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "代码大小统计"
)
```

---

## 第三步：构建项目

### 3.1 执行构建

```bash
# 返回项目根目录
cd ..

# 执行构建
.specify/scripts/build/build.sh
```

或手动执行 CMake：

```bash
# 配置
cmake -B build -S my-first-project -DCMAKE_BUILD_TYPE=Debug

# 构建
cmake --build build
```

### 3.2 查看构建产物

构建成功后，产物位于 `build/bin/`：

```bash
ls -lh build/bin/
```

输出：
```
hello-c11.elf    # ELF 可执行文件（用于调试）
hello-c11.hex    # HEX 文件（用于烧录）
hello-c11.bin    # 二进制文件（用于烧录）
hello-c11.map    # 内存映射文件（用于分析）
```

### 3.3 检查代码大小

```bash
arm-none-eabi-size build/bin/hello-c11.elf
```

输出示例：
```
   text    data     bss     dec     hex filename
  12288     512    2048   14848    3a00 build/bin/hello-c11.elf
```

- **text**: 代码段大小（Flash 占用）
- **data**: 已初始化数据段（Flash + RAM 占用）
- **bss**: 未初始化数据段（RAM 占用）

---

## 第四步：调试程序

### 4.1 连接硬件

1. 将 ST-Link 调试器连接到 STM32F4 开发板的 SWD 接口
2. 通过 USB 连接调试器到电脑
3. 确保开发板供电

### 4.2 安装 OpenOCD（如果还未安装）

#### Linux
```bash
sudo apt-get install openocd
```

#### macOS
```bash
brew install openocd
```

#### Windows
下载预编译包：https://gnutoolchains.com/arm-eabi/openocd/

### 4.3 启动调试服务器

```bash
.specify/scripts/debug/start-debug-server.sh --interface stlink-v2 --target stm32f4x
```

保持此终端窗口打开。

### 4.4 连接 GDB 调试器

在新终端窗口中：

```bash
.specify/scripts/debug/connect-gdb.sh build/bin/hello-c11.elf --load
```

GDB 启动后，程序已加载到 MCU。常用 GDB 命令：

```gdb
# 设置断点
(gdb) break main
Breakpoint 1 at 0x8000234: file main.c, line 15.

# 运行程序
(gdb) continue
Continuing.
Breakpoint 1, main () at main.c:15

# 单步执行
(gdb) next

# 查看变量
(gdb) print red
$1 = {r = 255 '\377', g = 0 '\000', b = 0 '\000', alpha = 255 '\377'}

# 继续执行
(gdb) continue

# 退出
(gdb) quit
```

---

## 第五步：优化和发布

### 5.1 发布版本构建

```bash
.specify/scripts/build/build.sh --config Release
```

或

```bash
cmake -B build-release -S my-first-project -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build-release
```

**构建类型对比**:

| 类型 | 优化级别 | 调试信息 | 适用场景 |
|------|---------|---------|---------|
| Debug | -O0 | 完整 | 开发调试 |
| Release | -O2 | 无 | 性能优先 |
| MinSizeRel | -Os | 无 | 代码大小优先 |

### 5.2 检查优化效果

```bash
arm-none-eabi-size build/bin/hello-c11.elf
arm-none-eabi-size build-release/bin/hello-c11.elf
```

通常 Release 版本代码大小减少 30-50%。

---

## 常见问题

### Q1: 工具链下载失败

**A**: 尝试使用国内镜像源：

```bash
.specify/scripts/setup/install-toolchain.sh --mirror tsinghua
```

### Q2: 构建时出现 "锁文件冲突"

**A**: 另一个构建进程正在运行。等待其完成或强制删除锁：

```bash
rm build/.build.lock
```

### Q3: 找不到链接器脚本

**A**: 链接器脚本（.ld 文件）需要根据具体 MCU 型号配置。可以从以下来源获取：
- STM32CubeMX 生成
- ST 官方示例代码
- 供应商 SDK

### Q4: OpenOCD 无法连接到调试器

**A**:
1. 检查调试器是否正确连接
2. 检查 USB 驱动是否安装（Windows）
3. 尝试以管理员权限运行
4. 检查设备管理器中是否识别到设备

### Q5: 如何支持其他 Cortex-M 型号？

**A**: 修改平台配置文件 `.specify/config/platform-cortex-m*.json` 和 CMake 工具链文件中的 CPU 标志。

---

## 下一步学习

1. **学习 C11 新特性**: 查看 C11 标准文档和示例代码
2. **优化代码**: 使用编译器优化选项和链接时优化（LTO）
3. **集成 RTOS**: 如 FreeRTOS、Zephyr 等
4. **添加外设驱动**: GPIO、UART、SPI、I2C 等
5. **单元测试**: 使用 Unity 或 Ceedling 框架

---

## 参考资源

- [ARM GCC 工具链文档](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain)
- [CMake 文档](https://cmake.org/documentation/)
- [OpenOCD 用户指南](http://openocd.org/doc/html/index.html)
- [C11 标准 (ISO/IEC 9899:2011)](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
- [STM32F4 参考手册](https://www.st.com/en/microcontrollers-microprocessors/stm32f4-series.html)

---

**反馈与支持**: 如有问题或建议，请提交 Issue 到项目仓库。
