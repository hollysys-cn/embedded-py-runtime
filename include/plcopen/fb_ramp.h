/**
 * @file fb_ramp.h
 * @brief PLCopen 斜坡发生器功能块
 */

#ifndef PLCOPEN_FB_RAMP_H
#define PLCOPEN_FB_RAMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plcopen/common.h"

typedef struct {
    float rise_rate;       /**< 上升速率（单位/秒，> 0） */
    float fall_rate;       /**< 下降速率（单位/秒，> 0） */
    float sample_time;     /**< 采样周期（秒） */
} FB_RAMP_Config_t;

typedef struct {
    float output;          /**< 当前输出值 */
    bool first_run;        /**< 首次运行标志 */
    FB_Status_t status;    /**< 状态码 */
} FB_RAMP_State_t;

typedef struct {
    FB_RAMP_Config_t config; /**< 配置参数 */
    FB_RAMP_State_t state;   /**< 运行时状态 */
} FB_RAMP_t;

/**
 * @brief 初始化 RAMP 斜坡发生器
 *
 * @param fb RAMP 功能块实例指针
 * @param config 配置参数指针
 * @return int 返回码：0=成功，-1=配置错误
 */
int FB_RAMP_Init(FB_RAMP_t* fb, const FB_RAMP_Config_t* config);

/**
 * @brief 执行 RAMP 斜坡发生器
 *
 * 根据上升/下降速率限制输出向目标值逼近。
 *
 * @param fb RAMP 功能块实例指针
 * @param target 目标值
 * @return float 当前输出值
 */
float FB_RAMP_Execute(FB_RAMP_t* fb, float target);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_RAMP_H */
