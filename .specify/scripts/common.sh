#!/usr/bin/env bash
# -*- coding: utf-8 -*-
#
# 通用工具函数库
# 提供平台检测、日志输出等通用功能
#
# 编码: UTF-8
# 换行符: LF

set -euo pipefail

#######################################
# 检测当前运行平台
# Globals:
#   无
# Arguments:
#   无
# Returns:
#   输出平台标识: linux-x86_64, darwin-x86_64, win32
#######################################
detect_platform() {
    local os_name
    local arch

    # 检测操作系统
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        os_name="linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        os_name="darwin"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
        os_name="win32"
        echo "$os_name"
        return 0
    else
        log_error "不支持的操作系统: $OSTYPE"
        return 1
    fi

    # 检测架构
    arch="$(uname -m)"
    case "$arch" in
        x86_64|amd64)
            arch="x86_64"
            ;;
        aarch64|arm64)
            arch="aarch64"
            ;;
        *)
            log_error "不支持的架构: $arch"
            return 1
            ;;
    esac

    echo "${os_name}-${arch}"
}

#######################################
# 检查是否支持彩色输出
# Globals:
#   NO_COLOR - 如果设置，禁用彩色
# Arguments:
#   无
# Returns:
#   0 支持彩色, 1 不支持
#######################################
supports_color() {
    # 如果设置了 NO_COLOR 环境变量，禁用彩色
    if [[ -n "${NO_COLOR:-}" ]]; then
        return 1
    fi

    # 检查是否连接到终端
    if [[ ! -t 1 ]]; then
        return 1
    fi

    # 检查 TERM 变量
    if [[ "${TERM:-}" == "dumb" ]]; then
        return 1
    fi

    return 0
}

# 颜色定义
if supports_color; then
    COLOR_RED='\033[0;31m'
    COLOR_GREEN='\033[0;32m'
    COLOR_YELLOW='\033[1;33m'
    COLOR_BLUE='\033[0;34m'
    COLOR_CYAN='\033[0;36m'
    COLOR_RESET='\033[0m'
else
    COLOR_RED=''
    COLOR_GREEN=''
    COLOR_YELLOW=''
    COLOR_BLUE=''
    COLOR_CYAN=''
    COLOR_RESET=''
fi

#######################################
# 输出信息日志
# Arguments:
#   $@ - 日志消息
#######################################
log_info() {
    echo -e "${COLOR_BLUE}[INFO]${COLOR_RESET} $*"
}

#######################################
# 输出警告日志
# Arguments:
#   $@ - 日志消息
#######################################
log_warn() {
    echo -e "${COLOR_YELLOW}[WARN]${COLOR_RESET} $*" >&2
}

#######################################
# 输出错误日志
# Arguments:
#   $@ - 日志消息
#######################################
log_error() {
    echo -e "${COLOR_RED}[ERROR]${COLOR_RESET} $*" >&2
}

#######################################
# 输出成功日志
# Arguments:
#   $@ - 日志消息
#######################################
log_success() {
    echo -e "${COLOR_GREEN}[OK]${COLOR_RESET} $*"
}

#######################################
# 输出调试日志（仅在 DEBUG=1 时）
# Arguments:
#   $@ - 日志消息
#######################################
log_debug() {
    if [[ "${DEBUG:-0}" == "1" ]]; then
        echo -e "${COLOR_CYAN}[DEBUG]${COLOR_RESET} $*" >&2
    fi
}

#######################################
# 检查命令是否存在
# Arguments:
#   $1 - 命令名称
# Returns:
#   0 存在, 1 不存在
#######################################
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

#######################################
# 检查必需命令
# Arguments:
#   $@ - 命令列表
# Returns:
#   0 所有命令存在, 1 有命令缺失
#######################################
check_required_commands() {
    local missing_commands=()

    for cmd in "$@"; do
        if ! command_exists "$cmd"; then
            missing_commands+=("$cmd")
        fi
    done

    if [[ ${#missing_commands[@]} -gt 0 ]]; then
        log_error "缺少必需命令: ${missing_commands[*]}"
        return 1
    fi

    return 0
}

#######################################
# 获取脚本目录
# Returns:
#   输出脚本所在目录的绝对路径
#######################################
get_script_dir() {
    local source="${BASH_SOURCE[0]}"
    while [[ -h "$source" ]]; do
        local dir
        dir="$(cd -P "$(dirname "$source")" && pwd)"
        source="$(readlink "$source")"
        [[ $source != /* ]] && source="$dir/$source"
    done
    cd -P "$(dirname "$source")" && pwd
}

#######################################
# 获取项目根目录
# Returns:
#   输出项目根目录的绝对路径
#######################################
get_project_root() {
    local script_dir
    script_dir="$(get_script_dir)"
    # 假设此脚本在 .specify/scripts/ 下
    cd "$script_dir/../.." && pwd
}
