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
    float min_val;
    float max_val;
} FB_LIMIT_Config_t;

typedef struct {
    FB_Status_t status;
} FB_LIMIT_State_t;

typedef struct {
    FB_LIMIT_Config_t config;
    FB_LIMIT_State_t state;
} FB_LIMIT_t;

int FB_LIMIT_Init(FB_LIMIT_t* fb, const FB_LIMIT_Config_t* config);
float FB_LIMIT_Execute(FB_LIMIT_t* fb, float input);

#ifdef __cplusplus
}
#endif

#endif /* PLCOPEN_FB_LIMIT_H */
