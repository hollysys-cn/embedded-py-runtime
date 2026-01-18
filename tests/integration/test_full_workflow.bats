#!/usr/bin/env bats
# 端到端集成测试 - 完整工作流程
# Task: T067 - 端到端集成测试

setup() {
    export PROJECT_ROOT="$(cd "$(dirname "$BATS_TEST_FILENAME")/../.." && pwd)"
    export INSTALL_SCRIPT="$PROJECT_ROOT/scripts/setup/install-toolchain.sh"
    export VERIFY_SCRIPT="$PROJECT_ROOT/scripts/setup/verify-toolchain.sh"
    export CONFIG_SCRIPT="$PROJECT_ROOT/scripts/setup/configure-environment.sh"
    export BUILD_SCRIPT="$PROJECT_ROOT/scripts/build/build.sh"
    export CLEAN_SCRIPT="$PROJECT_ROOT/scripts/build/clean.sh"
    export DEBUG_SERVER_SCRIPT="$PROJECT_ROOT/scripts/debug/start-debug-server.sh"
    export CONNECT_GDB_SCRIPT="$PROJECT_ROOT/scripts/debug/connect-gdb.sh"
    export TEST_WORKSPACE="$BATS_TMPDIR/integration-test-$$"

    # 创建测试工作区
    mkdir -p "$TEST_WORKSPACE"/{src,include}
}

teardown() {
    # 清理测试工作区
    [ -d "$TEST_WORKSPACE" ] && rm -rf "$TEST_WORKSPACE"
}

# ============================================================================
# 阶段 1: 环境设置测试
# ============================================================================

@test "[集成] 所有脚本都存在且可执行" {
    [ -f "$INSTALL_SCRIPT" ] && [ -x "$INSTALL_SCRIPT" ]
    [ -f "$VERIFY_SCRIPT" ] && [ -x "$VERIFY_SCRIPT" ]
    [ -f "$CONFIG_SCRIPT" ] && [ -x "$CONFIG_SCRIPT" ]
    [ -f "$BUILD_SCRIPT" ] && [ -x "$BUILD_SCRIPT" ]
    [ -f "$CLEAN_SCRIPT" ] && [ -x "$CLEAN_SCRIPT" ]
    [ -f "$DEBUG_SERVER_SCRIPT" ] && [ -x "$DEBUG_SERVER_SCRIPT" ]
    [ -f "$CONNECT_GDB_SCRIPT" ] && [ -x "$CONNECT_GDB_SCRIPT" ]
}

@test "[集成] 所有脚本都有帮助信息" {
    run "$INSTALL_SCRIPT" --help
    [ "$status" -eq 0 ]

    run "$VERIFY_SCRIPT" --help
    [ "$status" -eq 0 ]

    run "$CONFIG_SCRIPT" --help
    [ "$status" -eq 0 ]

    run "$BUILD_SCRIPT" --help
    [ "$status" -eq 0 ]

    run "$CLEAN_SCRIPT" --help
    [ "$status" -eq 0 ]

    run "$DEBUG_SERVER_SCRIPT" --help
    [ "$status" -eq 0 ]

    run "$CONNECT_GDB_SCRIPT" --help
    [ "$status" -eq 0 ]
}

@test "[集成] 配置文件都存在" {
    [ -f "$PROJECT_ROOT/config/toolchain.json" ]
    [ -f "$PROJECT_ROOT/config/mirrors.json" ]
    [ -f "$PROJECT_ROOT/config/platform-cortex-m4.json" ]
    [ -f "$PROJECT_ROOT/config/debug-openocd.json" ]
}

# ============================================================================
# 阶段 2: 工具链安装和验证工作流
# ============================================================================

@test "[集成] 工具链验证流程（如果已安装）" {
    skip "需要预先安装工具链"

    # 验证工具链
    run "$VERIFY_SCRIPT"
    [ "$status" -eq 0 ]

    # 配置环境
    run "$CONFIG_SCRIPT"
    [ "$status" -eq 0 ]
}

# ============================================================================
# 阶段 3: 完整的 C11 项目构建工作流
# ============================================================================

@test "[集成] 创建并构建简单的 C11 项目" {
    skip "需要安装完整工具链"

    cd "$TEST_WORKSPACE"

    # 1. 创建 C11 测试程序
    cat > src/main.c << 'EOF'
#include <stdint.h>

// C11 特性: _Static_assert
_Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");

// C11 特性: 匿名结构体
struct {
    uint32_t x;
    uint32_t y;
} point = {100, 200};

int main(void) {
    // C11 特性: 泛型选择
    uint32_t value = 42;

    while (1) {
        // 嵌入式主循环
        value++;
        if (value > 1000000) {
            value = 0;
        }
    }

    return 0;
}
EOF

    # 2. 创建 CMakeLists.txt（使用工具链模板）
    cp "$PROJECT_ROOT/templates/cmake/toolchain-arm-cortex-m4.cmake" .

    cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.20)

# 使用 ARM Cortex-M4 工具链
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain-arm-cortex-m4.cmake")

project(IntegrationTest C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(integration_test src/main.c)

# 生成 HEX 和 BIN 文件
add_custom_command(TARGET integration_test POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:integration_test> integration_test.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:integration_test> integration_test.bin
    COMMENT "生成 HEX 和 BIN 文件"
)

# 显示代码大小
add_custom_command(TARGET integration_test POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:integration_test>
    COMMENT "显示代码大小"
)
EOF

    # 3. 配置构建
    run "$BUILD_SCRIPT" --config Release
    [ "$status" -eq 0 ]

    # 4. 验证生成的文件
    [ -f "build/integration_test" ] || [ -f "build/Release/integration_test" ]

    # 5. 验证生成的 HEX 和 BIN 文件
    find build -name "*.hex" | grep -q .
    find build -name "*.bin" | grep -q .
}

@test "[集成] 多文件 C11 项目构建工作流" {
    skip "需要安装完整工具链"

    cd "$TEST_WORKSPACE"

    # 1. 创建头文件
    cat > include/math_utils.h << 'EOF'
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>

uint32_t add(uint32_t a, uint32_t b);
uint32_t multiply(uint32_t a, uint32_t b);
uint32_t factorial(uint32_t n);

#endif // MATH_UTILS_H
EOF

    # 2. 创建实现文件
    cat > src/math_utils.c << 'EOF'
#include "math_utils.h"

uint32_t add(uint32_t a, uint32_t b) {
    return a + b;
}

uint32_t multiply(uint32_t a, uint32_t b) {
    return a * b;
}

uint32_t factorial(uint32_t n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
EOF

    # 3. 创建主程序
    cat > src/main.c << 'EOF'
#include <stdint.h>
#include "math_utils.h"

// C11 特性
_Static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes");

int main(void) {
    uint32_t result = add(10, 20);
    result = multiply(result, 2);
    result = factorial(5);

    while (1) {
        // 主循环
    }

    return 0;
}
EOF

    # 4. 创建 CMakeLists.txt
    cp "$PROJECT_ROOT/templates/cmake/toolchain-arm-cortex-m4.cmake" .

    cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain-arm-cortex-m4.cmake")
project(MultiFileTest C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(multi_file_test
    src/main.c
    src/math_utils.c
)

add_custom_command(TARGET multi_file_test POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:multi_file_test>
)
EOF

    # 5. 完整构建
    run "$BUILD_SCRIPT" --config Debug
    [ "$status" -eq 0 ]

    # 6. 修改一个文件，测试增量编译
    sleep 2
    echo "// Modified" >> src/math_utils.c

    run "$BUILD_SCRIPT" --config Debug --verbose
    [ "$status" -eq 0 ]
    [[ "$output" =~ "math_utils.c" ]]
}

# ============================================================================
# 阶段 4: 构建清理工作流
# ============================================================================

@test "[集成] 构建和清理循环" {
    skip "需要安装完整工具链"

    cd "$TEST_WORKSPACE"

    # 使用简单的示例项目
    cp -r "$PROJECT_ROOT/templates/examples/hello-c11"/* .
    cp "$PROJECT_ROOT/templates/cmake/toolchain-arm-cortex-m4.cmake" .

    # 1. 构建项目
    run "$BUILD_SCRIPT" --config Debug
    [ "$status" -eq 0 ]
    [ -d "build" ]

    # 2. 部分清理（只清理对象文件）
    run "$CLEAN_SCRIPT" --obj
    [ "$status" -eq 0 ]

    # 3. 重新构建
    run "$BUILD_SCRIPT" --config Debug
    [ "$status" -eq 0 ]

    # 4. 完全清理
    run "$CLEAN_SCRIPT" --all
    [ "$status" -eq 0 ]
    [ ! -d "build" ] || [ -z "$(ls -A build 2>/dev/null)" ]

    # 5. 再次完整构建
    run "$BUILD_SCRIPT" --config Release
    [ "$status" -eq 0 ]
}

# ============================================================================
# 阶段 5: 示例项目验证
# ============================================================================

@test "[集成] hello-c11 示例可以构建" {
    skip "需要安装完整工具链"

    cd "$TEST_WORKSPACE"

    # 复制 hello-c11 示例
    cp -r "$PROJECT_ROOT/templates/examples/hello-c11"/* .
    cp "$PROJECT_ROOT/templates/cmake/toolchain-arm-cortex-m4.cmake" .

    # 构建
    run "$BUILD_SCRIPT" --config Debug
    [ "$status" -eq 0 ]

    # 验证输出文件
    find build -name "*.elf" -o -name "hello-c11" | grep -q .
}

@test "[集成] blinky 示例可以构建" {
    skip "需要安装完整工具链"

    cd "$TEST_WORKSPACE"

    # 复制 blinky 示例
    cp -r "$PROJECT_ROOT/templates/examples/blinky"/* .
    cp "$PROJECT_ROOT/templates/cmake/toolchain-arm-cortex-m4.cmake" .

    # 构建
    run "$BUILD_SCRIPT" --config Release
    [ "$status" -eq 0 ]

    # 验证输出文件（包括 HEX 和 BIN）
    find build -name "*.hex" | grep -q .
    find build -name "*.bin" | grep -q .
}

# ============================================================================
# 阶段 6: 调试工作流验证
# ============================================================================

@test "[集成] 调试脚本配置正确" {
    # 验证调试服务器脚本
    run "$DEBUG_SERVER_SCRIPT" --help
    [ "$status" -eq 0 ]
    [[ "$output" =~ "interface" ]]
    [[ "$output" =~ "target" ]]

    # 验证 GDB 连接脚本
    run "$CONNECT_GDB_SCRIPT" --help
    [ "$status" -eq 0 ]
}

@test "[集成] 调试配置文件有效" {
    local debug_config="$PROJECT_ROOT/config/debug-openocd.json"

    [ -f "$debug_config" ]

    # 验证 JSON 格式（如果有 jq）
    if command -v jq &>/dev/null; then
        run jq empty "$debug_config"
        [ "$status" -eq 0 ]
    fi
}

# ============================================================================
# 阶段 7: 错误处理和恢复
# ============================================================================

@test "[集成] 从构建失败中恢复" {
    skip "需要安装完整工具链"

    cd "$TEST_WORKSPACE"

    # 1. 创建有错误的项目
    cat > src/main.c << 'EOF'
#include <stdint.h>

int main(void) {
    int x = 10  // 语法错误
    return 0;
}
EOF

    cp "$PROJECT_ROOT/templates/cmake/toolchain-arm-cortex-m4.cmake" .
    cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain-arm-cortex-m4.cmake")
project(ErrorRecovery C)

set(CMAKE_C_STANDARD 11)
add_executable(error_test src/main.c)
EOF

    # 2. 构建失败
    run "$BUILD_SCRIPT" --config Debug
    [ "$status" -ne 0 ]

    # 3. 修复代码
    cat > src/main.c << 'EOF'
#include <stdint.h>

int main(void) {
    int x = 10;
    return 0;
}
EOF

    # 4. 成功构建
    run "$BUILD_SCRIPT" --config Debug
    [ "$status" -eq 0 ]
}

# ============================================================================
# 阶段 8: 完整工作流程测试
# ============================================================================

@test "[集成] 完整开发工作流：创建->构建->修改->重构建->清理" {
    skip "需要安装完整工具链"

    cd "$TEST_WORKSPACE"

    # 1. 创建初始项目
    cat > src/main.c << 'EOF'
#include <stdint.h>

int main(void) {
    uint32_t counter = 0;
    while (1) {
        counter++;
    }
    return 0;
}
EOF

    cp "$PROJECT_ROOT/templates/cmake/toolchain-arm-cortex-m4.cmake" .
    cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain-arm-cortex-m4.cmake")
project(FullWorkflow C)
set(CMAKE_C_STANDARD 11)
add_executable(workflow_test src/main.c)
EOF

    # 2. 初次构建
    "$BUILD_SCRIPT" --config Debug
    [ -d "build" ]

    # 3. 修改代码
    sleep 2
    cat > src/main.c << 'EOF'
#include <stdint.h>

_Static_assert(sizeof(int) >= 4, "int too small");

int main(void) {
    uint32_t counter = 0;
    uint32_t limit = 1000000;
    while (1) {
        counter++;
        if (counter >= limit) {
            counter = 0;
        }
    }
    return 0;
}
EOF

    # 4. 增量构建
    "$BUILD_SCRIPT" --config Debug

    # 5. 切换到 Release 配置
    "$BUILD_SCRIPT" --config Release

    # 6. 清理 Debug 构建
    "$CLEAN_SCRIPT" --all

    # 7. 完全重新构建
    "$BUILD_SCRIPT" --config Release

    # 验证最终状态
    [ -d "build" ]
    find build -name "workflow_test*" | grep -q .
}
