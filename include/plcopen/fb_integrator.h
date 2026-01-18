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
    float sample_time;
    float out_min;
    float out_max;
    bool enable_limit;
} FB_INTEGRATOR_Config_t;

typedef struct {
    float integral;
    FB_Status_t status;
} FB_INTEGRATOR_State_t;

typedef struct {
    FB_INTEGRATOR_Config_t config;
    FB_INTEGRATOR_State_t state;
} FB_INTEGRATOR_t;

int FB_INTEGRATOR_Init(FB_INTEGRATOR_t* fb, const FB_INTEGRATOR_Config_t* config);
float FB_INTEGRATOR_Execute(FB_INTEGRATOR_t* fb, float input);
void FB_INTEGRATOR_Reset(FB_INTEGRATOR_t* fb);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_INTEGRATOR_H */
