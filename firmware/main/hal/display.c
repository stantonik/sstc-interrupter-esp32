/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file display.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/24/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "display.h"
#include "driver/i2c.h"
#include "esp_check.h"
#include "esp_lcd_io_i2c.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "sdkconfig.h"
#include "esp_log.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "display"

#define I2C_HOST 0
#define LCD_PIXEL_CLOCK_HZ (400 * 1000)
#define PIN_NUM_SDA CONFIG_INTERRUPTER_PIN_SDA
#define PIN_NUM_SCL CONFIG_INTERRUPTER_PIN_SCL
#define PIN_NUM_RST -1
#define I2C_HW_ADDR 0x3C

#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static display_handles_t handles = {0};

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static esp_err_t check_i2c_device(i2c_port_t i2c_num, uint8_t addr)
{
    uint8_t dummy = 0;
    esp_err_t ret = i2c_master_write_to_device(i2c_num, addr, &dummy, 0, 100 / portTICK_PERIOD_MS);
    return ret;
}

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t
display_init(void)
{
    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_NUM_SDA,
        .scl_io_num = PIN_NUM_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = LCD_PIXEL_CLOCK_HZ,
    };
    i2c_param_config(I2C_HOST, &i2c_conf);
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_HOST, I2C_MODE_MASTER, 0, 0, 0), TAG, "Failed to install I2C driver");
    
    ESP_RETURN_ON_ERROR(check_i2c_device(I2C_HOST, I2C_HW_ADDR), TAG, "Display not found on 0x%02X", I2C_HW_ADDR);

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = I2C_HW_ADDR,
        .control_phase_bytes = 1,       // According to SSD1306 datasheet
        .lcd_cmd_bits = LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = LCD_CMD_BITS, // According to SSD1306 datasheet
        .dc_bit_offset = 6,             // According to SSD1306 datasheet
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(I2C_HOST, &io_config, &io_handle), TAG, "Failed to install panel IO");

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = PIN_NUM_RST,
    };
    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = DISPLAY_HEIGHT,
    };
    panel_config.vendor_config = &ssd1306_config;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle), TAG, "Failed to install SSD1306 panel driver");
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);
    esp_lcd_panel_invert_color(panel_handle, true);

    handles.io_handle = io_handle;
    handles.panel_handle = panel_handle;
    
    ESP_LOGI(TAG, "Initializaion succeeded");

    return ESP_OK;
}

inline display_handles_t display_get_handles(void) { return handles; }
