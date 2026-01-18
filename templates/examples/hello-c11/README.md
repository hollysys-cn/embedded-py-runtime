# hello-c11 Example

C11 语言特性演示程序，用于验证 ARM Cortex-M4 C11 编译环境。

## C11 特性演示

此示例程序展示以下 C11 特性：

1. **`_Static_assert`** - 编译时断言
   - 验证类型大小
   - 验证结构体对齐

2. **`_Generic`** - 泛型宏
   - 类型安全的泛型函数
   - 自动类型选择

3. **`_Alignas`** - 内存对齐
   - DMA 缓冲区对齐
   - 性能优化

4. **匿名结构体/联合体**
   - 寄存器位字段定义
   - 嵌入式开发常用

5. **复合字面量**
   - 简化结构体初始化
   - 减少中间变量

## 构建

### 前提条件

- 已安装 ARM 工具链
- 已配置 CMake 工具链文件

### 构建步骤

```bash
# 1. 进入示例目录
cd templates/examples/hello-c11

# 2. 配置 CMake
cmake -B build -DCMAKE_TOOLCHAIN_FILE=../../../../templates/cmake/toolchain-arm-cortex-m4.cmake

# 3. 编译
cmake --build build

# 4. 查看生成的文件
ls -lh build/
```

## 生成的文件

- `hello-c11.elf` - ELF 可执行文件（包含调试符号）
- `hello-c11.hex` - Intel HEX 格式（用于烧写）
- `hello-c11.bin` - 纯二进制格式
- `hello-c11.map` - 链接器映射文件

## 验证 C11 支持

```bash
# 查看 ELF 文件信息
arm-none-eabi-objdump -h build/hello-c11.elf

# 验证架构
file build/hello-c11.elf
# 输出应包含: ARM, version 1 (SYSV), ... Cortex-M4

# 查看代码大小
arm-none-eabi-size build/hello-c11.elf
```

## 注意事项

此示例使用默认链接选项，**不包含**：
- 硬件初始化代码
- 中断向量表
- 启动汇编文件

如需在实际硬件上运行，请参考 `blinky` 示例，它包含完整的启动代码和链接脚本。

## 学习资源

- C11 标准: ISO/IEC 9899:2011
- ARM Cortex-M4 手册: [ARM Developer](https://developer.arm.com/documentation/)
- GCC ARM Embedded: [ARM GNU Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain)
