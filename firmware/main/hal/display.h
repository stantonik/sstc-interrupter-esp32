/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file display.h
 * @brief 
 *
 * 
 *
 * @author Stanley Arnaud 
 * @date 10/24/2025
 * @version 0
 */

#ifndef DISPLAY_H
#define DISPLAY_H

// clang-format off
#ifdef __cplusplus
extern "C" 
{
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "esp_err.h"
#include "esp_lcd_types.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 128

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------
typedef struct
{
    esp_lcd_panel_handle_t panel_handle;
    esp_lcd_panel_io_handle_t io_handle;
} display_handles_t;

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t display_init(void);
display_handles_t display_get_handles(void);

#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !DISPLAY_H */

