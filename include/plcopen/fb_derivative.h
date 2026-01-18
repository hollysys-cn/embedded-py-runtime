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
    float sample_time;
    float filter_time_constant;  /**< 滤波时间常数（0 表示无滤波） */
} FB_DERIVATIVE_Config_t;

typedef struct {
    float prev_input;
    float filtered_output;
    bool first_run;
    FB_Status_t status;
} FB_DERIVATIVE_State_t;

typedef struct {
    FB_DERIVATIVE_Config_t config;
    FB_DERIVATIVE_State_t state;
} FB_DERIVATIVE_t;

int FB_DERIVATIVE_Init(FB_DERIVATIVE_t* fb, const FB_DERIVATIVE_Config_t* config);
float FB_DERIVATIVE_Execute(FB_DERIVATIVE_t* fb, float input);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_DERIVATIVE_H */
