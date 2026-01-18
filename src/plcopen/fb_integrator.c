/**
 * @file fb_integrator.c
 * @brief PLCopen 积分器实现
 */

#include "plcopen/fb_integrator.h"
#include <string.h>

int FB_INTEGRATOR_Init(FB_INTEGRATOR_t* fb, const FB_INTEGRATOR_Config_t* config) {
    if (fb == NULL || config == NULL) return -1;
    if (config->sample_time <= 0.0f || config->sample_time >= MAX_SAMPLE_TIME) return -1;
    if (config->enable_limit && config->out_max <= config->out_min) return -1;

    memcpy(&fb->config, config, sizeof(FB_INTEGRATOR_Config_t));
    fb->state.integral = 0.0f;
    fb->state.status = FB_STATUS_OK;
    return 0;
}

float FB_INTEGRATOR_Execute(FB_INTEGRATOR_t* fb, float input) {
    if (check_nan_inf(input)) {
        fb->state.status = check_nan(input) ? FB_STATUS_ERROR_NAN : FB_STATUS_ERROR_INF;
        return fb->state.integral;
    }

    fb->state.integral += input * fb->config.sample_time;

    if (fb->config.enable_limit) {
        if (fb->state.integral > fb->config.out_max) {
            fb->state.integral = fb->config.out_max;
            fb->state.status = FB_STATUS_LIMIT_HI;
        } else if (fb->state.integral < fb->config.out_min) {
            fb->state.integral = fb->config.out_min;
            fb->state.status = FB_STATUS_LIMIT_LO;
        } else {
            fb->state.status = FB_STATUS_OK;
        }
    } else {
        fb->state.status = FB_STATUS_OK;
    }

    return fb->state.integral;
}

void FB_INTEGRATOR_Reset(FB_INTEGRATOR_t* fb) {
    fb->state.integral = 0.0f;
    fb->state.status = FB_STATUS_OK;
}
