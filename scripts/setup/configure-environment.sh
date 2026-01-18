#!/usr/bin/env bash
# -*- coding: utf-8 -*-
#
# 环境变量配置脚本
# 配置工具链路径和环境变量
#
# 编码: UTF-8
# 换行符: LF
#
# 用法: source scripts/setup/configure-environment.sh

# 注意：此脚本必须使用 source 或 . 执行

# 检测脚本是否被 source 执行
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "[ERROR] 此脚本必须使用 source 或 . 执行"
    echo "用法: source ${0}"
    exit 1
fi

# 获取脚本目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 加载通用函数库
# shellcheck source=../common.sh
source "$SCRIPT_DIR/../common.sh"

PROJECT_ROOT="$(get_project_root)"
CONFIG_DIR="$PROJECT_ROOT/config"

#######################################
# 配置环境
#######################################
configure_environment() {
    # 检查必需命令
    if ! command_exists jq; then
        log_error "请安装 jq 命令"
        return 1
    fi

    # 读取工具链配置
    local toolchain_config="$CONFIG_DIR/toolchain.json"
    if [[ ! -f "$toolchain_config" ]]; then
        log_error "未找到工具链配置文件: $toolchain_config"
        return 1
    fi

    # 获取安装路径
    local install_path
    install_path="$(jq -r '.install_path' "$toolchain_config")"
    local full_install_path="$PROJECT_ROOT/$install_path"

    # 检查工具链是否安装
    if [[ ! -d "$full_install_path" ]]; then
        log_error "工具链未安装: $full_install_path"
        log_info "请先运行: scripts/setup/install-toolchain.sh"
        return 1
    fi

    log_info "工具链路径: $full_install_path"

    # 添加到 PATH
    local bin_dir="$full_install_path/bin"
    if [[ -d "$bin_dir" ]]; then
        if [[ ":$PATH:" != *":$bin_dir:"* ]]; then
            export PATH="$bin_dir:$PATH"
            log_info "已添加到 PATH: $bin_dir"
        else
            log_info "PATH 中已包含: $bin_dir"
        fi
    else
        log_error "未找到 bin 目录: $bin_dir"
        return 1
    fi

    # 设置工具链环境变量
    export ARM_TOOLCHAIN_PATH="$full_install_path"
    log_info "ARM_TOOLCHAIN_PATH=$ARM_TOOLCHAIN_PATH"

    # 设置 CMake 工具链文件
    local cmake_toolchain_file="$PROJECT_ROOT/templates/cmake/toolchain-arm-cortex-m4.cmake"
    if [[ -f "$cmake_toolchain_file" ]]; then
        export CMAKE_TOOLCHAIN_FILE="$cmake_toolchain_file"
        log_info "CMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE"
    fi

    # 创建便捷别名
    alias arm-gcc='arm-none-eabi-gcc'
    alias arm-g++='arm-none-eabi-g++'
    alias arm-gdb='arm-none-eabi-gdb'
    alias arm-objdump='arm-none-eabi-objdump'
    alias arm-size='arm-none-eabi-size'

    log_success "环境配置完成"
    log_info ""
    log_info "可用命令:"
    log_info "  arm-none-eabi-gcc (或 arm-gcc)"
    log_info "  arm-none-eabi-g++ (或 arm-g++)"
    log_info "  arm-none-eabi-gdb (或 arm-gdb)"
    log_info "  arm-none-eabi-objdump (或 arm-objdump)"
    log_info "  arm-none-eabi-size (或 arm-size)"

    return 0
}

# 执行配置
configure_environment
