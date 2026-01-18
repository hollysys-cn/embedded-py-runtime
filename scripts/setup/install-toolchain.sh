#!/usr/bin/env bash
# -*- coding: utf-8 -*-
#
# ARM GCC 工具链自动安装脚本
# 支持从官方源或国内镜像源下载并验证工具链
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
TOOLCHAIN_DIR="$PROJECT_ROOT/.toolchain"

#######################################
# 显示使用帮助
#######################################
show_help() {
    cat << EOF
用法: $(basename "$0") [选项]

自动下载、验证并安装 ARM GCC 工具链

选项:
    --platform <PLATFORM>  指定平台 (linux-x86_64|darwin-x86_64|win32)
                          默认自动检测
    --version <VERSION>    指定工具链版本，默认从 toolchain.json 读取
    --mirror <MIRROR>      使用镜像源 (tsinghua|aliyun|ustc|official)
                          默认按优先级尝试
    --force               强制重新下载，即使已安装
    --dry-run             仅显示将要执行的操作，不实际下载
    --no-color            禁用彩色输出
    --help                显示此帮助信息

示例:
    $(basename "$0")                          # 自动检测平台并安装
    $(basename "$0") --mirror tsinghua        # 使用清华镜像源
    $(basename "$0") --force                  # 强制重新安装
    $(basename "$0") --dry-run                # 预览安装操作

退出码:
    0 - 成功
    1 - 参数错误
    2 - 下载失败
    3 - 校验失败
    4 - 解压失败

EOF
}

#######################################
# 计算文件的 SHA256 校验和
# Arguments:
#   $1 - 文件路径
# Returns:
#   输出 SHA256 值
#######################################
calculate_sha256() {
    local file="$1"

    if command_exists sha256sum; then
        sha256sum "$file" | awk '{print $1}'
    elif command_exists shasum; then
        shasum -a 256 "$file" | awk '{print $1}'
    else
        log_error "未找到 sha256sum 或 shasum 命令"
        return 1
    fi
}

#######################################
# 验证文件的 SHA256 校验和
# Arguments:
#   $1 - 文件路径
#   $2 - 期望的 SHA256 值
# Returns:
#   0 验证成功, 1 验证失败
#######################################
verify_sha256() {
    local file="$1"
    local expected="$2"

    log_info "验证 SHA256 校验和..."

    local actual
    actual="$(calculate_sha256 "$file")"

    if [[ "$actual" == "$expected" ]]; then
        log_success "校验成功"
        return 0
    else
        log_error "SHA256 校验失败"
        log_error "  期望: $expected"
        log_error "  实际: $actual"
        return 1
    fi
}

#######################################
# 下载文件
# Arguments:
#   $1 - URL
#   $2 - 输出文件路径
# Returns:
#   0 成功, 1 失败
#######################################
download_file() {
    local url="$1"
    local output="$2"

    log_info "下载: $url"

    if command_exists wget; then
        wget --progress=bar:force -O "$output" "$url" 2>&1 || return 1
    elif command_exists curl; then
        curl -# -L -o "$output" "$url" || return 1
    else
        log_error "未找到 wget 或 curl 命令"
        return 1
    fi

    log_success "下载完成: $output"
    return 0
}

#######################################
# 解压工具链
# Arguments:
#   $1 - 压缩文件路径
#   $2 - 输出目录
#   $3 - 文件类型 (tar.bz2|zip)
# Returns:
#   0 成功, 1 失败
#######################################
extract_toolchain() {
    local archive="$1"
    local output_dir="$2"
    local file_type="$3"

    log_info "解压: $archive"

    mkdir -p "$output_dir"

    case "$file_type" in
        tar.bz2)
            if ! command_exists tar; then
                log_error "未找到 tar 命令"
                return 1
            fi
            tar -xjf "$archive" -C "$output_dir" --strip-components=1 || return 1
            ;;
        zip)
            if ! command_exists unzip; then
                log_error "未找到 unzip 命令"
                return 1
            fi
            local temp_dir
            temp_dir="$(mktemp -d)"
            unzip -q "$archive" -d "$temp_dir" || return 1
            # 移动解压后的内容（去掉顶层目录）
            local inner_dir
            inner_dir="$(find "$temp_dir" -mindepth 1 -maxdepth 1 -type d | head -n 1)"
            if [[ -n "$inner_dir" ]]; then
                mv "$inner_dir"/* "$output_dir/" || return 1
            fi
            rm -rf "$temp_dir"
            ;;
        *)
            log_error "不支持的文件类型: $file_type"
            return 1
            ;;
    esac

    log_success "解压完成"
    return 0
}

#######################################
# 主函数
#######################################
main() {
    local platform=""
    local version=""
    local mirror=""
    local force=false
    local dry_run=false

    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --platform)
                platform="$2"
                shift 2
                ;;
            --version)
                version="$2"
                shift 2
                ;;
            --mirror)
                mirror="$2"
                shift 2
                ;;
            --force)
                force=true
                shift
                ;;
            --dry-run)
                dry_run=true
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

    # 检查必需命令
    if ! check_required_commands jq; then
        log_error "请安装 jq 命令"
        exit 1
    fi

    # 自动检测平台
    if [[ -z "$platform" ]]; then
        platform="$(detect_platform)"
        log_info "检测到平台: $platform"
    fi

    # 读取工具链配置
    local toolchain_config="$CONFIG_DIR/toolchain.json"
    if [[ ! -f "$toolchain_config" ]]; then
        log_error "未找到工具链配置文件: $toolchain_config"
        exit 1
    fi

    # 获取工具链信息
    local install_path
    install_path="$(jq -r '.install_path' "$toolchain_config")"
    local full_install_path="$PROJECT_ROOT/$install_path"

    # 检查是否已安装
    if [[ -d "$full_install_path" ]] && [[ "$force" == false ]]; then
        log_info "工具链已安装: $full_install_path"
        log_info "使用 --force 选项强制重新安装"
        exit 0
    fi

    # 获取平台特定配置
    local url
    local sha256
    url="$(jq -r ".platforms.\"$platform\".url" "$toolchain_config")"
    sha256="$(jq -r ".platforms.\"$platform\".sha256" "$toolchain_config")"

    if [[ "$url" == "null" ]] || [[ -z "$url" ]]; then
        log_error "不支持的平台: $platform"
        exit 1
    fi

    log_info "工具链版本: $(jq -r '.version' "$toolchain_config")"
    log_info "目标平台: $platform"

    if [[ "$dry_run" == true ]]; then
        log_info "[DRY RUN] 将要下载: $url"
        log_info "[DRY RUN] SHA256: $sha256"
        log_info "[DRY RUN] 安装到: $full_install_path"
        exit 0
    fi

    # 创建临时下载目录
    local download_dir
    download_dir="$(mktemp -d)"
    trap 'rm -rf "$download_dir"' EXIT

    # 确定文件名和类型
    local filename
    filename="$(basename "$url")"
    local file_type
    if [[ "$filename" == *.tar.bz2 ]]; then
        file_type="tar.bz2"
    elif [[ "$filename" == *.zip ]]; then
        file_type="zip"
    else
        log_error "不支持的文件格式: $filename"
        exit 1
    fi

    local download_file="$download_dir/$filename"

    # 下载文件
    if ! download_file "$url" "$download_file"; then
        log_error "下载失败"
        exit 2
    fi

    # 验证 SHA256
    if ! verify_sha256 "$download_file" "$sha256"; then
        exit 3
    fi

    # 清理旧安装
    if [[ -d "$full_install_path" ]]; then
        log_info "清理旧安装..."
        rm -rf "$full_install_path"
    fi

    # 解压
    if ! extract_toolchain "$download_file" "$full_install_path" "$file_type"; then
        log_error "解压失败"
        exit 4
    fi

    # 创建安装标记
    echo "$(date -u +"%Y-%m-%dT%H:%M:%SZ")" > "$full_install_path/.installed"

    log_success "工具链安装完成: $full_install_path"
    log_info ""
    log_info "下一步:"
    log_info "  1. 运行验证脚本: .specify/scripts/setup/verify-toolchain.sh"
    log_info "  2. 配置环境: source .specify/scripts/setup/configure-environment.sh"

    return 0
}

main "$@"
