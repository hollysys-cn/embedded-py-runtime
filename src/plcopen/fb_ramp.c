/**
 * @file fb_ramp.c
 * @brief PLCopen 斜坡发生器实现
 */

#include "plcopen/fb_ramp.h"
#include <string.h>
#include <math.h>

int FB_RAMP_Init(FB_RAMP_t* fb, const FB_RAMP_Config_t* config) {
    if (fb == NULL || config == NULL) return -1;
    if (config->rise_rate <= 0.0f || config->fall_rate <= 0.0f) return -1;
    if (config->sample_time <= 0.0f || config->sample_time >= MAX_SAMPLE_TIME) return -1;

    memcpy(&fb->config, config, sizeof(FB_RAMP_Config_t));
    fb->state.output = 0.0f;
    fb->state.first_run = true;
    fb->state.status = FB_STATUS_OK;
    return 0;
}

float FB_RAMP_Execute(FB_RAMP_t* fb, float target) {
    if (check_nan_inf(target)) {
        fb->state.status = check_nan(target) ? FB_STATUS_ERROR_NAN : FB_STATUS_ERROR_INF;
        return fb->state.output;
    }

    if (fb->state.first_run) {
        fb->state.output = target;
        fb->state.first_run = false;
        fb->state.status = FB_STATUS_OK;
        return target;
    }

    float error = target - fb->state.output;
    float rate = (error > 0.0f) ? fb->config.rise_rate : fb->config.fall_rate;
    float max_change = rate * fb->config.sample_time;

    if (fabsf(error) <= max_change) {
        fb->state.output = target;
    } else {
        fb->state.output += (error > 0.0f) ? max_change : -max_change;
    }

    fb->state.status = FB_STATUS_OK;
    return fb->state.output;
}
