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
    float width;
    float center;
} FB_DEADBAND_Config_t;

typedef struct {
    FB_Status_t status;
} FB_DEADBAND_State_t;

typedef struct {
    FB_DEADBAND_Config_t config;
    FB_DEADBAND_State_t state;
} FB_DEADBAND_t;

int FB_DEADBAND_Init(FB_DEADBAND_t* fb, const FB_DEADBAND_Config_t* config);
float FB_DEADBAND_Execute(FB_DEADBAND_t* fb, float input);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_DEADBAND_H */
