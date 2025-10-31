/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file pwm.h
 * @brief
 *
 *
 *
 * @author Stanley Arnaud
 * @date 10/25/2025
 * @version 0
 */

#ifndef PWM_H
#define PWM_H

// clang-format off
#ifdef __cplusplus
extern "C" 
{
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "esp_err.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
// PWM Modulation Configuration
#define PWM_MOD_DUTY_RES_BITS   (8)        // 8-bit duty resolution (LEDC_TIMER_8_BIT)
#define PWM_MOD_CARRIER_FREQ_HZ   (30000)    // 30 kHz PWM carrier frequency
#define PWM_MOD_DUTY_MAX   ((1U << PWM_MOD_DUTY_RES_BITS) - 1)

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------
typedef enum
{
    PWM_MODE_MANUAL = 1,
    PWM_MODE_MODULATION
} pwm_mode_t;

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t pwm_init(void);

esp_err_t pwm_manual_update(float freq_hz, uint16_t pulse_width_us);
esp_err_t pwm_modulation_update(uint8_t dutycycle);
esp_err_t pwm_enable(void);
esp_err_t pwm_disable(void);

esp_err_t pwm_set_mode(pwm_mode_t mode);


#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !PWM_H */
