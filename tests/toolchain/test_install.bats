#!/usr/bin/env bats
# -*- coding: utf-8 -*-
#
# 工具链安装脚本测试
#
# 运行方式: bats tests/toolchain/test_install.bats

setup() {
    # 测试环境设置
    export TEST_ROOT="$(cd "${BATS_TEST_DIRNAME}/../.." && pwd)"
    export NO_COLOR=1

    # 创建临时测试目录
    export TEST_TEMP_DIR="$(mktemp -d)"
}

teardown() {
    # 清理测试环境
    if [[ -n "${TEST_TEMP_DIR:-}" ]] && [[ -d "$TEST_TEMP_DIR" ]]; then
        rm -rf "$TEST_TEMP_DIR"
    fi
}

@test "install-toolchain.sh --help 显示帮助信息" {
    run "$TEST_ROOT/.specify/scripts/setup/install-toolchain.sh" --help
    [ "$status" -eq 0 ]
    [[ "$output" =~ "用法" ]]
    [[ "$output" =~ "--help" ]]
}

@test "install-toolchain.sh --dry-run 显示安装计划" {
    run "$TEST_ROOT/.specify/scripts/setup/install-toolchain.sh" --dry-run
    [ "$status" -eq 0 ]
    [[ "$output" =~ "DRY RUN" ]]
    [[ "$output" =~ "将要下载" ]]
}

@test "平台检测功能正常工作" {
    source "$TEST_ROOT/.specify/scripts/common.sh"
    run detect_platform
    [ "$status" -eq 0 ]
    # 输出应该是 linux-x86_64, darwin-x86_64, 或 win32
    [[ "$output" =~ ^(linux-x86_64|darwin-x86_64|win32|linux-aarch64)$ ]]
}

@test "SHA256 校验函数存在且可调用" {
    source "$TEST_ROOT/.specify/scripts/setup/install-toolchain.sh"

    # 创建测试文件
    echo "test content" > "$TEST_TEMP_DIR/test.txt"

    # 测试 calculate_sha256 函数
    run calculate_sha256 "$TEST_TEMP_DIR/test.txt"
    [ "$status" -eq 0 ]
    [ -n "$output" ]
    # SHA256 应该是 64 个十六进制字符
    [[ "$output" =~ ^[a-f0-9]{64}$ ]]
}

@test "配置文件存在且格式正确" {
    [ -f "$TEST_ROOT/.specify/config/toolchain.json" ]

    # 验证 JSON 格式
    run jq -e '.name' "$TEST_ROOT/.specify/config/toolchain.json"
    [ "$status" -eq 0 ]

    run jq -e '.platforms' "$TEST_ROOT/.specify/config/toolchain.json"
    [ "$status" -eq 0 ]
}
