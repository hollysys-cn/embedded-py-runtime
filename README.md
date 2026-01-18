# Embedded Python Runtime

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/hollysys-cn/embedded-py-runtime)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![C11](https://img.shields.io/badge/C-11-blue)](docs/c11-embedded-environment.md)
[![ARM](https://img.shields.io/badge/ARM-Cortex--M4-orange)](docs/c11-embedded-environment.md)

为嵌入式系统提供完整的开发环境和运行时支持。

## 📋 项目概述

`embedded-py-runtime` 是一个面向嵌入式系统的综合开发环境项目，提供从底层 C11 开发环境到高级运行时特性的完整工具链支持。

## 🎯 功能模块

### ✅ C11 嵌入式开发环境

完整的 ARM Cortex-M4 C11 标准开发环境，支持自动化构建、增量编译和硬件调试。

- **工具链管理**: 自动下载、安装和验证 ARM GCC 工具链
- **交叉编译**: CMake 工具链配置和平台特定编译选项
- **构建系统**: 增量编译、构建锁、失败恢复
- **调试支持**: OpenOCD 和 GDB 集成

📖 **详细文档**: [docs/c11-embedded-environment.md](docs/c11-embedded-environment.md)

**快速开始**:
```bash
# 安装工具链
./scripts/setup/install-toolchain.sh --mirror tsinghua

# 验证安装
./scripts/setup/verify-toolchain.sh

# 配置环境
source ./scripts/setup/configure-environment.sh
```

### ✅ PLCopen 基础功能块库

符合 PLCopen 标准的工业控制功能块库，提供 7 个基础控制算法模块。

- **PID 控制器**: 微分项先行、条件积分法抗饱和、手自动切换
- **PT1 滤波器**: 一阶惯性滤波，信号平滑和噪声抑制
- **其他功能块**: RAMP、LIMIT、DEADBAND、INTEGRATOR、DERIVATIVE
- **工业级特性**: 完整数值保护、零动态内存、高性能（~3KB 代码）

📖 **详细文档**: [docs/plcopen-README.md](docs/plcopen-README.md)
📊 **构建报告**: [docs/BUILD_REPORT_002.md](docs/BUILD_REPORT_002.md)

**快速开始**:
```bash
# 配置环境（如未配置）
source ./scripts/setup/configure-environment.sh

# 构建库
./scripts/build/build-plcopen.sh

# 使用示例
# arm-none-eabi-gcc -o app.elf main.c build/libplcopen.a -lm
```

### 🔄 未来功能

更多功能模块正在开发中...

## 🚀 快速开始

### 环境要求

- **操作系统**: Linux (Ubuntu 20.04+) / macOS (11+) / Windows (Git Bash)
- **必需工具**: bash, wget/curl, tar, jq, cmake 3.20+
- **可选工具**: OpenOCD, bats-core

### 安装步骤

1. **克隆仓库**
   ```bash
   git clone https://github.com/hollysys-cn/embedded-py-runtime.git
   cd embedded-py-runtime
   ```

2. **选择功能模块**

   根据您的需求，参考对应的功能模块文档：
   - C11 开发环境: [docs/c11-embedded-environment.md](docs/c11-embedded-environment.md)

## 📁 项目结构

```
embedded-py-runtime/
├── config/                      # 配置文件
│   ├── toolchain.json           # 工具链配置
│   ├── mirrors.json             # 镜像源配置
│   └── platform-*.json          # 平台特定配置
├── scripts/                     # 脚本工具集
│   ├── setup/                   # 环境设置脚本
│   ├── build/                   # 构建脚本
│   └── debug/                   # 调试脚本
├── templates/                   # 项目模板
│   ├── cmake/                   # CMake 配置模板
│   └── examples/                # 示例项目
├── tests/                       # 测试套件
├── docs/                        # 文档目录
│   ├── c11-embedded-environment.md  # C11 环境详细文档
│   ├── toolchain-setup-guide.md     # 工具链设置指南
│   └── reports/                     # 项目报告
├── specs/                       # 功能规格文档
└── .toolchain/                  # 工具链安装目录（自动创建）
```

## 📚 文档导航

| 文档 | 说明 |
|------|------|
| [C11 嵌入式环境](docs/c11-embedded-environment.md) | ARM Cortex-M4 C11 开发环境完整指南 |
| [工具链设置指南](docs/toolchain-setup-guide.md) | 详细的工具链安装和配置说明 |
| [功能规格](specs/001-c11-embedded-env/) | C11 环境的完整技术规格和设计文档 |

## 🧪 测试

```bash
# 安装测试工具
npm install -g bats  # 或 brew install bats-core

# 运行所有测试
bats tests/**/*.bats

# 运行特定模块测试
bats tests/toolchain/*.bats     # 工具链测试
bats tests/build/*.bats         # 构建系统测试
bats tests/integration/*.bats   # 集成测试
```

## 🛠️ 常见问题

### 工具链下载缓慢？

使用国内镜像源：
```bash
./scripts/setup/install-toolchain.sh --mirror tsinghua
# 可用镜像: tsinghua, aliyun, ustc
```

### Windows 环境支持？

本项目在 Windows 上需要 Git Bash 环境：
1. 安装 [Git for Windows](https://git-scm.com/download/win)
2. 使用 Git Bash 运行所有脚本

### 更多问题？

查看对应功能模块的详细文档或提交 [Issue](https://github.com/hollysys-cn/embedded-py-runtime/issues)。

## 🤝 贡献

欢迎贡献代码、文档或提出建议！

1. Fork 本仓库
2. 创建特性分支: `git checkout -b feature/your-feature`
3. 提交更改: `git commit -am 'Add some feature'`
4. 推送分支: `git push origin feature/your-feature`
5. 提交 Pull Request

## 📄 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

使用的第三方工具和依赖遵循各自的许可证：
- ARM GCC Embedded: GPL
- CMake: BSD 3-Clause
- OpenOCD: GPL

## 🔗 相关资源

- [ARM 官方文档](https://developer.arm.com/)
- [CMake 文档](https://cmake.org/documentation/)
- [OpenOCD 文档](http://openocd.org/doc/)

---

**项目状态**: 积极维护中 | **最后更新**: 2026-01-18
