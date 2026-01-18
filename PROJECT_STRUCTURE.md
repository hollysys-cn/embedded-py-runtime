# C11 嵌入式开发环境 - 项目结构

## 目录组织

```
embedded-py-runtime/
├── config/                     # 配置文件
│   ├── toolchain.json          # ARM GCC 工具链配置
│   ├── mirrors.json            # 国内镜像源配置
│   ├── platform-cortex-m4.json # Cortex-M4 平台配置
│   └── debug-openocd.json      # OpenOCD 调试配置
│
├── scripts/                    # 脚本工具
│   ├── common.sh               # 公共函数库
│   ├── setup/                  # 环境设置脚本
│   │   ├── install-toolchain.sh       # 工具链安装
│   │   ├── verify-toolchain.sh        # 工具链验证
│   │   └── configure-environment.sh   # 环境配置
│   ├── build/                  # 构建脚本
│   │   ├── build.sh            # 主构建脚本
│   │   ├── clean.sh            # 清理脚本
│   │   └── lock-manager.sh     # 构建锁管理
│   └── debug/                  # 调试脚本
│       ├── start-debug-server.sh  # 启动 OpenOCD
│       └── connect-gdb.sh      # 连接 GDB
│
├── templates/                  # 模板文件
│   ├── cmake/                  # CMake 模板
│   │   ├── toolchain-arm-cortex-m4.cmake  # 工具链文件
│   │   └── CMakeLists.txt.template        # 项目模板
│   └── examples/               # 示例项目
│       ├── hello-c11/          # C11 特性演示
│       │   ├── main.c
│       │   ├── CMakeLists.txt
│       │   └── README.md
│       └── blinky/             # LED 闪烁示例
│           ├── main.c
│           ├── startup.s
│           ├── linker.ld
│           └── CMakeLists.txt
│
├── tests/                      # 测试文件（可选）
│   ├── toolchain/
│   │   ├── test_install.bats
│   │   └── test_compile.bats
│   ├── build/
│   └── debug/
│
├── .toolchain/                 # 工具链安装目录（自动创建）
│   └── gcc-arm-none-eabi/
│
├── build/                      # 构建输出目录（自动创建）
│   ├── .build-state.json
│   └── logs/
│       └── error.log
│
├── specs/                      # 功能规范（Speckit）
│   └── 001-c11-embedded-env/
│       ├── spec.md
│       ├── plan.md
│       ├── tasks.md
│       └── ...
│
├── .specify/                   # Speckit 工具（不要放项目文件）
│   ├── config/
│   ├── memory/
│   ├── scripts/
│   └── templates/
│
├── README-c11-embedded.md      # 用户指南
├── IMPLEMENTATION_REPORT.md    # 实现报告
└── .gitignore
```

## 文件说明

### 配置文件 (config/)

#### toolchain.json
- ARM GCC 工具链版本配置
- 不同平台的下载 URL
- SHA256 校验和

#### mirrors.json
- 国内镜像源配置
- 优先级设置
- 超时和重试策略

#### platform-cortex-m4.json
- Cortex-M4 编译选项
- 优化标志
- C11 特性支持列表

#### debug-openocd.json
- OpenOCD 接口配置
- 目标 MCU 配置
- GDB 端口设置

### 脚本工具 (scripts/)

#### 设置脚本 (scripts/setup/)

**install-toolchain.sh**
- 自动下载 ARM GCC 工具链
- 支持国内镜像加速
- SHA256 完整性验证
- 跨平台支持

**verify-toolchain.sh**
- 验证工具链安装
- 测试 C11 特性支持
- 检查 Cortex-M4 支持

**configure-environment.sh**
- 配置 PATH 环境变量
- 设置 CMAKE_TOOLCHAIN_FILE
- 创建工具别名

#### 构建脚本 (scripts/build/)

**build.sh**
- 主构建脚本
- CMake 配置和构建
- 增量编译支持
- 构建状态跟踪

**clean.sh**
- 清理构建产物
- 支持部分清理
- 自动清理失败构建

**lock-manager.sh**
- 防止并发构建
- PID 验证
- 自动清理僵尸锁

#### 调试脚本 (scripts/debug/)

**start-debug-server.sh**
- 启动 OpenOCD 服务器
- 支持多种调试接口
- 守护进程模式

**connect-gdb.sh**
- 连接 GDB 到 OpenOCD
- 自动加载程序
- TUI 模式支持

### 模板文件 (templates/)

#### CMake 模板 (templates/cmake/)

**toolchain-arm-cortex-m4.cmake**
- CMake 工具链配置
- Cortex-M4 编译标志
- 链接器选项

**CMakeLists.txt.template**
- 项目配置模板
- 自动生成 HEX/BIN
- 代码大小统计

#### 示例项目 (templates/examples/)

**hello-c11/**
- C11 特性演示
- 无硬件依赖
- 适合快速验证

**blinky/**
- LED 闪烁示例
- 完整硬件项目
- 包含启动代码和链接脚本

## 使用流程

### 1. 初始设置
```bash
# 安装工具链
scripts/setup/install-toolchain.sh

# 验证安装
scripts/setup/verify-toolchain.sh

# 配置环境
source scripts/setup/configure-environment.sh
```

### 2. 构建项目
```bash
# 使用示例项目
cd templates/examples/hello-c11
cmake -B build -DCMAKE_TOOLCHAIN_FILE=../../cmake/toolchain-arm-cortex-m4.cmake
cmake --build build

# 或使用构建脚本（项目根目录有 CMakeLists.txt 时）
scripts/build/build.sh --config Release
```

### 3. 调试（需要硬件）
```bash
# 启动调试服务器
scripts/debug/start-debug-server.sh --daemon

# 连接 GDB
scripts/debug/connect-gdb.sh --load --tui build/firmware.elf
```

## 设计原则

1. **清晰的职责分离**
   - `config/` - 配置数据
   - `scripts/` - 可执行工具
   - `templates/` - 项目模板
   - `.specify/` - Speckit 框架（不放项目文件）

2. **跨平台兼容**
   - 所有脚本支持 Linux/macOS/Windows (Git Bash)
   - 路径处理兼容不同系统
   - 命令检查和降级方案

3. **用户友好**
   - 中文界面和日志
   - 详细的帮助信息
   - 彩色输出（可禁用）

4. **生产就绪**
   - 完整的错误处理
   - 构建锁机制
   - 日志记录和状态跟踪

## 扩展指南

### 添加新的平台支持

1. 更新 `config/toolchain.json` 添加平台信息
2. 修改 `scripts/common.sh` 的 `detect_platform()` 函数
3. 测试安装和编译流程

### 添加新的 MCU 支持

1. 创建新的平台配置文件 `config/platform-xxx.json`
2. 更新工具链文件或创建新的工具链文件
3. 在 `templates/examples/` 添加示例项目

### 添加新的镜像源

1. 更新 `config/mirrors.json`
2. 确保镜像源结构与现有格式兼容
3. 测试下载功能

## 注意事项

- **不要在 `.specify/` 下放置项目文件**，该目录仅用于 Speckit 框架
- `.toolchain/` 和 `build/` 目录会自动创建，已加入 `.gitignore`
- 配置文件使用 JSON 格式，便于程序解析和人工编辑
- 所有脚本使用 UTF-8 编码，LF 换行符
- 脚本遵循 shellcheck 规范，使用 `set -euo pipefail`
