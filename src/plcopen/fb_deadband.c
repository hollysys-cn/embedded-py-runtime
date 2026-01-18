/**
 * @file fb_deadband.c
 * @brief PLCopen 死区处理实现
 */

#include "plcopen/fb_deadband.h"
#include <string.h>
#include <math.h>

int FB_DEADBAND_Init(FB_DEADBAND_t* fb, const FB_DEADBAND_Config_t* config) {
    if (fb == NULL || config == NULL) return -1;
    if (config->width < 0.0f) return -1;

    memcpy(&fb->config, config, sizeof(FB_DEADBAND_Config_t));
    fb->state.status = FB_STATUS_OK;
    return 0;
}

float FB_DEADBAND_Execute(FB_DEADBAND_t* fb, float input) {
    if (check_nan_inf(input)) {
        fb->state.status = check_nan(input) ? FB_STATUS_ERROR_NAN : FB_STATUS_ERROR_INF;
        return fb->config.center;
    }

    float deviation = fabsf(input - fb->config.center);

    if (deviation <= fb->config.width) {
        fb->state.status = FB_STATUS_OK;
        return fb->config.center;
    }

    fb->state.status = FB_STATUS_OK;
    return input;
}
