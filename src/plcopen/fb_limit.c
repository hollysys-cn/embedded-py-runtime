/**
 * @file fb_limit.c
 * @brief PLCopen 限幅器实现
 */

#include "plcopen/fb_limit.h"
#include <string.h>

int FB_LIMIT_Init(FB_LIMIT_t* fb, const FB_LIMIT_Config_t* config) {
    if (fb == NULL || config == NULL) return -1;
    if (config->max_val <= config->min_val) return -1;

    memcpy(&fb->config, config, sizeof(FB_LIMIT_Config_t));
    fb->state.status = FB_STATUS_OK;
    return 0;
}

float FB_LIMIT_Execute(FB_LIMIT_t* fb, float input) {
    if (check_nan_inf(input)) {
        fb->state.status = check_nan(input) ? FB_STATUS_ERROR_NAN : FB_STATUS_ERROR_INF;
        return 0.0f;
    }

    if (input > fb->config.max_val) {
        fb->state.status = FB_STATUS_LIMIT_HI;
        return fb->config.max_val;
    } else if (input < fb->config.min_val) {
        fb->state.status = FB_STATUS_LIMIT_LO;
        return fb->config.min_val;
    }

    fb->state.status = FB_STATUS_OK;
    return input;
}
