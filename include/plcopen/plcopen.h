/**
 * @file plcopen.h
 * @brief PLCopen 基础功能块库 - 主头文件
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 * @version 1.0.0
 *
 * 本文件提供 PLCopen 标准功能块库的统一接口入口。
 * 包含所有功能块的头文件，用户只需包含此文件即可使用全部功能。
 *
 * 支持的功能块：
 * - FB_PID: PID 控制器（比例-积分-微分控制）
 * - FB_PT1: 一阶惯性滤波器（信号平滑）
 * - FB_RAMP: 斜坡发生器（平滑设定值变化）
 * - FB_LIMIT: 限幅器（输出信号限制）
 * - FB_DEADBAND: 死区处理（消除微小波动）
 * - FB_INTEGRATOR: 积分器（累计量计算）
 * - FB_DERIVATIVE: 微分器（变化率计算）
 *
 * 使用示例：
 * @code
 * #include <plcopen/plcopen.h>
 *
 * FB_PID_t my_pid;
 * FB_PID_Config_t pid_config = {
 *     .kp = 1.0f,
 *     .ki = 0.1f,
 *     .kd = 0.05f,
 *     .sample_time = 0.01f,
 *     .out_min = 0.0f,
 *     .out_max = 100.0f,
 *     .int_min = -50.0f,
 *     .int_max = 50.0f
 * };
 *
 * FB_PID_Init(&my_pid, &pid_config);
 * float output = FB_PID_Execute(&my_pid, 50.0f, 45.0f);
 * @endcode
 *
 * @note 所有功能块都是线程不安全的，需要用户自行保证线程同步
 * @note 所有功能块均不使用动态内存分配，适合实时系统
 */

#ifndef PLCOPEN_H
#define PLCOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 通用定义和工具函数 */
#include "plcopen/common.h"

/* 功能块头文件 */
#include "plcopen/fb_pid.h"
#include "plcopen/fb_pt1.h"
#include "plcopen/fb_ramp.h"
#include "plcopen/fb_limit.h"
#include "plcopen/fb_deadband.h"
#include "plcopen/fb_integrator.h"
#include "plcopen/fb_derivative.h"

/* 版本信息 */
#define PLCOPEN_VERSION_MAJOR 1
#define PLCOPEN_VERSION_MINOR 0
#define PLCOPEN_VERSION_PATCH 0
#define PLCOPEN_VERSION "1.0.0"

/**
 * @brief 获取库版本字符串
 * @return 版本字符串（如 "1.0.0"）
 */
static inline const char* plcopen_get_version(void) {
    return PLCOPEN_VERSION;
}

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_H */
