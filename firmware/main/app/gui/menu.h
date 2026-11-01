/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file menu.h
 * @brief 
 *
 * 
 *
 * @author Stanley Arnaud 
 * @date 10/24/2025
 * @version 0
 */

#ifndef MENU_H
#define MENU_H

// clang-format off
#ifdef __cplusplus
extern "C" 
{
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "esp_err.h"
#include <stdbool.h>

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Type Definitions
// -----------------------------------------------------------------------------
typedef enum
{
    MENU_MODE_MANUAL,
    MENU_MODE_AUDIO_JACK,
    MENU_MODE_MIDI
} menu_mode_t;

typedef enum
{
    MENU_STATE_IDLE = (1 << 0),
    MENU_STATE_ARMED = (1 << 1)
} menu_state_t;

typedef uint8_t menu_state_mask_t;

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t menu_init(void);
esp_err_t menu_set_mode(menu_mode_t m, bool msg_box);
menu_mode_t menu_get_mode(void);
esp_err_t menu_set_state(menu_state_t s);
void menu_set_header_text(const char *text);
void menu_display_msg_box(const char *msg, uint16_t duration_ms);
void menu_hide_msg_box(void);

#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !MENU_H */

