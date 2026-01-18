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
    float output;
    bool first_run;
    FB_Status_t status;
} FB_RAMP_State_t;

typedef struct {
    FB_RAMP_Config_t config;
    FB_RAMP_State_t state;
} FB_RAMP_t;

int FB_RAMP_Init(FB_RAMP_t* fb, const FB_RAMP_Config_t* config);
float FB_RAMP_Execute(FB_RAMP_t* fb, float target);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_RAMP_H */
