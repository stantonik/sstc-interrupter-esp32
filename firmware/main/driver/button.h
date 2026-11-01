/**
 * \file button.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-11-01
 */

#ifndef BUTTON_H
#define BUTTON_H

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef struct
{
    int gpio;
    bool active_high;
} btn_config_t;

typedef struct
{
    btn_config_t cfg;
} btn_t;

/* --------------------------------------------------------------------------
 * Function Declarations
 * -------------------------------------------------------------------------- */
int btn_init(const btn_config_t *config, btn_t *btn);
bool btn_is_pressed(btn_t *btn);

/* --------------------------------------------------------------------------
 * Inline Function Definitions
 * -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !BUTTON_H */
