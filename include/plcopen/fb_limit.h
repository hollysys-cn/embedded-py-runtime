/**
 * @file fb_limit.h
 * @brief PLCopen 限幅器功能块
 */

#ifndef PLCOPEN_FB_LIMIT_H
#define PLCOPEN_FB_LIMIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plcopen/common.h"

typedef struct {
    float min_val; /**< 输出下限 */
    float max_val; /**< 输出上限（必须 > min_val） */
} FB_LIMIT_Config_t;

typedef struct {
    FB_Status_t status; /**< 状态码 (OK, LIMIT_HI, LIMIT_LO) */
} FB_LIMIT_State_t;

typedef struct {
    FB_LIMIT_Config_t config; /**< 配置参数 */
    FB_LIMIT_State_t state;   /**< 运行时状态 */
} FB_LIMIT_t;

/**
 * @brief 初始化 LIMIT 限幅器
 *
 * @param fb LIMIT 功能块实例指针
 * @param config 配置参数指针
 * @return int 返回码：0=成功，-1=配置错误
 */
int FB_LIMIT_Init(FB_LIMIT_t* fb, const FB_LIMIT_Config_t* config);

/**
 * @brief 执行 LIMIT 限幅器
 *
 * 将输入值限制在 [min, max] 范围内。
 *
 * @param fb LIMIT 功能块实例指针
 * @param input 输入值
 * @return float 限幅后的输出值
 */
float FB_LIMIT_Execute(FB_LIMIT_t* fb, float input);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_LIMIT_H */
