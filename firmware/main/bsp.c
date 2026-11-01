/**
 * \file bsp.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-09
 */

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include "bsp.h"
#include "bsp/constants.h"
#include "bsp/pins.h"
#include "driver/button.h"
#include "driver/display.h"
#include "system/error.h"
#include "system/logging.h"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_io_i2c.h"
#include "esp_mac.h"
#include "nvs.h"
#include "nvs_flash.h"

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */
#define TAG "bsp"

/* --------------------------------------------------------------------------
 * Global Variables
 * -------------------------------------------------------------------------- */
struct bsp_ctx_s bsp = {0};

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
int
bsp_shared_init(void)
{
    int ret;
    i2c_master_bus_config_t i2c_cfg = {0};

    /* Get MAC addr */
    esp_read_mac(bsp.info.mac, ESP_MAC_BASE);

    /* Initialize NVS */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        LOG_WARN(TAG, "NVS erased");
        ret = nvs_flash_init();
    }

    if (ret != ESP_OK)
    {
        LOG_ERROR(TAG, "failed to initialize NVS");
        return RET_ERR_FAIL;
    }

    /* Initialize shared peripherals */
    /* Initialize I2C bus */
    i2c_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_cfg.i2c_port = I2C_PORT;
    i2c_cfg.scl_io_num = I2C_SCL_PIN;
    i2c_cfg.sda_io_num = I2C_SDA_PIN;
    i2c_cfg.glitch_ignore_cnt = 7;
    // i2c_cfg.trans_queue_depth = 10;
    ret = i2c_new_master_bus(&i2c_cfg, &bsp.shared.i2c);
    if (ret != ESP_OK)
    {
        LOG_ERROR(TAG, "failed to initialize I2C bus");
        return RET_ERR_FAIL;
    }
    LOG_INFO(TAG, "I2C bus initialized");

    return RET_OK;
}

int
bsp_synth_init(void)
{
    int ret;
    synth_config_t synth_cfg = {};

    synth_cfg.output_gpio = OUTPUT_PIN;
    if ((ret = synth_create(&synth_cfg, &bsp.devs.synth)) != RET_OK)
    {
        return ret;
    }

    LOG_INFO(TAG, "synth initialized");

    return RET_OK;
}

int
bsp_display_init(void)
{
    int ret;
    esp_lcd_panel_io_i2c_config_t io_config = {0};
    esp_lcd_panel_io_handle_t io_handle = NULL;
    display_config_t disp_config = {0};

    /* Add display I2C device on bus */
    io_config.dev_addr = SSD1306_I2C_ADDR;
    io_config.scl_speed_hz = I2C_MASTER_FREQ_HZ;
    io_config.control_phase_bytes = 1; // According to SSD1306/SSD1315 datasheet
    io_config.lcd_cmd_bits = 8;        // According to SSD1306/SSD1315 datasheet
    io_config.lcd_param_bits = 8;      // According to SSD1306/SSD1315 datasheet
    io_config.dc_bit_offset = 6;       // According to SSD1306/SSD1315 datasheet
    io_config.user_ctx = &bsp.devs.display;
    ret = esp_lcd_new_panel_io_i2c(bsp.shared.i2c, &io_config, &io_handle);
    if (ret != ESP_OK)
    {
        LOG_ERROR(TAG, "failed to add display on the I2C bus");
        return RET_ERR_FAIL;
    }

    /* Initialize display device */
    disp_config.hor_res = DISPLAY_HOR_RES;
    disp_config.ver_res = DISPLAY_VER_RES;
    disp_config.bits_per_pixel = 1;
    disp_config.res_pin = -1;
    disp_config.io_handle = io_handle;
    ret = display_init(&disp_config, &bsp.devs.display);
    if (ret != RET_OK)
    {
        LOG_ERROR(TAG, "failed to initialize display");
        return RET_ERR_FAIL;
    }

    LOG_INFO(TAG, "display initialized");

    return RET_OK;
}

int
bsp_input_init()
{
    int ret;
    re_config_t re_cfg = {0};
    btn_config_t trigger_cfg = {0};

    // Initialize rotary encoder
    re_cfg.a_gpio = ROTARY_ENCODER_A_PIN;
    re_cfg.b_gpio = ROTARY_ENCODER_B_PIN;
    re_cfg.sw_gpio = ROTARY_ENCODER_SW_PIN;
    ret = re_init(&re_cfg, &bsp.devs.re);
    if (ret != RET_OK)
    {
        LOG_ERROR(TAG, "failed to initialize rotary encoder");
        return RET_ERR_FAIL;
    }

    trigger_cfg.gpio = TRIGGER_PIN;
    trigger_cfg.active_high = false;
    ret = btn_init(&trigger_cfg, &bsp.devs.trigger);
    if (ret != RET_OK)
    {
        LOG_ERROR(TAG, "failed to initialize trigger");
        return RET_ERR_FAIL;
    }

    LOG_INFO(TAG, "inputs initialized");

    return RET_OK;
}

/* --------------------------------------------------------------------------
 * Static Function Definitions
 * -------------------------------------------------------------------------- */
