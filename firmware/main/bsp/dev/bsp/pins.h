/**
 * \file pins.h
 * \brief 
 *
 * \author Stanley Arnaud 
 * \date 05/20/2026
 * \version 0
 */

#ifndef BSP_PINS_H
#define BSP_PINS_H

/* --------------------------------------------------------------------------
 * Includes
 * ------------------------------------------------------------------------- */
#include "hal/adc_types.h"
#include "soc/gpio_num.h"

/* --------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/* Rotary encoder module pins */
#define ROTARY_ENCODER_A_PIN  GPIO_NUM_9
#define ROTARY_ENCODER_B_PIN  GPIO_NUM_7
#define ROTARY_ENCODER_SW_PIN GPIO_NUM_4

#define TRIGGER_PIN           GPIO_NUM_1

#define OUTPUT_PIN            GPIO_NUM_8

/* I2C bus pins (only port 0 is used) */
#define I2C_SCL_PIN           GPIO_NUM_6
#define I2C_SDA_PIN           GPIO_NUM_5

#endif /* !BSP_PINS_H */
