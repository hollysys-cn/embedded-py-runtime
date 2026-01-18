# 数据模型: C11 嵌入式开发环境

**日期**: 2026-01-18
**关联**: [spec.md](spec.md) | [plan.md](plan.md) | [research.md](research.md)

## 概述

本文档定义 C11 嵌入式开发环境中的核心数据实体、配置结构和状态管理。

## 核心实体

### 1. 工具链配置 (ToolchainConfig)

存储工具链的元数据和获取信息。

**文件**: `.specify/config/toolchain.json`

```json
{
  "name": "ARM GNU Toolchain",
  "version": "10.3-2021.10",
  "vendor": "ARM",
  "targets": ["arm-none-eabi"],
  "platforms": {
    "linux-x86_64": {
      "url": "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2",
      "sha256": "97dbb4f019ad1650b732faffcc881689cedc14e2b7ee863d390e0a41ef16c9a3",
      "mirrors": [
        "https://mirrors.tuna.tsinghua.edu.cn/armbian-releases/_toolchain/gcc-arm-10.3-2021.10-x86_64-linux.tar.bz2"
      ]
    },
    "darwin-x86_64": {
      "url": "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-mac.tar.bz2",
      "sha256": "fb613dacbc7d0c4a7a35ee4a8c3a1ae151f1b2f81e5e981b5cc32d3ecc75bbe2"
    },
    "win32": {
      "url": "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-win32.zip",
      "sha256": "f7b8b8b3c4d04d7e3f4fa70a7b2e72e5b3c8f2bef6e8b3c1d3e4b5f9c8d7e6f5"
    }
  },
  "install_path": ".toolchain/gcc-arm-none-eabi",
  "executables": {
    "gcc": "bin/arm-none-eabi-gcc",
    "g++": "bin/arm-none-eabi-g++",
    "as": "bin/arm-none-eabi-as",
    "ld": "bin/arm-none-eabi-ld",
    "objcopy": "bin/arm-none-eabi-objcopy",
    "objdump": "bin/arm-none-eabi-objdump",
    "size": "bin/arm-none-eabi-size",
    "gdb": "bin/arm-none-eabi-gdb"
  }
}
```

**关系**:
- 被 `BuildConfig` 引用
- 验证脚本读取此配置进行完整性检查

### 2. 平台配置 (PlatformConfig)

定义目标嵌入式平台的硬件特性和编译参数。

**文件**: `.specify/config/platform-cortex-m4.json`

```json
{
  "name": "ARM Cortex-M4",
  "family": "ARM Cortex-M",
  "architecture": "ARMv7E-M",
  "cpu": "cortex-m4",
  "fpu": "fpv4-sp-d16",
  "float_abi": "hard",
  "endian": "little",
  "pointer_size": 32,
  "examples": ["STM32F4", "LPC4300", "Kinetis K60"],
  "compiler_flags": {
    "common": [
      "-mcpu=cortex-m4",
      "-mthumb",
      "-mfloat-abi=hard",
      "-mfpu=fpv4-sp-d16"
    ],
    "c_standard": ["-std=c11"],
    "optimization": {
      "debug": ["-O0", "-g3"],
      "release_size": ["-Os", "-g0"],
      "release_speed": ["-O2", "-g0"]
    },
    "warnings": [
      "-Wall",
      "-Wextra",
      "-Wpedantic",
      "-Wshadow"
    ]
  },
  "linker_flags": [
    "--specs=nano.specs",
    "--specs=nosys.specs",
    "-Wl,--gc-sections"
  ],
  "c11_features": {
    "supported": [
      "_Static_assert",
      "_Generic",
      "_Alignas",
      "_Alignof",
      "anonymous_structs",
      "atomic_types_basic"
    ],
    "limited": [
      "threads_h"
    ],
    "unsupported": []
  },
  "memory_layout": {
    "flash_start": "0x08000000",
    "flash_size": "512K",
    "ram_start": "0x20000000",
    "ram_size": "128K",
    "note": "示例值，实际应根据具体 MCU 型号配置"
  }
}
```

**关系**:
- 被 `BuildConfig` 引用
- 用于生成 CMake toolchain 文件

### 3. 构建配置 (BuildConfig)

定义单次构建的完整配置。

**文件**: 在项目根目录的 `CMakeLists.txt` 或 `.specify/config/build.json`

```json
{
  "project_name": "hello-c11",
  "version": "1.0.0",
  "toolchain": "arm-none-eabi",
  "platform": "cortex-m4",
  "build_type": "Debug",
  "source_dirs": ["src"],
  "include_dirs": ["include"],
  "defines": {
    "USE_HAL_DRIVER": true,
    "STM32F407xx": true
  },
  "output": {
    "elf": "build/bin/${project_name}.elf",
    "hex": "build/bin/${project_name}.hex",
    "bin": "build/bin/${project_name}.bin",
    "map": "build/bin/${project_name}.map"
  },
  "dependencies": [],
  "linker_script": "STM32F407VGTx_FLASH.ld"
}
```

### 4. 构建状态 (BuildState)

跟踪构建过程的状态信息。

**文件**: `build/.build-state.json` (运行时生成)

```json
{
  "status": "success",
  "timestamp": "2026-01-18T14:30:00Z",
  "duration_seconds": 45.3,
  "toolchain_version": "10.3-2021.10",
  "cmake_version": "3.25.1",
  "build_type": "Debug",
  "platform": "cortex-m4",
  "statistics": {
    "total_files": 25,
    "compiled_files": 25,
    "skipped_files": 0,
    "warnings": 0,
    "errors": 0
  },
  "artifacts": {
    "elf": {
      "path": "build/bin/hello-c11.elf",
      "size_bytes": 15360,
      "sections": {
        "text": 12288,
        "data": 512,
        "bss": 2048
      }
    },
    "hex": {
      "path": "build/bin/hello-c11.hex",
      "size_bytes": 34816
    }
  },
  "last_error": null
}
```

**关系**:
- 用于增量构建决策
- 构建失败时记录错误信息

### 5. 调试配置 (DebugConfig)

配置调试会话的连接参数。

**文件**: `.specify/config/debug-openocd.json`

```json
{
  "debugger": "openocd",
  "version": "0.11.0",
  "interface": {
    "type": "stlink-v2",
    "config_file": "interface/stlink-v2.cfg"
  },
  "target": {
    "type": "stm32f4x",
    "config_file": "target/stm32f4x.cfg"
  },
  "gdb": {
    "executable": ".toolchain/gcc-arm-none-eabi/bin/arm-none-eabi-gdb",
    "port": 3333,
    "commands": [
      "target extended-remote localhost:3333",
      "monitor reset halt",
      "load",
      "monitor reset init"
    ]
  },
  "transport": "swd",
  "speed_khz": 4000
}
```

### 6. 依赖关系图 (DependencyGraph)

记录源文件之间的依赖关系（由 CMake 自动生成和维护）。

**文件**: `build/CMakeFiles/*.dir/depend.make` (CMake 生成)

这是 CMake 内部格式，用于增量编译。开发者通常不需要手动编辑。

## 配置文件层次

```
优先级从高到低：
1. 项目根目录的 CMakeLists.txt (项目特定配置)
2. .specify/config/*.json (环境配置)
3. .toolchain/ (工具链安装，自动生成)
4. build/ (构建产物和状态，自动生成)
```

## 状态转换

### 工具链安装状态

```
[Not Installed] --install--> [Downloading] --verify--> [Installed]
                                   |                       |
                                   |                       |
                                [Failed] <---------[Verification Failed]
```

### 构建状态

```
[Clean] --configure--> [Configured] --build--> [Built] --success--> [Success]
   ^                        |                     |
   |                        |                     |
   |                   [Config Failed]       [Build Failed]
   |                        |                     |
   +------------------------+---------------------+
                       (clean)
```

## 数据验证规则

### ToolchainConfig
- `version`: 必须是有效的语义化版本号
- `sha256`: 必须是 64 个十六进制字符
- `url`: 必须是有效的 HTTPS URL

### PlatformConfig
- `cpu`: 必须是有效的 ARM Cortex 型号
- `memory_layout`: 地址必须是有效的十六进制值

### BuildConfig
- `project_name`: 只能包含字母、数字、下划线和连字符
- `linker_script`: 必须指向存在的 `.ld` 文件

## 文件编码规范

- **编码**: UTF-8 (无 BOM)
- **换行符**: LF (Unix 风格)
- **缩进**: JSON 使用 2 空格，Shell 脚本使用 4 空格

## 总结

所有核心数据模型已定义完毕，覆盖了工具链管理、平台配置、构建流程和调试配置的完整生命周期。
