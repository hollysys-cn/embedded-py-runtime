# C11 嵌入式开发环境

![Build](https://img.shields.io/badge/build-passing-brightgreen) ![C11](https://img.shields.io/badge/C-11-blue) ![ARM](https://img.shields.io/badge/ARM-Cortex--M4-orange) ![License](https://img.shields.io/badge/license-MIT-green)

为 ARM Cortex-M4 平台提供完整的 C11 标准开发环境，支持自动化构建、增量编译和硬件调试。

## 📋 特性概览

### ✅ 完整的开发工具链

- ✅ **User Story 1**: 基础编译环境设置
  - 自动下载和安装 ARM GCC 工具链
  - SHA256 校验确保安全性
  - 支持国内镜像源（清华、阿里云、中科大）
  - C11 标准特性完整支持
  - Cortex-M4 目标平台支持

- ✅ **User Story 2**: 交叉编译配置
  - CMake 工具链文件
  - 平台特定编译选项
  - 自动生成 HEX 和 BIN 文件
  - 代码大小分析

- ✅ **User Story 3**: 构建系统集成
  - 自动化构建脚本
  - 增量编译支持
  - 构建锁机制防止并发冲突
  - 失败恢复和错误日志

- ✅ **User Story 4**: 调试环境配置
  - OpenOCD 调试服务器
  - GDB 源码级调试
  - 多种调试接口支持（ST-Link, J-Link）

### 🎯 跨平台兼容

- Linux (Ubuntu 20.04+)
- macOS (11+)
- Windows (Git Bash/WSL)

### 🧪 完整的测试套件

- 工具链安装测试
- 编译功能测试
- 增量构建测试
- 失败恢复测试
- 端到端集成测试

## 🚀 快速开始

### 前置要求

**必需工具**:
- **Linux/macOS**: Bash 4.0+, wget/curl, tar, jq, cmake 3.20+
- **Windows**: Git Bash, wget/curl, tar, jq, cmake 3.20+

**可选工具**:
- OpenOCD 或 pyOCD（用于硬件调试）
- bats-core（用于运行测试套件）

### 步骤 1: 安装工具链

```bash
# 自动检测平台并安装
scripts/setup/install-toolchain.sh

# 或使用国内镜像源加速下载
scripts/setup/install-toolchain.sh --mirror tsinghua

# 支持的镜像源: tsinghua, aliyun, ustc
```

### 步骤 2: 验证安装

```bash
# 验证工具链和 C11 特性
scripts/setup/verify-toolchain.sh

# 应该显示：
# ✓ 工具链已安装
# ✓ C11 特性支持
# ✓ Cortex-M4 支持
```

### 步骤 3: 配置环境

```bash
# 配置环境变量
source scripts/setup/configure-environment.sh

# 验证配置
arm-none-eabi-gcc --version
```

### 步骤 4: 构建示例项目

```bash
# 构建 hello-c11 示例
cd templates/examples/hello-c11
cp ../../cmake/toolchain-arm-cortex-m4.cmake .
../../../scripts/build/build.sh --config Debug

# 或构建 blinky LED 示例
cd templates/examples/blinky
cp ../../cmake/toolchain-arm-cortex-m4.cmake .
../../../scripts/build/build.sh --config Release
```

### 步骤 5: 调试（可选）

```bash
# 启动调试服务器（需要硬件连接）
scripts/debug/start-debug-server.sh --interface stlink-v2 --target stm32f4x

# 在另一个终端连接 GDB
scripts/debug/connect-gdb.sh --load build/your_program.elf
```

## 📁 项目结构

```
embedded-py-runtime/
├── config/                      # 配置文件
│   ├── toolchain.json           # 工具链配置（版本、URL、SHA256）
│   ├── mirrors.json             # 国内镜像源配置
│   ├── platform-cortex-m4.json  # Cortex-M4 编译选项
│   └── debug-openocd.json       # OpenOCD 调试配置
│
├── scripts/                     # 脚本工具集
│   ├── common.sh                # 通用函数库
│   ├── setup/                   # 环境设置
│   │   ├── install-toolchain.sh
│   │   ├── verify-toolchain.sh
│   │   └── configure-environment.sh
│   ├── build/                   # 构建脚本
│   │   ├── build.sh             # 主构建脚本
│   │   └── clean.sh             # 清理脚本
│   └── debug/                   # 调试脚本
│       ├── start-debug-server.sh
│       └── connect-gdb.sh
│
├── templates/                   # 项目模板
│   ├── cmake/                   # CMake 配置
│   │   ├── toolchain-arm-cortex-m4.cmake
│   │   └── CMakeLists.txt.template
│   └── examples/                # 示例项目
│       ├── hello-c11/           # C11 特性演示
│       └── blinky/              # LED 闪烁（含启动代码）
│
├── tests/                       # 测试套件
│   ├── toolchain/               # 工具链测试
│   ├── build/                   # 构建测试
│   ├── debug/                   # 调试测试
│   └── integration/             # 集成测试
│
├── .toolchain/                  # 工具链安装目录（gitignore）
│   └── gcc-arm-none-eabi/
│
└── build/                       # 构建输出目录（gitignore）
```

## 🧪 运行测试

需要安装 [bats-core](https://github.com/bats-core/bats-core)：

```bash
# 安装 bats-core (Linux/macOS)
npm install -g bats
# 或
brew install bats-core

# 运行所有测试
bats tests/**/*.bats

# 运行特定测试套件
bats tests/toolchain/test_install.bats     # 工具链安装测试
bats tests/toolchain/test_compile.bats     # C11 编译测试
bats tests/build/test_build.bats           # 构建系统测试
bats tests/build/test_incremental.bats     # 增量编译测试
bats tests/build/test_recovery.bats        # 失败恢复测试
bats tests/debug/test_debug_server.bats    # 调试服务器测试
bats tests/integration/test_full_workflow.bats  # 端到端测试
```

**注意**: 大多数测试需要先安装工具链。运行 `scripts/setup/install-toolchain.sh` 后再执行测试。

## ⚙️ 配置说明

### 工具链配置 (config/toolchain.json)

定义工具链版本、下载 URL 和 SHA256 校验值。支持：
- Linux x86_64
- macOS x86_64 / arm64
- Windows x86_64 (通过 Git Bash)

```json
{
  "name": "gcc-arm-none-eabi",
  "version": "10.3-2021.10",
  "install_dir": ".toolchain/gcc-arm-none-eabi",
  "platforms": {
    "linux-x86_64": {
      "url": "https://developer.arm.com/-/media/Files/downloads/...",
      "archive": "gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2",
      "sha256": "..."
    }
  }
}
```

### 镜像源配置 (config/mirrors.json)

国内加速镜像源，优先使用清华大学镜像：

```json
{
  "mirrors": {
    "tsinghua": {
      "name": "清华大学开源镜像站",
      "base_url": "https://mirrors.tuna.tsinghua.edu.cn/armbian-releases/_toolchain",
      "priority": 1
    },
    "aliyun": {...},
    "ustc": {...}
  }
}
```

### 平台配置 (config/platform-cortex-m4.json)

定义 ARM Cortex-M4 特定的编译标志：

```json
{
  "platform": "arm-cortex-m4",
  "compiler_flags": {
    "cpu": "-mcpu=cortex-m4",
    "thumb": "-mthumb",
    "fpu": "-mfpu=fpv4-sp-d16",
    "float_abi": "-mfloat-abi=hard"
  },
  "c11_features": [
    "_Static_assert",
    "anonymous_structures",
    "generic_selection"
  ]
}
```

### 调试配置 (config/debug-openocd.json)

OpenOCD 调试接口和目标配置：

```json
{
  "interfaces": {
    "stlink-v2": "interface/stlink-v2.cfg",
    "jlink": "interface/jlink.cfg",
    "cmsis-dap": "interface/cmsis-dap.cfg"
  },
  "targets": {
    "stm32f4x": "target/stm32f4x.cfg",
    "stm32f7x": "target/stm32f7x.cfg"
  },
  "default_port": 3333
}
```

## 📊 开发进度

### ✅ 已完成 (70/70 任务 - 100%)

- ✅ **Phase 1**: Setup (T001-T005) - 项目结构初始化
- ✅ **Phase 2**: Foundational (T006-T010) - 基础配置和工具
- ✅ **Phase 3**: User Story 1 (T011-T021) - 基础编译环境 **[MVP]**
- ✅ **Phase 4**: User Story 2 (T022-T030) - 交叉编译配置
- ✅ **Phase 5**: User Story 3 (T031-T043) - 构建系统集成
- ✅ **Phase 6**: User Story 4 (T047-T056) - 调试环境配置
- ✅ **Phase 7**: Polish (T058-T066) - 示例和文档完善
- ✅ **测试套件** (T044-T046, T057, T067) - 完整测试覆盖

### 🎯 完成的用户故事

1. ✅ **US1**: 基础编译环境设置 - 自动安装和验证 ARM GCC 工具链
2. ✅ **US2**: 交叉编译配置 - CMake 工具链和平台特定选项
3. ✅ **US3**: 构建系统集成 - 增量编译和失败恢复
4. ✅ **US4**: 调试环境配置 - OpenOCD 和 GDB 集成

详细任务列表：[specs/001-c11-embedded-env/tasks.md](specs/001-c11-embedded-env/tasks.md)

## 📚 文档

完整的开发文档位于 `specs/001-c11-embedded-env/` 目录：

| 文档 | 说明 |
|------|------|
| [spec.md](specs/001-c11-embedded-env/spec.md) | 功能规格 - 用户故事和需求 |
| [plan.md](specs/001-c11-embedded-env/plan.md) | 实现计划 - 技术架构和里程碑 |
| [tasks.md](specs/001-c11-embedded-env/tasks.md) | 任务清单 - 详细的任务分解 |
| [data-model.md](specs/001-c11-embedded-env/data-model.md) | 数据模型 - 配置文件结构 |
| [contracts/](specs/001-c11-embedded-env/contracts/) | 接口规范 - API 文档 |
| [quickstart.md](specs/001-c11-embedded-env/quickstart.md) | 快速入门 - 用户指南 |
| [research.md](specs/001-c11-embedded-env/research.md) | 技术研究 - 技术选型依据 |

## 🛠️ 常见问题

### Q: 工具链下载失败怎么办？

```bash
# 方法 1: 使用国内镜像源
scripts/setup/install-toolchain.sh --mirror tsinghua

# 方法 2: 手动指定镜像
scripts/setup/install-toolchain.sh --mirror aliyun

# 方法 3: 查看详细错误信息
DEBUG=1 scripts/setup/install-toolchain.sh
```

### Q: SHA256 校验失败？

1. 检查网络连接是否稳定
2. 删除不完整的下载文件：`rm -rf .toolchain/downloads/*`
3. 重新下载：`scripts/setup/install-toolchain.sh`
4. 如果持续失败，更换镜像源

### Q: 构建失败提示找不到工具链？

```bash
# 1. 验证工具链是否正确安装
scripts/setup/verify-toolchain.sh

# 2. 重新配置环境变量
source scripts/setup/configure-environment.sh

# 3. 检查 PATH
echo $PATH | grep arm-none-eabi
```

### Q: 如何在 Windows 上使用？

本项目需要 **Git Bash** 环境：

1. 安装 [Git for Windows](https://git-scm.com/download/win)
2. 使用 Git Bash 运行所有脚本
3. 确保安装了必需的工具（wget, tar, jq）

**WSL 用户**: 也可以在 WSL (Ubuntu) 环境下使用。

### Q: 增量编译不工作？

```bash
# 完全清理并重新构建
scripts/build/clean.sh --all
scripts/build/build.sh --config Debug

# 检查构建状态文件
cat build/.build-state.json
```

### Q: 调试器连接失败？

1. 检查硬件连接（ST-Link/J-Link）
2. 确认 OpenOCD 已安装：`which openocd`
3. 验证接口和目标配置：
   ```bash
   scripts/debug/start-debug-server.sh --help
   ```
4. 查看 OpenOCD 日志以获取详细错误

### Q: 如何添加新的目标平台？

1. 在 `config/platform-*.json` 添加平台配置
2. 更新 CMake 工具链文件
3. 测试编译和链接选项

## 🤝 贡献

欢迎贡献！请遵循以下步骤：

1. Fork 本仓库
2. 创建特性分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -am 'Add some feature'`
4. 推送到分支：`git push origin feature/your-feature`
5. 提交 Pull Request

## 📄 许可证

本项目使用的工具链和依赖：
- **ARM GCC Embedded**: GPL 许可证
- **CMake**: BSD 3-Clause 许可证
- **OpenOCD**: GPL 许可证

项目脚本和配置文件：MIT 许可证

## 🔗 相关资源

- [ARM GCC Toolchain 官方文档](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
- [CMake 文档](https://cmake.org/documentation/)
- [OpenOCD 用户指南](http://openocd.org/doc/html/index.html)
- [ARM Cortex-M4 技术参考手册](https://developer.arm.com/documentation/100166/0001)
- [C11 标准文档](https://www.iso.org/standard/57853.html)

---

**完成度**: 70/70 任务 (100%) | **状态**: ✅ 生产就绪
2. 尝试不同的镜像源
3. 手动下载并验证

### Windows 兼容性问题

- 确保使用 Git Bash 而非 PowerShell
- 安装 GNU 工具：`pacman -S wget tar`（在 Git Bash 中）

## 许可证

本项目遵循 [项目根目录许可证]。

工具链组件许可证：
- ARM GCC: GPL v3
- Newlib: BSD-like licenses
- CMake: BSD 3-Clause

## 贡献

查看 [specs/001-c11-embedded-env/tasks.md](specs/001-c11-embedded-env/tasks.md) 了解待实现功能。

## 更新日志

### 2026-01-18 - MVP 版本

- ✅ 实现工具链自动安装和验证
- ✅ 支持 C11 标准和 Cortex-M4 平台
- ✅ 国内镜像源加速
- ✅ 跨平台支持和自动化测试
