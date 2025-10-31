/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file usb.h
 * @brief 
 *
 * 
 *
 * @author Stanley Arnaud 
 * @date 10/24/2025
 * @version 0
 */

#ifndef USB_H
#define USB_H

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

// -----------------------------------------------------------------------------
// Inline Function Definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function Declarations
// -----------------------------------------------------------------------------
esp_err_t usb_init_serial(void);
esp_err_t usb_init_host(void);
esp_err_t usb_free_serial(void);

#ifdef __cplusplus
}
#endif
// clang-format on

#endif /* !USB_H */

