/**
 * @file fb_pt1.h
 * @brief PLCopen 一阶惯性（PT1）滤波器功能块
 * @author Hollysys Embedded Team
 * @date 2026-01-18
 *
 * 实现一阶惯性滤波器（PT1 = Proportional Time-delay of 1st order）。
 * 用于信号平滑和高频噪声抑制。
 *
 * 传递函数：H(s) = 1 / (τs + 1)
 * 离散化：使用前向欧拉法
 *
 * 典型应用：
 * - 传感器信号滤波
 * - 噪声抑制
 * - 信号平滑
 */

#ifndef PLCOPEN_FB_PT1_H
#define PLCOPEN_FB_PT1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plcopen/common.h"

typedef struct {
    float time_constant;   /**< 时间常数 τ（秒，>= 1e-6） */
    float sample_time;     /**< 采样周期（秒，> 0 且 < 1000） */
} FB_PT1_Config_t;

typedef struct {
    float output;          /**< 当前输出值 */
    bool first_run;        /**< 首次运行标志 */
    FB_Status_t status;    /**< 状态码 */
} FB_PT1_State_t;

typedef struct {
    FB_PT1_Config_t config;
    FB_PT1_State_t state;
} FB_PT1_t;

FB_Status_t FB_PT1_Init(FB_PT1_t* fb, const FB_PT1_Config_t* config);
float FB_PT1_Execute(FB_PT1_t* fb, float input);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_PT1_H */
