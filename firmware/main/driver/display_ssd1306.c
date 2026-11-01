/**
 * \file display_ssd1315.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-10
 */

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include "system/error.h"
#include "display.h"

#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_ssd1306.h"

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Static Variables
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Static Function Declarations
 * -------------------------------------------------------------------------- */
static bool io_panel_color_trans_done_cb(
    esp_lcd_panel_io_handle_t io_panel, esp_lcd_panel_io_event_data_t *edata, void *user_ctx
);

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
int
display_init(const display_config_t *config, display_t *display)
{
    // TODO: check config

    esp_err_t ret;
    esp_lcd_panel_dev_config_t panel_config = {0};
    esp_lcd_panel_ssd1306_config_t ssd1306_config = {0};
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_io_callbacks_t cbs = {0};

    // Initialize display panel
    panel_config.bits_per_pixel = config->bits_per_pixel;
    panel_config.reset_gpio_num = config->res_pin;
    ssd1306_config.height = config->ver_res;
    panel_config.vendor_config = &ssd1306_config;
    ret = esp_lcd_new_panel_ssd1306(config->io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK)
    {
        ret = RET_ERR_FAIL;
        goto init_cleanup;
    }

    cbs.on_color_trans_done = io_panel_color_trans_done_cb;
    ret = esp_lcd_panel_io_register_event_callbacks(config->io_handle, &cbs, display);
    if (ret != ESP_OK)
    {
        ret = RET_ERR_FAIL;
        goto init_cleanup;
    }

    // Initialization sequence
    ret = esp_lcd_panel_reset(panel_handle);
    if (ret != ESP_OK)
    {
        ret = RET_ERR_FAIL;
        goto init_cleanup;
    }
    ret = esp_lcd_panel_init(panel_handle);
    if (ret != ESP_OK)
    {
        ret = RET_ERR_TIMEOUT;
        goto init_cleanup;
    }
    // ret = esp_lcd_panel_set_gap(panel_handle, 32, 0);
    // if (ret != ESP_OK)
    // {
    //     ret = RET_ERR_TIMEOUT;
    //     goto init_cleanup;
    // }

    display->config = *config;
    display->state.panel_handle = panel_handle;
    display->state.initialized = true;

    return RET_OK;

init_cleanup:
    if (panel_handle) esp_lcd_panel_del(panel_handle);
    return ret;
}

int
display_deinit(display_t *display)
{
    esp_err_t ret;

    ret = esp_lcd_panel_disp_on_off(display->state.panel_handle, false);
    if (ret != ESP_OK)
    {
        return RET_ERR_TIMEOUT;
    }

    esp_lcd_panel_del(display->state.panel_handle);

    display->state.initialized = false;

    return RET_OK;
}

int display_turn_on(display_t *display)
{
    esp_err_t ret;

    ret = esp_lcd_panel_disp_on_off(display->state.panel_handle, true);
    if (ret != ESP_OK)
    {
        return RET_ERR_TIMEOUT;
    }

    return RET_OK;
}

int display_turn_off(display_t *display)
{
    esp_err_t ret;

    ret = esp_lcd_panel_disp_on_off(display->state.panel_handle, false);
    if (ret != ESP_OK)
    {
        return RET_ERR_TIMEOUT;
    }

    return RET_OK;
}

int
display_draw_bitmap(
    display_t *display, int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end,
    const void *data
)
{
    esp_err_t res;

    res = esp_lcd_panel_draw_bitmap(
        display->state.panel_handle, x_start, y_start, x_end, y_end, data
    );
    if (res != ESP_OK)
    {
        return RET_ERR_TIMEOUT;
    }

    return RET_OK;
}

int
display_set_color_trans_done_cb(display_t *display, display_color_trans_done_cb_t cb, void *arg)
{
    display->state.color_trans_done_cb_arg = arg;
    display->state.color_trans_done_cb = cb;

    return RET_OK;
}

int
display_get_color_res(display_t *display, int *hor_res, int *ver_res)
{
    *hor_res = display->config.hor_res;
    *ver_res = display->config.ver_res;

    return RET_OK;
}

/* --------------------------------------------------------------------------
 * Static Function Definitions
 * -------------------------------------------------------------------------- */
static bool
io_panel_color_trans_done_cb(
    esp_lcd_panel_io_handle_t io_panel, esp_lcd_panel_io_event_data_t *edata, void *user_ctx
)
{
    display_t *display = user_ctx;

    if (display != NULL && display->state.initialized && display->state.color_trans_done_cb)
    {
        display->state.color_trans_done_cb(display->state.color_trans_done_cb_arg);
    }

    return false;
}
