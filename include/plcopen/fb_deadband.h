/**
 * @file fb_deadband.h
 * @brief PLCopen 死区处理功能块
 */

#ifndef PLCOPEN_FB_DEADBAND_H
#define PLCOPEN_FB_DEADBAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "plcopen/common.h"

typedef struct {
    float width;  /**< 死区宽度（半宽，>= 0） */
    float center; /**< 死区中心值 */
} FB_DEADBAND_Config_t;

typedef struct {
    FB_Status_t status; /**< 状态码 */
} FB_DEADBAND_State_t;

typedef struct {
    FB_DEADBAND_Config_t config; /**< 配置参数 */
    FB_DEADBAND_State_t state;   /**< 运行时状态 */
} FB_DEADBAND_t;

/**
 * @brief 初始化 DEADBAND 死区处理
 *
 * @param fb DEADBAND 功能块实例指针
 * @param config 配置参数指针
 * @return int 返回码：0=成功，-1=配置错误
 */
int FB_DEADBAND_Init(FB_DEADBAND_t* fb, const FB_DEADBAND_Config_t* config);

/**
 * @brief 执行 DEADBAND 死区处理
 *
 * 如果 |input - center| <= width，则输出 center；否则输出 input。
 *
 * @param fb DEADBAND 功能块实例指针
 * @param input 输入值
 * @return float 处理后的输出值
 */
float FB_DEADBAND_Execute(FB_DEADBAND_t* fb, float input);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_DEADBAND_H */
