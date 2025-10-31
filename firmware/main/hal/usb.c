/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file usb.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/24/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "usb.h"
#include "driver/usb_serial_jtag.h"
#include "esp_check.h"
#include "esp_log.h"
#include "usb/usb_host.h"

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "usb_system"

#define HOST_TASK_PRIO 2
#define HOST_TASK_STACK (5 * 1024)

// -----------------------------------------------------------------------------
// Private Typedef
// -----------------------------------------------------------------------------
typedef struct
{
    TaskHandle_t parent_task_handle;
    esp_err_t *ret_ptr;
} usbh_args_t;

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static TaskHandle_t usb_host_task_handle;

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static void usb_host_task(void *pvParams)
{
    usbh_args_t *args = pvParams;
    esp_err_t ret;

    ESP_LOGI(TAG, "Installing USB Host Library");
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    ret = usb_host_install(&host_config);
    if (ret != ESP_OK)
    {
        *args->ret_ptr = ret;
        ESP_LOGE(TAG, "Failed to install USB Host Library");
        vTaskSuspend(NULL);
    }

    xTaskNotifyGive(args->parent_task_handle);

    bool has_clients = true;
    bool has_devices = false;
    while (has_clients)
    {
        uint32_t event_flags;
        ret = usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        if (ret != ESP_OK)
        {
            ESP_LOGW(TAG, "USB Host failed to handle events");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
        {
            if (ESP_OK == usb_host_device_free_all())
                has_clients = false;
            else
                has_devices = true;
        }
        if (has_devices && event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
        {
            has_clients = false;
        }
    }

    ret = usb_host_uninstall();
    if (ret != ESP_OK)
    {
        *args->ret_ptr = ret;
        ESP_LOGE(TAG, "Failed to uninstall USB Host Library");
    }

    vTaskSuspend(NULL);
}

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t usb_init_serial(void)
{
    usb_serial_jtag_driver_config_t sj_conf = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(usb_serial_jtag_driver_install(&sj_conf), TAG, "Failed to install serial bridge driver");

    ESP_LOGI(TAG, "Serial bridge driver installation succeeded");

    return ESP_OK;
}

esp_err_t usb_free_serial(void)
{
    ESP_RETURN_ON_ERROR(usb_serial_jtag_driver_uninstall(), TAG, "Failed to uninstall serial bridge");
    return ESP_OK;
}

esp_err_t usb_init_host(void)
{
    esp_err_t ret = ESP_OK;
    usbh_args_t args = { .parent_task_handle=xTaskGetCurrentTaskHandle(), .ret_ptr=&ret};
    BaseType_t task_created = xTaskCreatePinnedToCore(
        usb_host_task, "usb_host", HOST_TASK_STACK, &args, HOST_TASK_PRIO, &usb_host_task_handle, 0);
    ESP_RETURN_ON_FALSE(task_created == pdPASS, ESP_ERR_NO_MEM, TAG, "Failed to create USB host task");

    ESP_RETURN_ON_FALSE(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) == pdPASS, ESP_ERR_TIMEOUT, TAG, "USB host task did not initialize in time");
    ESP_RETURN_ON_ERROR(ret, TAG, "");

    ESP_LOGI(TAG, "USB Host driver installation succeeded");

    return ESP_OK;
}
