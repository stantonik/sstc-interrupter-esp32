/**
 * \file display.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-10
 */

#ifndef DISPLAY_H
#define DISPLAY_H

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
/**
 * \brief Configuration parameters if display for display_init function
 */
typedef struct
{
    uint16_t hor_res;      /*!< Horizontal resolution */
    uint16_t ver_res;      /*!< Vertical resolution */
    size_t bits_per_pixel; /* Pixel bit-width */
    int8_t res_pin;        /*!< Reset GPIO pin (optional) */
    void *io_handle;       /*!< Generic IO device handle */
} display_config_t;

/**
 * \brief Callback triggered after a color transaction started by display_draw_bitmap function is done
 */
typedef void (*display_color_trans_done_cb_t)(void *arg);

/**
 * \brief Display internal data (shall not access)
 */
typedef struct
{
    display_config_t config;

    struct
    {
        bool initialized;
        void *panel_handle;
        display_color_trans_done_cb_t color_trans_done_cb;
        void *color_trans_done_cb_arg;
    } state;
} display_t;

/* --------------------------------------------------------------------------
 * Function Declarations
 * -------------------------------------------------------------------------- */
/**
 * \brief Initialize display
 *
 * \param[in] config Pointer to configuration structure
 * \param[in] display Display handle
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_ARG
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_TIMEOUT: transaction failed
 *     - RET_ERR_FAIL
 */
int display_init(const display_config_t *config, display_t *display);

/**
 * \brief Deinitialize display
 *
 * \param[in] display Display handle
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_ARG
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_TIMEOUT: transaction failed
 */
int display_deinit(display_t *display);

/**
 * \brief Draw bitmap
 *
 * \param[in] display Display handle
 * \param[in] x_start Start pixel index in the target frame buffer, on x-axis (x_start is included)
 * \param[in] y_start Start pixel index in the target frame buffer, on y-axis (y_start is included)
 * \param[in] x_end End pixel index in the target frame buffer, on x-axis (x_end is not included)
 * \param[in] y_end End pixel index in the target frame buffer, on y-axis (y_end is not included)
 * \param[in] data RGB color data that will be dumped to the specific window range
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_ARG
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_TIMEOUT: transaction failed
 */
int display_draw_bitmap(
    display_t *display, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end,
    const void *data
);

/**
 * \brief Turn display on
 *
 * \param[in] display Display handle
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_ARG
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_TIMEOUT: transaction failed
 */
int display_turn_on(display_t *display);

/**
 * \brief Turn display off
 *
 * \param[in] display Display handle
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_ARG
 *     - RET_ERR_INVALID_STATE
 *     - RET_ERR_TIMEOUT: transaction failed
 */
int display_turn_off(display_t *display);

/**
 * \brief Set the callback that will trigger after the transaction started by display_draw_bitmap will be done
 *
 * \param[in] display Display handle
 * \param[in] cb Callback
 * \param[in] arg Argument passed to callback
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_ARG
 *     - RET_ERR_INVALID_STATE
 */
int
display_set_color_trans_done_cb(display_t *display, display_color_trans_done_cb_t cb, void *arg);

/**
 * \brief Get display resolution
 *
 * \param[in] display Display handle
 * \param[in] hor_res Pointer to horizontal resolution output
 * \param[in] ver_res Pointer to vertical resolution output
 *
 * \return
 *     - RET_OK
 *     - RET_ERR_INVALID_ARG
 *     - RET_ERR_INVALID_STATE
 */
int display_get_color_res(display_t *display, int *hor_res, int *ver_res);

/* --------------------------------------------------------------------------
 * Inline Function Definitions
 * -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !DISPLAY_H */
