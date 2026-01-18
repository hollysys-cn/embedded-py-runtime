# Implementation Plan: 构建 C11 嵌入式开发环境

**Branch**: `001-c11-embedded-env` | **Date**: 2026-01-18 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `/specs/001-c11-embedded-env/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

本功能旨在为 ARM Cortex-M4 嵌入式平台构建一个完整的 C11 标准开发环境。包括自动获取和配置交叉编译工具链（ARM GCC Embedded）、建立自动化构建系统（支持增量编译和失败恢复）、配置调试环境。技术方法采用 Shell/Python 脚本进行工具链管理，CMake 或 Make 作为构建系统，并集成 GDB 进行调试。

## Technical Context

**Language/Version**: Shell Script (Bash 4.0+), Python 3.8+ (用于工具链管理脚本)
**Primary Dependencies**:
- ARM GCC Embedded Toolchain 10.3+ (交叉编译器)
- CMake 3.20+ 或 GNU Make 4.0+ (构建系统)
- GDB (ARM 版本, 调试器)
- OpenOCD 或 pyOCD (调试服务器, 用于 JTAG/SWD 连接)

**Storage**: 文件系统 (工具链安装、构建产物、配置文件)
**Testing**:
- 脚本功能测试 (Shell 脚本测试框架 bats-core)
- 编译测试 (编译 C11 测试程序验证工具链)
- 集成测试 (完整的构建-调试流程测试)

**Target Platform**:
- 主机平台: Windows 10+, Linux (Ubuntu 20.04+), macOS 11+
- 目标嵌入式平台: ARM Cortex-M4 (STM32F4 系列 MCU)

**Project Type**: 工具链项目 (开发环境配置工具集)
**Performance Goals**:
- 环境设置时间: <30 分钟 (包括工具链下载)
- 中型项目完整构建时间: <2 分钟 (100 个源文件)
- 增量构建时间: <10 秒 (单文件修改)

**Constraints**:
- 跨平台兼容性 (Windows/Linux/macOS 一致的体验)
- 不污染系统环境 (工具链安装在项目目录内)
- 支持离线使用 (工具链下载后可离线开发)
- 国内网络优化 (优先使用国内镜像源)

**Scale/Scope**:
- 支持项目规模: 50-1000 个源文件
- 支持并发开发人数: 1-10 人 (通过锁机制保证构建一致性)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

基于 `.specify/memory/constitution.md` 进行检查：

- ✅ **本土化与语言**: 所有文档、脚本注释、用户提示信息均使用简体中文
- ✅ **架构约束**: 保持项目现有结构，在 `.specify/` 目录下添加工具脚本，不修改根目录结构
- ✅ **环境与配置**: 工具链安装在项目 `.toolchain/` 目录，不污染系统环境
- ✅ **工程实践**: 脚本兼容 Windows/Linux/macOS，提供一致开发体验
- ✅ **依赖管理**: 优先使用国内镜像源 (阿里云镜像)，新依赖需人工确认
- ✅ **许可合规**: ARM GCC (GPL), CMake (BSD), OpenOCD (GPL) - 均为工具，不影响用户代码许可
- ✅ **代码质量**: 脚本包含注释，使用 UTF-8 编码， LF 换行符
- ✅ **修改范围**: 只添加新的工具脚本和配置，不修改现有代码

**结论**: 通过所有章程检查。无需记录复杂度违例。

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature]/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)

```text
.specify/
├── scripts/
│   ├── setup/
│   │   ├── install-toolchain.sh       # 自动下载和安装 ARM GCC
│   │   ├── verify-toolchain.sh        # 验证工具链安装
│   │   └── configure-environment.sh   # 配置开发环境变量
│   ├── build/
│   │   ├── build.sh                   # 构建主脚本
│   │   ├── clean.sh                   # 清理构建产物
│   │   └── lock-manager.sh            # 构建锁管理
│   └── debug/
│       ├── start-debug-server.sh      # 启动 OpenOCD 调试服务器
│       └── connect-gdb.sh             # 连接 GDB 调试器
├── templates/
│   ├── cmake/
│   │   ├── CMakeLists.txt.template    # CMake 项目模板
│   │   └── toolchain-arm-cortex-m4.cmake  # 工具链配置文件
│   └── examples/
│       ├── hello-c11/                 # C11 示例项目
│       │   ├── main.c
│       │   └── CMakeLists.txt
│       └── blinky/                    # LED 闪烁示例（嵌入式经典）
│           ├── main.c
│           ├── startup.s
│           ├── linker.ld
│           └── CMakeLists.txt
└── config/
    ├── toolchain.json             # 工具链配置 (版本、下载源、SHA256)
    ├── platform-cortex-m4.json   # 目标平台配置
    └── mirrors.json               # 国内镜像源配置

.toolchain/                        # 工具链安装目录 (不提交到 git)
├── arm-none-eabi-gcc/
└── openocd/

build/                             # 构建输出目录 (不提交到 git)
├── .build.lock                    # 构建锁文件
├── obj/                           # 目标文件
├── bin/                           # 可执行文件
└── logs/                          # 构建日志

tests/
├── toolchain/                     # 工具链功能测试
│   ├── test_install.bats
│   └── test_compile.bats
├── build/                         # 构建系统测试
│   ├── test_incremental.bats
│   └── test_recovery.bats
└── integration/                   # 集成测试
    └── test_full_workflow.bats
```

**Structure Decision**: 采用工具链项目结构。核心是 `.specify/` 目录下的脚本和模板，不修改项目根目录的现有结构。工具链安装在 `.toolchain/` 目录，构建输出在 `build/` 目录，两者均不提交到版本控制。

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |

---

## Phase 0: Outline & Research ✅ COMPLETED

**输出**: [research.md](research.md)

**研究任务**:
1. ✅ ARM Cortex-M4 工具链选择 → ARM GNU Toolchain 10.3+
2. ✅ 构建系统选择 → CMake 3.20+
3. ✅ 调试环境配置 → GDB + OpenOCD
4. ✅ C11 标准库支持 → Newlib (ARM GCC 内置)
5. ✅ 工具链自动获取策略 → Shell/Python 脚本 + SHA256 校验
6. ✅ 构建失败恢复机制 → 智能清理 + 锁文件保护
7. ✅ 跨平台兼容性 → Bash + PowerShell 双版本脚本

**结论**: 所有技术选型明确，无遗留 NEEDS CLARIFICATION。

---

## Phase 1: Design & Contracts ✅ COMPLETED

**输出**:
- ✅ [data-model.md](data-model.md) - 定义配置文件结构和数据模型
- ✅ [contracts/script-interface.md](contracts/script-interface.md) - 定义脚本接口契约
- ✅ [quickstart.md](quickstart.md) - 用户快速入门指南

**设计决策**:

### 数据模型
- **ToolchainConfig**: 工具链元数据（版本、下载 URL、SHA256）
- **PlatformConfig**: 目标平台配置（CPU 标志、内存布局、C11 特性支持）
- **BuildConfig**: 构建配置（源文件、编译选项、链接脚本）
- **BuildState**: 构建状态跟踪（用于增量构建和错误恢复）
- **DebugConfig**: 调试会话配置（OpenOCD 接口、GDB 端口）

### 脚本接口契约
定义了 7 个核心脚本的接口规范：
1. `install-toolchain.sh` - 工具链安装
2. `verify-toolchain.sh` - 工具链验证
3. `configure-environment.sh` - 环境配置
4. `build.sh` - 项目构建
5. `clean.sh` - 清理构建产物
6. `start-debug-server.sh` - 启动调试服务器
7. `connect-gdb.sh` - GDB 连接

每个脚本都有明确的：
- 命令行选项
- 退出码定义
- 输入/输出格式
- 前置/后置条件

### Agent Context 更新
✅ 已更新 GitHub Copilot context 文件，添加：
- Shell Script (Bash 4.0+), Python 3.8+ (工具链管理脚本)
- 文件系统存储（工具链安装、构建产物、配置文件）
- 工具链项目类型（开发环境配置工具集）

---

## Phase 2: Implementation Roadmap

**注意**: 详细的任务列表将由 `/speckit.tasks` 命令生成到 `tasks.md`

### 高层实现顺序

#### 里程碑 1: 工具链管理 (P1)
**目标**: 实现工具链的自动获取、验证和安装

**关键交付物**:
- `.specify/config/toolchain.json` - 工具链配置文件
- `.specify/config/mirrors.json` - 国内镜像源配置
- `.specify/scripts/setup/install-toolchain.sh` - 安装脚本
- `.specify/scripts/setup/verify-toolchain.sh` - 验证脚本
- `tests/toolchain/` - 工具链测试

**验收标准**:
- 能在 Windows/Linux/macOS 上自动下载并安装 ARM GCC
- SHA256 校验通过
- 验证脚本能检测 C11 和 Cortex-M4 支持
- 支持国内镜像源（清华、阿里云、中科大）

**依赖**: 无

---

#### 里程碑 2: 环境配置 (P1)
**目标**: 配置开发环境变量和 PATH

**关键交付物**:
- `.specify/scripts/setup/configure-environment.sh` - 环境配置脚本
- `.specify/templates/cmake/toolchain-arm-cortex-m4.cmake` - CMake 工具链文件
- `.specify/config/platform-cortex-m4.json` - 平台配置

**验收标准**:
- 工具链添加到 PATH
- 环境变量正确设置
- CMake 能找到交叉编译工具链
- 跨平台一致性

**依赖**: 里程碑 1

---

#### 里程碑 3: 构建系统 (P1)
**目标**: 实现自动化构建流程

**关键交付物**:
- `.specify/scripts/build/build.sh` - 构建主脚本
- `.specify/scripts/build/lock-manager.sh` - 构建锁管理
- `.specify/templates/cmake/CMakeLists.txt.template` - CMake 模板
- `tests/build/` - 构建系统测试

**验收标准**:
- 支持 Debug/Release/MinSizeRel 构建配置
- 增量编译正常工作
- 构建锁机制防止并发冲突
- 生成 ELF/HEX/BIN 文件
- 显示代码大小统计

**依赖**: 里程碑 2

---

#### 里程碑 4: 构建失败恢复 (P2)
**目标**: 处理构建失败和恢复

**关键交付物**:
- `.specify/scripts/build/clean.sh` - 清理脚本
- 构建状态跟踪（build/.build-state.json）
- 错误日志保存
- `tests/build/test_recovery.bats` - 恢复测试

**验收标准**:
- 构建失败时自动清理中间文件
- 保留详细错误日志
- 支持强制完整重建
- 过期锁文件自动清理

**依赖**: 里程碑 3

---

#### 里程碑 5: 调试环境 (P3)
**目标**: 配置调试工具和流程

**关键交付物**:
- `.specify/config/debug-openocd.json` - 调试配置
- `.specify/scripts/debug/start-debug-server.sh` - OpenOCD 启动脚本
- `.specify/scripts/debug/connect-gdb.sh` - GDB 连接脚本
- `tests/debug/` - 调试功能测试

**验收标准**:
- OpenOCD 能连接 ST-Link/V2
- GDB 能连接到 OpenOCD
- 支持源码级调试
- 断点和变量查看正常工作

**依赖**: 里程碑 3

---

#### 里程碑 6: 示例和文档 (P3)
**目标**: 提供示例项目和完整文档

**关键交付物**:
- `.specify/templates/examples/hello-c11/` - C11 示例
- `.specify/templates/examples/blinky/` - LED 闪烁示例
- `quickstart.md` - 快速入门指南 ✅ 已完成
- README 更新

**验收标准**:
- 示例项目能成功编译
- 快速入门指南完整且易懂
- 覆盖常见问题和故障排除
- 提供参考资源链接

**依赖**: 里程碑 3, 里程碑 5

---

#### 里程碑 7: 测试覆盖 (P4)
**目标**: 完善自动化测试

**关键交付物**:
- 完整的 bats 测试套件
- 集成测试（完整工作流）
- CI 配置（可选）

**验收标准**:
- 测试覆盖率 >80%
- 所有关键路径有测试
- 跨平台测试通过
- 测试可在 CI 中自动运行

**依赖**: 所有其他里程碑

---

### 技术栈总结

| 层次 | 技术 | 用途 |
|------|------|------|
| 编译器 | ARM GCC 10.3+ | C11 交叉编译 |
| 构建系统 | CMake 3.20+ | 项目配置和构建 |
| 调试器 | GDB + OpenOCD | 源码级调试 |
| 标准库 | Newlib | C11 标准库实现 |
| 脚本 | Bash 4.0+, Python 3.8+ | 自动化工具 |
| 测试 | bats-core 1.5+ | Shell 脚本测试 |
| 配置 | JSON | 结构化配置存储 |

---

## Next Steps

### 立即行动

1. ✅ Phase 0 完成: 研究文档已生成
2. ✅ Phase 1 完成: 数据模型、契约和快速入门已生成
3. ⏭️ **执行 `/speckit.tasks`**: 生成详细的任务列表到 `tasks.md`
4. ��� **开始实现**: 按照里程碑顺序逐步实现

### 验证步骤

完成实现后，验证以下场景：

#### 场景 1: 全新安装
```bash
# 克隆项目
git clone <repo> && cd <repo>

# 安装工具链
.specify/scripts/setup/install-toolchain.sh --mirror tsinghua

# 验证
.specify/scripts/setup/verify-toolchain.sh

# 配置环境
source .specify/scripts/setup/configure-environment.sh

# 构建示例
.specify/scripts/build/build.sh

# 预期: 30 分钟内完成，示例成功编译
```

#### 场景 2: 增量构建
```bash
# 修改源文件
echo "// comment" >> my-project/src/main.c

# 重新构建
.specify/scripts/build/build.sh

# 预期: <10 秒完成增量构建
```

#### 场景 3: 构建失败恢复
```bash
# 引入编译错误
echo "invalid C code" >> my-project/src/main.c

# 尝试构建
.specify/scripts/build/build.sh  # 失败

# 修复错误
git checkout my-project/src/main.c

# 重新构建
.specify/scripts/build/build.sh  # 成功

# 预期: 自动清理，成功恢复
```

#### 场景 4: 调试
```bash
# 启动调试服务器
.specify/scripts/debug/start-debug-server.sh &

# 连接 GDB
.specify/scripts/debug/connect-gdb.sh build/bin/firmware.elf --load

# 预期: 成功连接并加载程序
```

---

## 风险与缓解

| 风险 | 影响 | 可能性 | 缓解措施 |
|------|------|--------|---------|
| 工具链下载失败（网络问题） | 高 | 中 | 提供多个镜像源，支持离线安装 |
| Windows 平台兼容性问题 | 中 | 中 | 提供 PowerShell 版本，充分测试 |
| 平台差异导致脚本失败 | 中 | 低 | 使用跨平台工具，编写防御性代码 |
| 调试器驱动问题 | 低 | 中 | 提供详细文档和故障排除指南 |
| C11 特性在嵌入式平台受限 | 低 | 高 | 文档明确说明限制，提供替代方案 |

---

## 附录: 文件清单

### 配置文件
- `.specify/config/toolchain.json`
- `.specify/config/platform-cortex-m4.json`
- `.specify/config/mirrors.json`
- `.specify/config/debug-openocd.json`

### 脚本文件
- `.specify/scripts/setup/install-toolchain.sh`
- `.specify/scripts/setup/verify-toolchain.sh`
- `.specify/scripts/setup/configure-environment.sh`
- `.specify/scripts/build/build.sh`
- `.specify/scripts/build/clean.sh`
- `.specify/scripts/build/lock-manager.sh`
- `.specify/scripts/debug/start-debug-server.sh`
- `.specify/scripts/debug/connect-gdb.sh`

### 模板文件
- `.specify/templates/cmake/CMakeLists.txt.template`
- `.specify/templates/cmake/toolchain-arm-cortex-m4.cmake`

### 示例项目
- `.specify/templates/examples/hello-c11/`
- `.specify/templates/examples/blinky/`

### 测试文件
- `tests/toolchain/test_install.bats`
- `tests/toolchain/test_compile.bats`
- `tests/build/test_build.bats`
- `tests/build/test_incremental.bats`
- `tests/build/test_recovery.bats`
- `tests/debug/test_debug_server.bats`
- `tests/integration/test_full_workflow.bats`

### 文档文件
- `specs/001-c11-embedded-env/spec.md` ✅
- `specs/001-c11-embedded-env/plan.md` ✅ (本文件)
- `specs/001-c11-embedded-env/research.md` ✅
- `specs/001-c11-embedded-env/data-model.md` ✅
- `specs/001-c11-embedded-env/quickstart.md` ✅
- `specs/001-c11-embedded-env/contracts/script-interface.md` ✅
- `specs/001-c11-embedded-env/tasks.md` ⏭️ (待生成)

---

**计划完成日期**: 2026-01-18  
**预计实现周期**: 2-3 周  
**下一步**: 执行 `/speckit.tasks` 生成任务列表
