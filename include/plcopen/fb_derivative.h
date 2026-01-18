/**
 * @file fb_derivative.h
 * @brief PLCopen 微分器功能块
 */

#ifndef PLCOPEN_FB_DERIVATIVE_H
#define PLCOPEN_FB_DERIVATIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plcopen/common.h"

typedef struct {
    float sample_time;           /**< 采样周期（秒，> 0） */
    float filter_time_constant;  /**< 滤波时间常数（>= 0，0 表示无滤波） */
} FB_DERIVATIVE_Config_t;

typedef struct {
    float prev_input;       /**< 上次输入值 */
    float filtered_output;  /**< 滤波后的导数值 */
    bool first_run;         /**< 首次运行标志 */
    FB_Status_t status;     /**< 状态码 */
} FB_DERIVATIVE_State_t;

typedef struct {
    FB_DERIVATIVE_Config_t config; /**< 配置参数 */
    FB_DERIVATIVE_State_t state;   /**< 运行时状态 */
} FB_DERIVATIVE_t;

/**
 * @brief 初始化 DERIVATIVE 微分器
 *
 * @param fb DERIVATIVE 功能块实例指针
 * @param config 配置参数指针
 * @return int 返回码：0=成功，-1=配置错误
 */
int FB_DERIVATIVE_Init(FB_DERIVATIVE_t* fb, const FB_DERIVATIVE_Config_t* config);

/**
 * @brief 执行 DERIVATIVE 微分器
 *
 * out = (input - input_prev) / dt
 * 可选支持对输出进行一阶低通滤波。
 *
 * @param fb DERIVATIVE 功能块实例指针
 * @param input 输入值
 * @return float 当前微分值
 */
float FB_DERIVATIVE_Execute(FB_DERIVATIVE_t* fb, float input);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_DERIVATIVE_H */
