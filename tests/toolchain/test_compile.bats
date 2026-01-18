#!/usr/bin/env bats
# -*- coding: utf-8 -*-
#
# C11 编译功能测试
#
# 运行方式: bats tests/toolchain/test_compile.bats

setup() {
    export TEST_ROOT="$(cd "${BATS_TEST_DIRNAME}/../.." && pwd)"
    export NO_COLOR=1
    export TEST_TEMP_DIR="$(mktemp -d)"

    # 检查工具链是否已安装
    if [[ ! -d "$TEST_ROOT/.toolchain/gcc-arm-none-eabi" ]]; then
        skip "工具链未安装，跳过编译测试"
    fi

    # 配置环境
    source "$TEST_ROOT/.specify/scripts/setup/configure-environment.sh" 2>/dev/null || true
}

teardown() {
    if [[ -n "${TEST_TEMP_DIR:-}" ]] && [[ -d "$TEST_TEMP_DIR" ]]; then
        rm -rf "$TEST_TEMP_DIR"
    fi
}

@test "arm-none-eabi-gcc 命令可用" {
    run which arm-none-eabi-gcc
    [ "$status" -eq 0 ]
}

@test "编译简单的 C11 程序" {
    cat > "$TEST_TEMP_DIR/test.c" << 'EOF'
#include <stddef.h>

_Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");

int main(void) {
    return 0;
}
EOF

    run arm-none-eabi-gcc -std=c11 -mcpu=cortex-m4 -mthumb -c "$TEST_TEMP_DIR/test.c" -o "$TEST_TEMP_DIR/test.o"
    [ "$status" -eq 0 ]
    [ -f "$TEST_TEMP_DIR/test.o" ]
}

@test "C11 _Generic 宏支持" {
    cat > "$TEST_TEMP_DIR/test_generic.c" << 'EOF'
int main(void) {
    int x = 42;
    const char* type = _Generic(x,
        int: "integer",
        float: "float",
        default: "unknown"
    );
    (void)type;
    return 0;
}
EOF

    run arm-none-eabi-gcc -std=c11 -mcpu=cortex-m4 -mthumb -c "$TEST_TEMP_DIR/test_generic.c" -o "$TEST_TEMP_DIR/test_generic.o"
    [ "$status" -eq 0 ]
}

@test "C11 匿名结构体支持" {
    cat > "$TEST_TEMP_DIR/test_anon.c" << 'EOF'
struct {
    union {
        int x;
        float y;
    };
    int z;
} test_struct;

int main(void) {
    test_struct.x = 42;
    test_struct.z = 10;
    return 0;
}
EOF

    run arm-none-eabi-gcc -std=c11 -mcpu=cortex-m4 -mthumb -c "$TEST_TEMP_DIR/test_anon.c" -o "$TEST_TEMP_DIR/test_anon.o"
    [ "$status" -eq 0 ]
}

@test "Cortex-M4 特定标志编译成功" {
    cat > "$TEST_TEMP_DIR/test_m4.c" << 'EOF'
int main(void) {
    volatile float x = 3.14f;
    volatile float y = x * 2.0f;
    (void)y;
    return 0;
}
EOF

    run arm-none-eabi-gcc -std=c11 -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -c "$TEST_TEMP_DIR/test_m4.c" -o "$TEST_TEMP_DIR/test_m4.o"
    [ "$status" -eq 0 ]
}
