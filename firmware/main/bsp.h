/**
 * \file bsp.h
 * \brief 
 *
 * \author Stanley Arnaud 
 * \date 05/26/2026
 * \version 0
 */

#ifndef BSP_H
#define BSP_H

/* --------------------------------------------------------------------------
 * Includes
 * ------------------------------------------------------------------------- */
#include "driver/button.h"
#include "driver/display.h"
#include "driver/rotary_encoder.h"

#include "driver/i2c_types.h"
#include "driver/pulse_cnt.h"
#include "driver/synth.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* -------------------------------------------------------------------------
 * Type Definitions
 * ------------------------------------------------------------------------- */
struct bsp_ctx_s
{
    struct
    {
        i2c_master_bus_handle_t i2c;
    } shared;

    struct
    {
        display_t display;
        synth_t *synth;
        re_t re;
        btn_t trigger;
    } devs;

    struct
    {
        uint8_t mac[6];
    } info;
};

// typedef enum
// {
//     BSP_DEVICE_DISPLAY,
//     BSP_DEVICE_RE,
//     BSP_DEVICE_SYNTH,
//     BSP_DEVICE_TRIGGER,

//     BSP_DEVICE_COUNT,
// } bsp_device_type_t;

/**
 * \brief BSP context (read-only)
 */
extern struct bsp_ctx_s bsp;

/* -------------------------------------------------------------------------
 * Function Declarations
 * ------------------------------------------------------------------------- */
/**
 * \brief Initialize shared MCU peripherals (I2C bus, ...)
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_FAIL
 */
int bsp_shared_init(void);
int bsp_display_init(void);
int bsp_input_init(void);
int bsp_synth_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !BSP_H */
