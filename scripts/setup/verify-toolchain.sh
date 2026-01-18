#!/usr/bin/env bash
# -*- coding: utf-8 -*-
#
# ARM GCC 工具链验证脚本
# 验证工具链是否正确安装并支持 C11 和 Cortex-M4
#
# 编码: UTF-8
# 换行符: LF

set -euo pipefail

# 加载通用函数库
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../common.sh
source "$SCRIPT_DIR/../common.sh"

PROJECT_ROOT="$(get_project_root)"
CONFIG_DIR="$PROJECT_ROOT/config"

#######################################
# 显示使用帮助
#######################################
show_help() {
    cat << EOF
用法: $(basename "$0") [选项]

验证 ARM GCC 工具链是否正确安装

选项:
    --verbose    显示详细验证信息
    --no-color   禁用彩色输出
    --help       显示此帮助信息

退出码:
    0 - 验证通过
    1 - 工具链未安装
    2 - 工具链版本不匹配
    3 - 必要工具缺失

EOF
}

#######################################
# 测试 C11 特性支持
# Arguments:
#   $1 - GCC 路径
# Returns:
#   0 支持, 1 不支持
#######################################
test_c11_support() {
    local gcc="$1"

    log_info "测试 C11 特性支持..."

    # 创建临时测试文件
    local test_file
    test_file="$(mktemp --suffix=.c)"
    trap 'rm -f "$test_file" "${test_file%.c}.o"' RETURN

    cat > "$test_file" << 'EOF'
#include <stddef.h>

// 测试 _Static_assert
_Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");

// 测试匿名结构体
struct {
    union {
        int x;
        float y;
    };
    int z;
} test_struct;

// 测试 _Alignas
_Alignas(16) char aligned_buffer[64];

int main(void) {
    // 测试 _Generic
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

    # 尝试编译
    if "$gcc" -std=c11 -c "$test_file" -o "${test_file%.c}.o" 2>/dev/null; then
        log_success "C11 特性支持: 已启用"
        return 0
    else
        log_error "C11 特性支持: 未启用或不完整"
        return 1
    fi
}

#######################################
# 测试 Cortex-M4 支持
# Arguments:
#   $1 - GCC 路径
# Returns:
#   0 支持, 1 不支持
#######################################
test_cortex_m4_support() {
    local gcc="$1"

    log_info "测试 Cortex-M4 支持..."

    # 创建临时测试文件
    local test_file
    test_file="$(mktemp --suffix=.c)"
    trap 'rm -f "$test_file" "${test_file%.c}.o"' RETURN

    cat > "$test_file" << 'EOF'
int main(void) { return 0; }
EOF

    # 尝试为 Cortex-M4 编译
    if "$gcc" -mcpu=cortex-m4 -mthumb -c "$test_file" -o "${test_file%.c}.o" 2>/dev/null; then
        log_success "Cortex-M4 支持: 已启用"
        return 0
    else
        log_error "Cortex-M4 支持: 编译失败"
        return 1
    fi
}

#######################################
# 主函数
#######################################
main() {
    local verbose=false

    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --verbose)
                verbose=true
                shift
                ;;
            --no-color)
                export NO_COLOR=1
                shift
                ;;
            --help)
                show_help
                exit 0
                ;;
            *)
                log_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done

    # 读取工具链配置
    local toolchain_config="$CONFIG_DIR/toolchain.json"
    if [[ ! -f "$toolchain_config" ]]; then
        log_error "未找到工具链配置文件: $toolchain_config"
        exit 1
    fi

    # 检查必需命令
    if ! check_required_commands jq; then
        log_error "请安装 jq 命令"
        exit 1
    fi

    # 获取安装路径
    local install_path
    install_path="$(jq -r '.install_path' "$toolchain_config")"
    local full_install_path="$PROJECT_ROOT/$install_path"

    # 检查工具链是否安装
    if [[ ! -d "$full_install_path" ]]; then
        log_error "工具链未安装: $full_install_path"
        log_info "请先运行: scripts/setup/install-toolchain.sh"
        exit 1
    fi

    log_info "验证工具链: $full_install_path"

    local errors=0

    # 获取可执行文件配置
    local executables
    executables="$(jq -r '.executables | to_entries | .[] | "\(.key)=\(.value)"' "$toolchain_config")"

    # 验证每个可执行文件
    while IFS= read -r line; do
        # 移除可能的回车符和空白
        line="$(echo "$line" | tr -d '\r' | xargs)"
        [[ -z "$line" ]] && continue

        local name="${line%%=*}"
        local path="${line#*=}"
        local full_path="$full_install_path/$path"

        if [[ -f "$full_path" ]] && [[ -x "$full_path" ]]; then
            if [[ "$verbose" == true ]]; then
                local version
                version="$("$full_path" --version 2>/dev/null | head -n 1 || echo "未知")"
                log_success "$name: $version"
            else
                log_success "$name: $(basename "$full_path")"
            fi
        else
            log_error "$name: 未找到或不可执行 ($full_path)"
            ((errors++))
        fi
    done <<< "$executables"

    if [[ $errors -gt 0 ]]; then
        log_error "发现 $errors 个错误"
        exit 3
    fi

    # 测试 C11 支持
    local gcc_path="$full_install_path/$(jq -r '.executables.gcc' "$toolchain_config")"
    if ! test_c11_support "$gcc_path"; then
        exit 3
    fi

    # 测试 Cortex-M4 支持
    if ! test_cortex_m4_support "$gcc_path"; then
        exit 3
    fi

    log_success "验证通过 ✓"

    return 0
}

main "$@"
