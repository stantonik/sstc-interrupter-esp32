/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file knobs.h
 * @brief 
 *
 * 
 *
 * @author Stanley Arnaud 
 * @date 10/26/2025
 * @version 0
 */

#ifndef KNOBS_H
#define KNOBS_H

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

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------
typedef enum
{
    KNOB_PD = 0,
    KNOB_PRF,
    KNOB_PWR,
    KNOB_GDB,
    KNOB_COUNT
} knobs_t;

typedef uint8_t knobs_mask_t;
typedef void (* knobs_on_change_cb_t)(knobs_mask_t knobs, const int16_t *values[]);

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t knobs_init(void);
esp_err_t knobs_set_on_change_cb(knobs_on_change_cb_t cb);
esp_err_t knobs_get_values(knobs_mask_t knobs, const int16_t *values[]);
uint16_t knobs_get_current_step(void);

#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !KNOBS_H */

