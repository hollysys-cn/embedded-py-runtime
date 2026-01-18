# CMakeLists.txt for blinky Example
# LED 闪烁示例 - 完整的嵌入式项目
#
# 使用方式:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=../../templates/cmake/toolchain-arm-cortex-m4.cmake
#   cmake --build build
#
# 编码: UTF-8
# 换行符: LF

cmake_minimum_required(VERSION 3.20)

# 项目信息
project(blinky
    VERSION 1.0.0
    DESCRIPTION "LED Blinky Example for STM32F407"
    LANGUAGES C ASM
)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "Build type" FORCE)
endif()

# 源文件
set(SOURCE_FILES
    main.c
    startup.s
)

# 链接脚本
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)

# 创建可执行文件
add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

# C11 标准
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# 编译定义
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    STM32F407xx
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:RELEASE>
)

# 链接脚本
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--print-memory-usage
)

# 生成 HEX 文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMENT "生成 HEX 文件: ${PROJECT_NAME}.hex"
)

# 生成 BIN 文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "生成 BIN 文件: ${PROJECT_NAME}.bin"
)

# 显示代码大小
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} --format=berkeley
        $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "代码大小统计:"
)

# 生成反汇编文件（用于调试）
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -h -S
        $<TARGET_FILE:${PROJECT_NAME}.elf>
        > ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.lst
    COMMENT "生成反汇编文件: ${PROJECT_NAME}.lst"
)

# 打印配置
message(STATUS "===================================")
message(STATUS "blinky 项目配置")
message(STATUS "===================================")
message(STATUS "目标 MCU: STM32F407VGT6")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "链接脚本: ${LINKER_SCRIPT}")
message(STATUS "C 编译器: ${CMAKE_C_COMPILER}")
message(STATUS "===================================")
