# CMake Toolchain File for ARM Cortex-M4
# 用于交叉编译 ARM Cortex-M4 目标平台的工具链配置
#
# 使用方式:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=.specify/templates/cmake/toolchain-arm-cortex-m4.cmake
#
# 编码: UTF-8
# 换行符: LF

# 设置系统信息
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# 禁用编译器检查（交叉编译时需要）
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 查找工具链路径
if(DEFINED ENV{ARM_TOOLCHAIN_PATH})
    set(TOOLCHAIN_PREFIX "$ENV{ARM_TOOLCHAIN_PATH}/bin/arm-none-eabi-")
    message(STATUS "使用环境变量中的工具链: $ENV{ARM_TOOLCHAIN_PATH}")
else()
    # 尝试在项目目录中查找
    get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
    if(EXISTS "${PROJECT_ROOT}/.toolchain/gcc-arm-none-eabi/bin")
        set(TOOLCHAIN_PREFIX "${PROJECT_ROOT}/.toolchain/gcc-arm-none-eabi/bin/arm-none-eabi-")
        message(STATUS "使用项目内工具链: ${PROJECT_ROOT}/.toolchain/gcc-arm-none-eabi")
    else()
        # 假设工具链在系统 PATH 中
        set(TOOLCHAIN_PREFIX "arm-none-eabi-")
        message(STATUS "使用系统 PATH 中的工具链")
    endif()
endif()

# 设置编译器
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)

# 设置工具
set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

# C11 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Cortex-M4 特定编译标志
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

# 通用编译标志
set(COMMON_FLAGS "${CPU_FLAGS} -fdata-sections -ffunction-sections")

# 警告标志
set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Wshadow")

# C 编译标志
set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} ${WARNING_FLAGS} -std=c11")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3" CACHE STRING "C Debug flags")
set(CMAKE_C_FLAGS_RELEASE "-O2 -g0 -DNDEBUG" CACHE STRING "C Release flags")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -g0 -DNDEBUG" CACHE STRING "C MinSizeRel flags")

# C++ 编译标志（如果需要）
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} ${WARNING_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3" CACHE STRING "C++ Debug flags")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g0 -DNDEBUG" CACHE STRING "C++ Release flags")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -g0 -DNDEBUG" CACHE STRING "C++ MinSizeRel flags")

# 汇编标志
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}" CACHE STRING "ASM flags")

# 链接器标志
set(LINKER_FLAGS "${CPU_FLAGS} --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${LINKER_FLAGS}" CACHE STRING "Linker flags")

# 设置查找根路径（仅在交叉编译的根目录中查找）
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 生成编译数据库（用于 IDE 支持）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "======================================")
message(STATUS "ARM Cortex-M4 工具链配置")
message(STATUS "======================================")
message(STATUS "系统名称: ${CMAKE_SYSTEM_NAME}")
message(STATUS "处理器: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "C 编译器: ${CMAKE_C_COMPILER}")
message(STATUS "C 标准: C${CMAKE_C_STANDARD}")
message(STATUS "CPU 标志: ${CPU_FLAGS}")
message(STATUS "======================================")
