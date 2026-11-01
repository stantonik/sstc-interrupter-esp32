/**
 * \file constants.h
 * \brief 
 *
 * \author Stanley Arnaud 
 * \date 05/21/2026
 * \version 0
 */

#ifndef BSP_CONSTANTS_H
#define BSP_CONSTANTS_H

/* --------------------------------------------------------------------------
 * Includes
 * ------------------------------------------------------------------------- */
#include "hal/i2c_types.h"

/* --------------------------------------------------------------------------
 * Macros and Constants
 * ------------------------------------------------------------------------- */

/* I2C parameters */
#define I2C_PORT           I2C_NUM_0

/* SSD1306 max I2C freq is 400.000Hz (fast-mode)
 */
#define I2C_MASTER_FREQ_HZ (400 * 1000)

#define SSD1306_I2C_ADDR   (0x3C)

#define DISPLAY_HOR_RES    (128)
#define DISPLAY_VER_RES    (64)

#endif /* !BSP_CONSTANTS_H */
