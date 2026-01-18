/**
 * @file fb_integrator.h
 * @brief PLCopen 积分器功能块
 */

#ifndef PLCOPEN_FB_INTEGRATOR_H
#define PLCOPEN_FB_INTEGRATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plcopen/common.h"

typedef struct {
    float sample_time; /**< 采样周期（秒，> 0） */
    float out_min;     /**< 输出下限（enable_limit 为 true 时有效） */
    float out_max;     /**< 输出上限（enable_limit 为 true 时有效） */
    bool enable_limit; /**< 是否启用输出限幅 */
} FB_INTEGRATOR_Config_t;

typedef struct {
    float integral;     /**< 当前积分值 */
    FB_Status_t status; /**< 状态码 */
} FB_INTEGRATOR_State_t;

typedef struct {
    FB_INTEGRATOR_Config_t config; /**< 配置参数 */
    FB_INTEGRATOR_State_t state;   /**< 运行时状态 */
} FB_INTEGRATOR_t;

/**
 * @brief 初始化 INTEGRATOR 积分器
 *
 * @param fb INTEGRATOR 功能块实例指针
 * @param config 配置参数指针
 * @return int 返回码：0=成功，-1=配置错误
 */
int FB_INTEGRATOR_Init(FB_INTEGRATOR_t* fb, const FB_INTEGRATOR_Config_t* config);

/**
 * @brief 执行 INTEGRATOR 积分器
 *
 * out = out_prev + input * dt
 *
 * @param fb INTEGRATOR 功能块实例指针
 * @param input 输入值
 * @return float 当前积分值
 */
float FB_INTEGRATOR_Execute(FB_INTEGRATOR_t* fb, float input);

/**
 * @brief 复位积分器
 *
 * 将积分值重置为 0。
 *
 * @param fb INTEGRATOR 功能块实例指针
 */
void FB_INTEGRATOR_Reset(FB_INTEGRATOR_t* fb);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_INTEGRATOR_H */
