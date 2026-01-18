# C11 嵌入式开发环境

为 ARM Cortex-M4 平台提供完整的 C11 标准开发环境。

## 特性

✅ **MVP 已完成** - User Story 1: 基础编译环境设置

- ✅ 自动下载和安装 ARM GCC 工具链
- ✅ SHA256 校验确保安全性
- ✅ 支持国内镜像源（清华、阿里云、中科大）
- ✅ C11 标准特性完整支持
- ✅ Cortex-M4 目标平台支持
- ✅ 跨平台兼容（Linux, macOS, Windows/Git Bash）
- ✅ 自动化测试套件

## 快速开始

### 前置要求

- **Linux/macOS**: Bash 4.0+, wget/curl, tar, jq
- **Windows**: Git Bash, wget/curl, tar, jq

### 1. 安装工具链

```bash
# 自动检测平台并安装
scripts/setup/install-toolchain.sh

# 或使用国内镜像源加速
scripts/setup/install-toolchain.sh --mirror tsinghua
```

### 2. 验证安装

```bash
scripts/setup/verify-toolchain.sh
```

### 3. 配置环境

```bash
source scripts/setup/configure-environment.sh
```

### 4. 测试编译

```bash
# 测试 C11 特性
arm-none-eabi-gcc -std=c11 -mcpu=cortex-m4 -mthumb --version
```

## 项目结构

```
.specify/
├── config/               # 配置文件
│   ├── toolchain.json           # 工具链配置
│   ├── mirrors.json             # 镜像源配置
│   └── platform-cortex-m4.json  # 平台配置
├── scripts/
│   ├── common.sh                # 通用函数库
│   └── setup/                   # 安装脚本
│       ├── install-toolchain.sh
│       ├── verify-toolchain.sh
│       └── configure-environment.sh
└── templates/            # 项目模板（待实现）

tests/
└── toolchain/           # 测试套件
    ├── test_install.bats
    └── test_compile.bats

.toolchain/              # 工具链安装目录（不提交）
└── gcc-arm-none-eabi/
```

## 运行测试

需要安装 [bats-core](https://github.com/bats-core/bats-core)：

```bash
# 测试安装脚本
bats tests/toolchain/test_install.bats

# 测试 C11 编译（需先安装工具链）
bats tests/toolchain/test_compile.bats
```

## 配置说明

### 工具链配置 (config/toolchain.json)

定义工具链版本、下载 URL 和 SHA256 校验值。支持：
- Linux x86_64
- macOS x86_64
- Windows (通过 Git Bash)

### 平台配置 (config/platform-cortex-m4.json)

定义 ARM Cortex-M4 特定的编译标志：
- CPU: cortex-m4
- FPU: fpv4-sp-d16 (硬浮点)
- C11 特性支持列表

### 镜像源配置 (config/mirrors.json)

国内镜像源优先级：
1. 清华大学开源镜像站
2. 阿里云开发者镜像站
3. 中国科学技术大学镜像站
99. 官方源（回退）

## 开发进度

### ✅ 已完成 (21/70 任务)

- [X] Phase 1: Setup (T001-T005) - 项目结构初始化
- [X] Phase 2: Foundational (T006-T010) - 基础配置和工具
- [X] Phase 3: User Story 1 (T011-T021) - 基础编译环境 **← MVP**

### 🔄 进行中

- [ ] Phase 4: User Story 2 (T022-T030) - 交叉编译配置
- [ ] Phase 5: User Story 3 (T031-T046) - 构建系统集成
- [ ] Phase 6: User Story 4 (T047-T057) - 调试环境配置
- [ ] Phase 7: Polish (T058-T070) - 示例和文档完善

详细任务列表：[specs/001-c11-embedded-env/tasks.md](specs/001-c11-embedded-env/tasks.md)

## 文档

- [功能规格](specs/001-c11-embedded-env/spec.md) - 用户故事和需求
- [实现计划](specs/001-c11-embedded-env/plan.md) - 技术架构和里程碑
- [数据模型](specs/001-c11-embedded-env/data-model.md) - 配置文件结构
- [脚本接口](specs/001-c11-embedded-env/contracts/script-interface.md) - API 规范
- [快速入门](specs/001-c11-embedded-env/quickstart.md) - 用户指南
- [技术研究](specs/001-c11-embedded-env/research.md) - 技术选型依据

## 故障排除

### 工具链下载失败

```bash
# 使用国内镜像源
scripts/setup/install-toolchain.sh --mirror tsinghua

# 查看详细错误信息
DEBUG=1 scripts/setup/install-toolchain.sh
```

### SHA256 校验失败

1. 检查网络连接
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
