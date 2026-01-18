# ARM GCC 工具链环境安装指南

本文档介绍如何使用 `scripts/setup/` 目录中的脚本来安装和配置 ARM Cortex-M4 开发环境。

## 📋 前置要求

### 必需工具

根据您的操作系统，需要安装以下工具：

**Linux / macOS:**
```bash
# 必需
- bash 4.0+
- wget 或 curl
- tar
- jq (JSON 处理工具)

# 安装 jq
# Ubuntu/Debian
sudo apt-get install jq

# macOS
brew install jq
```

**Windows (Git Bash):**
```bash
# 必需
- Git Bash (随 Git for Windows 安装)
- wget 或 curl (Git Bash 自带)
- tar (Git Bash 自带)
- unzip
- jq

# 安装 jq
curl -L https://github.com/jqlang/jq/releases/download/jq-1.7.1/jq-win64.exe -o /usr/bin/jq.exe
chmod +x /usr/bin/jq.exe
```

---

## 🚀 快速开始

完整的工具链安装流程包含三个步骤：

```bash
# 1. 安装工具链
./scripts/setup/install-toolchain.sh --mirror tsinghua

# 2. 验证安装
./scripts/setup/verify-toolchain.sh

# 3. 配置环境变量
source ./scripts/setup/configure-environment.sh
```

---

## 📦 详细步骤

### 步骤 1: 安装工具链

#### 脚本: `install-toolchain.sh`

自动下载、验证并安装 ARM GCC Embedded Toolchain。

#### 基本用法

```bash
# 自动检测平台并安装
./scripts/setup/install-toolchain.sh

# 使用国内镜像源加速（推荐）
./scripts/setup/install-toolchain.sh --mirror tsinghua

# 强制重新安装
./scripts/setup/install-toolchain.sh --force

# 预览安装操作（不实际下载）
./scripts/setup/install-toolchain.sh --dry-run
```

#### 完整选项

| 选项 | 说明 | 示例 |
|------|------|------|
| `--platform <PLATFORM>` | 指定平台 | `--platform linux-x86_64` |
| `--version <VERSION>` | 指定版本 | `--version 10.3-2021.10` |
| `--mirror <MIRROR>` | 使用镜像源 | `--mirror tsinghua` |
| `--force` | 强制重新安装 | |
| `--dry-run` | 仅预览，不执行 | |
| `--no-color` | 禁用彩色输出 | |
| `--help` | 显示帮助信息 | |

#### 支持的镜像源

| 镜像 | 说明 | 速度 |
|------|------|------|
| `tsinghua` | 清华大学开源镜像站 | ⚡⚡⚡ 快 |
| `aliyun` | 阿里云开发者镜像 | ⚡⚡ 较快 |
| `ustc` | 中国科大镜像站 | ⚡⚡ 较快 |
| `official` | ARM 官方源 | ⚡ 慢 |

#### 支持的平台

- `linux-x86_64` - Linux 64位
- `darwin-x86_64` - macOS Intel
- `darwin-aarch64` - macOS Apple Silicon
- `win32-x86_64` - Windows 64位

#### 安装位置

工具链将被安装到：
```
.toolchain/gcc-arm-none-eabi/
├── bin/              # 可执行文件
├── arm-none-eabi/    # 头文件和库
├── lib/              # GCC 库
└── share/            # 文档
```

#### 常见问题

**Q: 下载速度很慢？**
```bash
# 使用国内镜像源
./scripts/setup/install-toolchain.sh --mirror tsinghua
```

**Q: SHA256 校验失败？**
```bash
# 删除缓存重新下载
rm -rf /tmp/tmp.*
./scripts/setup/install-toolchain.sh
```

**Q: Windows 上提示找不到 jq？**
```bash
# 安装 jq
curl -L https://github.com/jqlang/jq/releases/download/jq-1.7.1/jq-win64.exe -o ~/jq.exe
chmod +x ~/jq.exe
mv ~/jq.exe /usr/bin/jq.exe
```

---

### 步骤 2: 验证安装

#### 脚本: `verify-toolchain.sh`

验证工具链是否正确安装，并测试 C11 和 Cortex-M4 支持。

#### 基本用法

```bash
# 标准验证
./scripts/setup/verify-toolchain.sh

# 详细输出（显示版本信息）
./scripts/setup/verify-toolchain.sh --verbose
```

#### 完整选项

| 选项 | 说明 |
|------|------|
| `--verbose` | 显示详细验证信息（包括版本） |
| `--no-color` | 禁用彩色输出 |
| `--help` | 显示帮助信息 |

#### 验证内容

脚本会检查以下内容：

1. **工具链可执行文件** (8 个)
   - ✅ `arm-none-eabi-gcc` - C 编译器
   - ✅ `arm-none-eabi-g++` - C++ 编译器
   - ✅ `arm-none-eabi-as` - 汇编器
   - ✅ `arm-none-eabi-ld` - 链接器
   - ✅ `arm-none-eabi-objcopy` - 对象文件转换
   - ✅ `arm-none-eabi-objdump` - 对象文件分析
   - ✅ `arm-none-eabi-size` - 代码大小分析
   - ✅ `arm-none-eabi-gdb` - 调试器

2. **C11 特性支持**
   - `_Static_assert` - 静态断言
   - 匿名结构体/联合体
   - `_Alignas` - 内存对齐
   - `_Generic` - 泛型选择

3. **Cortex-M4 支持**
   - `-mcpu=cortex-m4` 编译选项
   - `-mthumb` Thumb 指令集
   - 硬件浮点支持

#### 成功输出示例

```bash
[INFO] 验证工具链: /path/to/.toolchain/gcc-arm-none-eabi
[SUCCESS] gcc: arm-none-eabi-gcc.exe
[SUCCESS] g++: arm-none-eabi-g++.exe
[SUCCESS] as: arm-none-eabi-as.exe
[SUCCESS] ld: arm-none-eabi-ld.exe
[SUCCESS] objcopy: arm-none-eabi-objcopy.exe
[SUCCESS] objdump: arm-none-eabi-objdump.exe
[SUCCESS] size: arm-none-eabi-size.exe
[SUCCESS] gdb: arm-none-eabi-gdb.exe
[INFO] 测试 C11 特性支持...
[SUCCESS] C11 特性支持: 已启用
[INFO] 测试 Cortex-M4 支持...
[SUCCESS] Cortex-M4 支持: 已启用
[SUCCESS] 验证通过 ✓
```

#### 退出码

- `0` - 验证通过
- `1` - 工具链未安装
- `2` - 工具链版本不匹配
- `3` - 必要工具缺失或测试失败

---

### 步骤 3: 配置环境

#### 脚本: `configure-environment.sh`

配置 PATH 和其他环境变量，使工具链可以在当前 shell 中使用。

#### 基本用法

```bash
# 配置环境变量（必须使用 source）
source ./scripts/setup/configure-environment.sh

# 或使用点命令
. ./scripts/setup/configure-environment.sh
```

#### ⚠️ 重要提示

**必须使用 `source` 命令**，直接运行脚本不会生效：

```bash
# ✅ 正确
source ./scripts/setup/configure-environment.sh

# ❌ 错误 - 环境变量不会保留
./scripts/setup/configure-environment.sh
```

#### 配置内容

脚本会设置以下环境变量：

1. **PATH** - 添加工具链 bin 目录
   ```bash
   export PATH="$PROJECT_ROOT/.toolchain/gcc-arm-none-eabi/bin:$PATH"
   ```

2. **ARM_TOOLCHAIN_PATH** - 工具链根目录
   ```bash
   export ARM_TOOLCHAIN_PATH="$PROJECT_ROOT/.toolchain/gcc-arm-none-eabi"
   ```

#### 验证配置

配置完成后，验证环境变量：

```bash
# 检查 gcc 是否在 PATH 中
which arm-none-eabi-gcc

# 查看 gcc 版本
arm-none-eabi-gcc --version

# 查看工具链路径
echo $ARM_TOOLCHAIN_PATH
```

#### 预期输出

```bash
$ which arm-none-eabi-gcc
/path/to/.toolchain/gcc-arm-none-eabi/bin/arm-none-eabi-gcc

$ arm-none-eabi-gcc --version
arm-none-eabi-gcc (GNU Arm Embedded Toolchain 10.3-2021.10) 10.3.1
Copyright (C) 2020 Free Software Foundation, Inc.
```

#### 持久化配置

如果希望每次打开终端都自动配置，可以将命令添加到 shell 配置文件：

**Bash (~/.bashrc):**
```bash
# ARM GCC Toolchain
if [ -f ~/projects/embedded-py-runtime/scripts/setup/configure-environment.sh ]; then
    source ~/projects/embedded-py-runtime/scripts/setup/configure-environment.sh
fi
```

**Zsh (~/.zshrc):**
```bash
# ARM GCC Toolchain
[ -f ~/projects/embedded-py-runtime/scripts/setup/configure-environment.sh ] && \
    source ~/projects/embedded-py-runtime/scripts/setup/configure-environment.sh
```

---

## 🎯 完整示例

### 首次安装流程

```bash
# 1. 克隆项目（如果还没有）
cd ~/projects
git clone https://github.com/hollysys-cn/embedded-py-runtime.git
cd embedded-py-runtime

# 2. 安装工具链（使用清华镜像）
./scripts/setup/install-toolchain.sh --mirror tsinghua

# 输出示例：
# [INFO] 检测到平台: win32-x86_64
# [INFO] 工具链版本: 10.3-2021.10
# [INFO] 目标平台: win32-x86_64
# [INFO] 下载: https://developer.arm.com/.../gcc-arm-none-eabi-10.3-2021.10-win32.zip
# [SUCCESS] 下载完成
# [INFO] 验证 SHA256 校验和...
# [SUCCESS] 校验成功
# [INFO] 解压: /tmp/tmp.xxx/gcc-arm-none-eabi-10.3-2021.10-win32.zip
# [SUCCESS] 解压完成
# [SUCCESS] 工具链安装完成: /path/to/.toolchain/gcc-arm-none-eabi

# 3. 验证安装
./scripts/setup/verify-toolchain.sh --verbose

# 输出示例：
# [INFO] 验证工具链: /path/to/.toolchain/gcc-arm-none-eabi
# [SUCCESS] gcc: arm-none-eabi-gcc.exe (GNU Arm Embedded Toolchain 10.3-2021.10) 10.3.1
# [SUCCESS] g++: arm-none-eabi-g++.exe (GNU Arm Embedded Toolchain 10.3-2021.10) 10.3.1
# ...
# [SUCCESS] 验证通过 ✓

# 4. 配置环境
source ./scripts/setup/configure-environment.sh

# 输出示例：
# [INFO] 配置 ARM GCC 工具链环境
# [SUCCESS] 工具链路径: /path/to/.toolchain/gcc-arm-none-eabi
# [SUCCESS] 环境配置完成

# 5. 测试编译
arm-none-eabi-gcc --version
```

### 更新工具链

```bash
# 强制重新安装最新版本
./scripts/setup/install-toolchain.sh --force --mirror tsinghua

# 重新验证
./scripts/setup/verify-toolchain.sh

# 重新配置环境
source ./scripts/setup/configure-environment.sh
```

### 清理安装

```bash
# 删除工具链
rm -rf .toolchain/

# 重新安装
./scripts/setup/install-toolchain.sh --mirror tsinghua
```

---

## 🔧 故障排除

### 问题 1: 找不到 jq 命令

**错误信息:**
```
[ERROR] 缺少必需命令: jq
```

**解决方法:**
```bash
# Linux
sudo apt-get install jq

# macOS
brew install jq

# Windows Git Bash
curl -L https://github.com/jqlang/jq/releases/download/jq-1.7.1/jq-win64.exe -o /usr/bin/jq.exe
chmod +x /usr/bin/jq.exe
```

### 问题 2: 平台检测错误

**错误信息:**
```
[ERROR] 不支持的平台: xxx
```

**解决方法:**
```bash
# 手动指定平台
./scripts/setup/install-toolchain.sh --platform win32-x86_64
```

### 问题 3: SHA256 校验失败

**错误信息:**
```
[ERROR] SHA256 校验失败
```

**解决方法:**
```bash
# 删除临时下载文件
rm -rf /tmp/tmp.*

# 重新下载
./scripts/setup/install-toolchain.sh --mirror tsinghua
```

### 问题 4: 环境变量未生效

**症状:**
```bash
$ arm-none-eabi-gcc --version
bash: arm-none-eabi-gcc: command not found
```

**解决方法:**
```bash
# 确保使用 source 命令
source ./scripts/setup/configure-environment.sh

# 验证
which arm-none-eabi-gcc
```

### 问题 5: Windows 路径问题

**错误信息:**
```
[ERROR] 未找到或不可执行
```

**解决方法:**

配置文件已自动处理 Windows 的 `.exe` 扩展名。如果仍有问题：

```bash
# 检查文件是否存在
ls -la .toolchain/gcc-arm-none-eabi/bin/

# 手动测试
./.toolchain/gcc-arm-none-eabi/bin/arm-none-eabi-gcc.exe --version
```

---

## 📚 相关文档

- [快速开始指南](../README-c11-embedded.md) - 项目概览和快速入门
- [构建指南](./build-guide.md) - 如何编译嵌入式项目
- [调试指南](./debug-guide.md) - OpenOCD 和 GDB 使用
- [技术规格](../specs/001-c11-embedded-env/spec.md) - 完整的技术规格

---

## 🆘 获取帮助

如果遇到问题：

1. 查看脚本的 `--help` 输出
   ```bash
   ./scripts/setup/install-toolchain.sh --help
   ```

2. 使用 `--verbose` 选项获取详细信息
   ```bash
   ./scripts/setup/verify-toolchain.sh --verbose
   ```

3. 检查项目 Issues: https://github.com/hollysys-cn/embedded-py-runtime/issues

---

**最后更新**: 2026-01-18
**版本**: 1.0.0
