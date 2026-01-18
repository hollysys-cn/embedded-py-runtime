# Tasks: 构建 C11 嵌入式开发环境

**Input**: Design documents from `/specs/001-c11-embedded-env/`
**Prerequisites**: [plan.md](plan.md), [spec.md](spec.md), [research.md](research.md), [data-model.md](data-model.md), [contracts/script-interface.md](contracts/script-interface.md)

**Feature**: 为 ARM Cortex-M4 构建完整的 C11 标准开发环境

## 任务格式: `[ID] [P?] [Story] Description`

- **[P]**: 可并行执行（不同文件，无依赖）
- **[Story]**: 所属用户故事（US1, US2, US3, US4）
- 包含精确的文件路径

---

## Phase 1: Setup (项目初始化)

**目的**: 创建基础目录结构和配置文件

- [X] T001 创建 `.specify/config/` 目录及基础 JSON 配置结构
- [X] T002 创建 `.specify/scripts/` 目录（setup/, build/, debug/ 子目录）
- [X] T003 创建 `.specify/templates/` 目录（cmake/, examples/ 子目录）
- [X] T004 [P] 更新 `.gitignore` 添加 `.toolchain/`, `build/`, `*.lock` 等规则
- [X] T005 [P] 创建 `tests/` 目录结构（toolchain/, build/, debug/, integration/）

---

## Phase 2: Foundational (基础设施 - 阻塞性前置任务)

**目的**: 核心配置和工具，所有用户故事的前置依赖

**⚠️ 关键**: 此阶段完成前，任何用户故事都不能开始实现

- [X] T006 创建 `.specify/config/toolchain.json` - 定义工具链版本、下载URL和SHA256
- [X] T007 [P] 创建 `.specify/config/mirrors.json` - 配置国内镜像源（清华、阿里云、中科大）
- [X] T008 [P] 创建 `.specify/config/platform-cortex-m4.json` - 定义ARM Cortex-M4编译标志和特性
- [X] T009 实现平台检测功能（detect_platform函数，识别 Linux/macOS/Windows）
- [X] T010 [P] 实现日志输出格式化函数（支持 [INFO]/[WARN]/[ERROR] 彩色输出）

**Checkpoint**: 基础配置就绪 - 用户故事实现可以开始并行进行

---

## Phase 3: User Story 1 - 基础编译环境设置 (Priority: P1) 🎯 MVP

**目标**: 自动安装和验证 ARM GCC 工具链，支持 C11 标准编译

**独立测试**: 编译包含 C11 特性的测试程序（_Static_assert, 匿名结构体），验证编译器识别

### 实现 User Story 1

- [X] T011 [P] [US1] 创建 `.specify/scripts/setup/install-toolchain.sh` - 工具链下载和安装主脚本
- [X] T012 [P] [US1] 实现 SHA256 校验函数（使用 sha256sum 或 shasum）
- [X] T013 [US1] 实现工具链下载逻辑（支持官方源和国内镜像，带进度显示）
- [X] T014 [US1] 实现工具链解压和安装到 `.toolchain/gcc-arm-none-eabi/`
- [X] T015 [US1] 创建 `.specify/scripts/setup/verify-toolchain.sh` - 验证工具链安装
- [X] T016 [US1] 实现 C11 特性检测（编译测试程序验证 _Static_assert等）
- [X] T017 [US1] 实现 Cortex-M4 支持检测（检查 --target-help 输出）
- [X] T018 [US1] 创建 `.specify/scripts/setup/configure-environment.sh` - 环境变量配置
- [X] T019 [US1] 添加工具链 bin/ 到 PATH，设置 ARM_TOOLCHAIN_PATH 环境变量
- [X] T020 [P] [US1] 创建 `tests/toolchain/test_install.bats` - 测试安装脚本
- [X] T021 [P] [US1] 创建 `tests/toolchain/test_compile.bats` - 测试 C11 编译

**Checkpoint**: 此时 User Story 1 应完全可用 - 能下载、安装、验证工具链并编译 C11 程序

---

## Phase 4: User Story 2 - 交叉编译配置 (Priority: P2)

**目标**: 配置 CMake 工具链文件和平台特定编译选项，生成 ARM Cortex-M4 可执行文件

**独立测试**: 编译嵌入式程序，使用 `file` 或 `objdump` 检查生成的 ELF 文件架构

### 实现 User Story 2

- [X] T022 [P] [US2] 创建 `.specify/templates/cmake/toolchain-arm-cortex-m4.cmake` - CMake 工具链文件
- [X] T023 [US2] 配置 CMAKE_SYSTEM_NAME=Generic 和 CMAKE_SYSTEM_PROCESSOR=ARM
- [X] T024 [US2] 设置 C 编译器路径（arm-none-eabi-gcc）和 C11 标准标志
- [X] T025 [US2] 配置 Cortex-M4 特定标志（-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16）
- [X] T026 [US2] 配置链接器选项（--specs=nano.specs --specs=nosys.specs）
- [X] T027 [P] [US2] 创建 `.specify/config/debug-openocd.json` - OpenOCD 调试配置
- [X] T028 [P] [US2] 创建 `.specify/templates/cmake/CMakeLists.txt.template` - 项目 CMake 模板
- [X] T029 [US2] 配置自动生成 HEX 和 BIN 文件的 CMake 命令
- [X] T030 [US2] 添加代码大小显示（arm-none-eabi-size）到 CMake 构建后命令

**Checkpoint**: 此时 User Story 2 应完全可用 - 能交叉编译生成 ARM Cortex-M4 二进制文件

---

## Phase 5: User Story 3 - 构建系统集成 (Priority: P3)

**目标**: 实现自动化构建脚本，支持增量编译、构建锁和失败恢复

**独立测试**: 修改单个源文件，验证只重新编译该文件和依赖项

### 实现 User Story 3

- [X] T031 [P] [US3] 创建 `.specify/scripts/build/lock-manager.sh` - 构建锁管理
- [X] T032 [US3] 实现锁文件创建/检查逻辑（`build/.build.lock`，包含PID）
- [X] T033 [US3] 实现锁过期检测（检查PID是否仍在运行）
- [X] T034 [US3] 创建 `.specify/scripts/build/build.sh` - 构建主脚本
- [X] T035 [US3] 实现命令行参数解析（--config, --clean, --verbose, --jobs）
- [X] T036 [US3] 集成构建锁机制，防止并发构建
- [X] T037 [US3] 实现 CMake 配置步骤（cmake -B build -DCMAKE_BUILD_TYPE=...）
- [X] T038 [US3] 实现 CMake 构建步骤（cmake --build build --parallel）
- [X] T039 [US3] 实现构建状态跟踪（保存到 `build/.build-state.json`）
- [X] T040 [US3] 创建 `.specify/scripts/build/clean.sh` - 清理构建产物
- [X] T041 [US3] 实现清理选项（--all, --obj, --artifacts）
- [X] T042 [US3] 实现构建失败时的自动清理逻辑
- [X] T043 [US3] 添加错误日志保存到 `build/logs/error.log`
- [X] T044 [P] [US3] 创建 `tests/build/test_build.bats` - 测试构建脚本
- [X] T045 [P] [US3] 创建 `tests/build/test_incremental.bats` - 测试增量编译
- [X] T046 [P] [US3] 创建 `tests/build/test_recovery.bats` - 测试构建失败恢复

**Checkpoint**: 此时 User Story 3 应完全可用 - 构建系统健壮，支持增量和恢复

---

## Phase 6: User Story 4 - 调试环境配置 (Priority: P4)

**目标**: 配置 OpenOCD 和 GDB，支持源码级调试

**独立测试**: 启动 OpenOCD，连接 GDB，设置断点并验证程序暂停

### 实现 User Story 4

- [X] T047 [P] [US4] 创建 `.specify/scripts/debug/start-debug-server.sh` - 启动 OpenOCD
- [X] T048 [US4] 实现命令行参数解析（--interface, --target, --port, --daemon）
- [X] T049 [US4] 实现接口配置（stlink-v2, jlink, cmsis-dap）
- [X] T050 [US4] 实现目标配置（stm32f4x, stm32f7x）
- [X] T051 [US4] 实现 OpenOCD 进程管理（启动、守护进程模式）
- [X] T052 [P] [US4] 创建 `.specify/scripts/debug/connect-gdb.sh` - GDB 连接脚本
- [X] T053 [US4] 实现 GDB 自动连接到 OpenOCD（target extended-remote localhost:3333）
- [X] T054 [US4] 实现 --load 选项（自动加载程序到目标设备）
- [X] T055 [US4] 实现 --tui 选项（启用 GDB TUI 模式）
- [X] T056 [US4] 添加 GDB 初始化命令（monitor reset halt, load, monitor reset init）
- [X] T057 [P] [US4] 创建 `tests/debug/test_debug_server.bats` - 测试调试服务器启动

**Checkpoint**: 此时 User Story 4 应完全可用 - 完整的调试环境配置就绪

---

## Phase 7: Polish & Cross-Cutting Concerns (打磨和跨领域关注点)

**目的**: 示例项目、文档完善和测试覆盖

- [X] T058 [P] 创建 `.specify/templates/examples/hello-c11/main.c` - C11 特性示例程序
- [X] T059 [P] 创建 `.specify/templates/examples/hello-c11/CMakeLists.txt`
- [X] T060 [P] 创建 `.specify/templates/examples/blinky/main.c` - LED 闪烁示例
- [X] T061 [P] 创建 `.specify/templates/examples/blinky/startup.s` - 启动汇编文件
- [X] T062 [P] 创建 `.specify/templates/examples/blinky/linker.ld` - 链接器脚本示例
- [X] T063 [P] 创建 `.specify/templates/examples/blinky/CMakeLists.txt`
- [X] T064 [P] 为所有脚本添加 --help 选项和使用说明
- [X] T065 [P] 实现跨平台兼容性（Windows PowerShell 脚本或 WSL 说明）
- [X] T066 [P] 添加彩色输出支持（检测终端是否支持，提供 --no-color 选项）
- [X] T067 创建 `tests/integration/test_full_workflow.bats` - 端到端集成测试
- [X] T068 验证所有测试通过（toolchain, build, debug, integration）
- [X] T069 更新项目 README.md，添加快速入门链接和徽章
- [X] T070 验证文档完整性（README, quickstart.md, 所有脚本的 --help）

---

## 依赖关系图

展示用户故事的完成顺序和依赖关系：

```
Phase 1 (Setup) → Phase 2 (Foundational)
                       ↓
                   ┌───┴────┬────────┬────────┐
                   ↓        ↓        ↓        ↓
                 US1      US2      US3      US4
             (基础编译) (交叉编译) (构建系统) (调试环境)
                   ↓        ↓        ↓        ↓
                   └────┬───┴───┬────┴────────┘
                        ↓       ↓
                  Phase 7 (Polish)
```

**并行机会**:
- US1, US2, US3, US4 可以并行开发（在 Phase 2 完成后）
- 各用户故事内标记 [P] 的任务可以并行执行

**建议 MVP 范围**:
- Phase 1 + Phase 2 + US1 (基础编译环境)
- 这提供了最基本的可用环境，后续可增量添加 US2-US4

---

## 并行执行示例

### User Story 1 内的并行任务
```bash
# 可同时执行：
T011 (install-toolchain.sh)  || T012 (SHA256校验函数)
T020 (test_install.bats)     || T021 (test_compile.bats)
```

### 跨 User Story 的并行任务
```bash
# Phase 2 完成后，可同时执行：
T011-T021 (US1) || T022-T030 (US2) || T031-T046 (US3) || T047-T057 (US4)
```

### Phase 7 的并行任务
```bash
# 示例和文档可完全并行：
T058-T059 (hello-c11) || T060-T063 (blinky) || T064-T066 (脚本优化) || T069-T070 (文档)
```

---

## 实现策略

### MVP 优先 (最小可行产品)

**第一次交付** (1周):
- Phase 1 + Phase 2 + User Story 1
- 交付物：能自动安装和验证 ARM GCC 工具链

**第二次交付** (2周):
- User Story 2 + User Story 3
- 交付物：完整的交叉编译和构建系统

**第三次交付** (3周):
- User Story 4 + Phase 7
- 交付物：调试环境 + 示例项目 + 完整文档

### 增量交付

每完成一个用户故事，立即可：
- 独立测试该故事的功能
- 部署给用户使用
- 收集反馈改进

这样即使后续故事延期，也已有可用的功能交付。

---

## 任务统计

**总任务数**: 70
**已完成**: 70 (100%)
**用户故事分布**:
- US1 (基础编译环境): 11 个任务 ✅
- US2 (交叉编译配置): 9 个任务 ✅
- US3 (构建系统集成): 16 个任务 ✅
- US4 (调试环境配置): 11 个任务 ✅
- Setup & Foundational: 10 个任务 ✅
- Polish: 13 个任务 ✅

**可并行任务**: 约 40 个 (标记 [P])

**关键路径任务**:
- Phase 1 (5 tasks) → Phase 2 (5 tasks) → US1 核心 (6 tasks) → US3 核心 (10 tasks)
- 关键路径总计: ~26 个任务 ✅

**实际完成时间**: 所有任务已完成
**状态**: ✅ 生产就绪

---

## 验收标准

每个用户故事完成后，必须通过以下验证：

### US1 验收
```bash
# 安装工具链
.specify/scripts/setup/install-toolchain.sh --mirror tsinghua

# 验证安装
.specify/scripts/setup/verify-toolchain.sh  # 应输出 "验证通过 ✓"

# 配置环境
source .specify/scripts/setup/configure-environment.sh

# 编译 C11 测试程序
arm-none-eabi-gcc -std=c11 -mcpu=cortex-m4 test_c11.c  # 应成功
```

### US2 验收
```bash
# 使用 CMake 工具链文件编译
cmake -B build -DCMAKE_TOOLCHAIN_FILE=.specify/templates/cmake/toolchain-arm-cortex-m4.cmake
cmake --build build

# 检查生成的文件架构
file build/firmware.elf  # 应显示 "ARM aarch64"
arm-none-eabi-objdump -f build/firmware.elf  # 应显示 "arm"
```

### US3 验收
```bash
# 完整构建
.specify/scripts/build/build.sh  # 应成功

# 修改一个文件
echo "// comment" >> src/main.c

# 增量构建（应<10秒）
time .specify/scripts/build/build.sh

# 引入错误
echo "invalid" >> src/main.c

# 构建失败恢复
.specify/scripts/build/build.sh  # 失败
git checkout src/main.c
.specify/scripts/build/build.sh  # 应自动恢复并成功
```

### US4 验收
```bash
# 启动调试服务器
.specify/scripts/debug/start-debug-server.sh &  # 后台运行

# 连接 GDB
.specify/scripts/debug/connect-gdb.sh build/firmware.elf --load

# GDB 中验证
(gdb) break main
(gdb) continue  # 应在 main 处停止
(gdb) info locals  # 应显示局部变量
(gdb) quit
```

---

## 注意事项

### 跨平台开发
- **Linux/macOS**: 使用 Bash 脚本
- **Windows**: 推荐 Git Bash 或 WSL2，或提供 PowerShell 版本

### 测试覆盖
- 所有脚本应有对应的 bats 测试文件
- 目标测试覆盖率: >80%
- 优先测试关键路径和错误处理

### 文档更新
- 每完成一个用户故事，更新 quickstart.md
- 确保所有脚本都有 --help 输出
- README.md 应始终反映最新状态

### 代码质量
- 所有 Shell 脚本使用 `set -euo pipefail`
- 使用 shellcheck 检查脚本质量
- UTF-8 编码，LF 换行符
- 添加详细的中文注释

---

**任务列表生成日期**: 2026-01-18
**基于文档**: spec.md, plan.md, research.md, data-model.md, contracts/script-interface.md
**下一步**: 按照 Phase 顺序和用户故事优先级开始实现
