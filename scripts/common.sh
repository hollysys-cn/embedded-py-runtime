#!/usr/bin/env bash
#
# 公共工具函数库
# 功能: 提供跨平台的日志、命令检查、路径处理等工具函数
#
# 使用方式:
#   source scripts/common.sh
#
# 编码: UTF-8
# 换行符: LF

# 颜色输出（除非 NO_COLOR 已设置）
if [[ -z "${NO_COLOR:-}" ]] && [[ -t 1 ]]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m'
else
    RED='' GREEN='' YELLOW='' BLUE='' NC=''
fi

#######################################
# 日志函数
#######################################

log_info() {
    echo -e "${BLUE}[INFO]${NC} $*" >&2
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $*" >&2
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*" >&2
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $*" >&2
}

#######################################
# 检查命令是否存在
# 参数:
#   $1 - 命令名
# 返回:
#   0 - 存在
#   1 - 不存在
#######################################
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

#######################################
# 检查必需命令
# 参数:
#   $@ - 命令列表
# 返回:
#   0 - 所有命令都存在
#   1 - 有命令缺失
#######################################
check_required_commands() {
    local missing=()
    
    for cmd in "$@"; do
        if ! command_exists "$cmd"; then
            missing+=("$cmd")
        fi
    done
    
    if [[ ${#missing[@]} -gt 0 ]]; then
        log_error "缺少必需命令: ${missing[*]}"
        return 1
    fi
    
    return 0
}

#######################################
# 获取脚本所在目录
# 返回: 脚本目录绝对路径
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
# 返回: 项目根目录绝对路径
#######################################
get_project_root() {
    local script_dir
    script_dir="$(get_script_dir)"
    # 假设脚本在 scripts/ 子目录中
    cd "$script_dir/.." && pwd
}

#######################################
# 检测操作系统平台
# 输出: linux-x86_64, darwin-x86_64, win32
#######################################
detect_platform() {
    local os arch
    
    case "$OSTYPE" in
        linux*)
            os="linux"
            ;;
        darwin*)
            os="darwin"
            ;;
        msys*|mingw*|cygwin*|win32)
            os="win32"
            ;;
        *)
            log_error "不支持的操作系统: $OSTYPE"
            return 1
            ;;
    esac
    
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
    
    echo "${os}-${arch}"
}

#######################################
# 规范化路径（Windows 路径转 Unix 路径）
# 参数:
#   $1 - 路径
# 输出: 规范化后的路径
#######################################
normalize_path() {
    local path="$1"
    
    # Windows 路径转换 (C:\ -> /c/)
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "mingw"* ]]; then
        path="$(cygpath -u "$path" 2>/dev/null || echo "$path")"
    fi
    
    echo "$path"
}
