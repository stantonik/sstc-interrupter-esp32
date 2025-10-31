/*
 * Copyright (C) 2025 Stanley Arnaud <stantonik@stantonik-mba.local>
 *
 * Distributed under terms of the MIT license.
 */

/**
 * @file usb_midi.c
 * @brief
 *
 * @author Stanley Arnaud
 * @date 10/24/2025
 * @version 0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "usb_midi.h"
#include "core/event_bus.h"
#include "esp_check.h"
#include "usb/usb_host.h"
#include <math.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Macros and Constants
// -----------------------------------------------------------------------------
#define TAG "usb_midi_client"

#define ACTION_OPEN_DEV (1 << 1)
#define ACTION_CLOSE_DEV (1 << 2)

// MIDI message are 4byte and can be packed up to a 64bytes USB packet
#define MIDI_PACKET_SIZE 64
#define MIDI_EP_ADDR 0x81
#define MIDI_INTERFACE_NB 1

#define CLIENT_TASK_PRIO 1
#define CLIENT_TASK_STACK (5 * 1024)

// -----------------------------------------------------------------------------
// Private Typedefs
// -----------------------------------------------------------------------------
typedef struct
{
    uint32_t actions;
    uint8_t dev_addr;
    usb_host_client_handle_t client_hdl;
    usb_device_handle_t dev_hdl;
} class_driver_control_t;

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------
static TaskHandle_t client_task_handle = NULL;

volatile bool transfer_active = false;
static midi_on_receive_cb_t on_receive_cb = NULL;
static char dev_name[32] = "NO NAME";

const char *notes[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

// -----------------------------------------------------------------------------
// Static Function Declarations
// -----------------------------------------------------------------------------
static void usb_client_task(void *pvParams);
static void midi_in_cb(usb_transfer_t *transfer);
static void client_event_cb(const usb_host_client_event_msg_t *event_msg, void *arg);

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------
esp_err_t usb_midi_init(void)
{
    BaseType_t task_created = xTaskCreatePinnedToCore(
        usb_client_task, "usb_midi", CLIENT_TASK_STACK, NULL, CLIENT_TASK_PRIO, &client_task_handle, 0);
    ESP_RETURN_ON_FALSE(task_created == pdPASS, ESP_ERR_NO_MEM, TAG, "Failed to create USB MIDI client task");

    return ESP_OK;
}

esp_err_t usb_midi_set_on_receive_cb(midi_on_receive_cb_t cb)
{
    on_receive_cb = cb;
    return cb ? ESP_OK : ESP_ERR_INVALID_ARG;
}

midi_msg_parsed_t usb_midi_parse_msg(midi_message_t *msg)
{
    midi_msg_parsed_t parsed_msg = {0};

    if (msg == NULL) return parsed_msg;

    parsed_msg.velocity = msg->velocity / 127.f;
    parsed_msg.octave = -2 + msg->note / 12;
    parsed_msg.freq_hz = 440.0f * powf(2.0f, (msg->note - 69) / 12.0f);
    strncpy(parsed_msg.note, notes[msg->note % 12], sizeof(parsed_msg.note));

    return parsed_msg;
}

// -----------------------------------------------------------------------------
// Static Function Definitions
// -----------------------------------------------------------------------------
static void midi_in_cb(usb_transfer_t *transfer)
{
    for (int i = 0; i < transfer->actual_num_bytes; i += 4)
    {
        uint8_t cin = transfer->data_buffer[i] & 0x0F;
        uint8_t note = transfer->data_buffer[i + 2];
        uint8_t vel = transfer->data_buffer[i + 3];

        midi_message_t msg = {.note = note};

        switch (cin)
        {
        case 0x9: // Note On
            msg.state = 1;
            msg.velocity = vel;
            break;
        default:
            break;
        }

        if (on_receive_cb) on_receive_cb(msg);
    }

    if (transfer->actual_num_bytes > 0 && transfer_active && transfer->device_handle != NULL)
    {
        esp_err_t ret = usb_host_transfer_submit(transfer);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to resubmit: %d", ret);
            transfer_active = false;
        }
    }
}

static void client_event_cb(const usb_host_client_event_msg_t *event_msg, void *arg)
{
    class_driver_control_t *class_driver_obj = (class_driver_control_t *)arg;
    switch (event_msg->event)
    {
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
        class_driver_obj->dev_addr = event_msg->new_dev.address;
        class_driver_obj->dev_hdl = NULL;
        class_driver_obj->actions |= ACTION_OPEN_DEV;
        break;
    case USB_HOST_CLIENT_EVENT_DEV_GONE:
        class_driver_obj->actions |= ACTION_CLOSE_DEV;
        break;
    }
}

static void usb_client_task(void *pvParams)
{
    // Initialize class driver objects
    static class_driver_control_t class_driver_obj = {0};
    // Register the client
    usb_host_client_config_t client_config = {.is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = client_event_cb,
            .callback_arg = &class_driver_obj,
        }};
    esp_err_t ret = usb_host_client_register(&client_config, &class_driver_obj.client_hdl);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register client");
        vTaskDelete(NULL);
    }

    // Allocate a USB transfer
    usb_transfer_t *transfer;
    usb_host_transfer_alloc(MIDI_PACKET_SIZE, 0, &transfer);
    memset(transfer->data_buffer, 0xAA, MIDI_PACKET_SIZE);
    transfer->num_bytes = MIDI_PACKET_SIZE;
    transfer->bEndpointAddress = MIDI_EP_ADDR;
    transfer->callback = midi_in_cb;
    transfer->context = (void *)&class_driver_obj;

    while (1)
    {
        usb_host_client_handle_events(class_driver_obj.client_hdl, portMAX_DELAY);
        if (class_driver_obj.actions & ACTION_OPEN_DEV)
        {
            ESP_LOGI(TAG, "USB device connected");
            // Open the device and claim interface 1
            esp_err_t err =
                usb_host_device_open(class_driver_obj.client_hdl, class_driver_obj.dev_addr, &class_driver_obj.dev_hdl);
            err = usb_host_interface_claim(class_driver_obj.client_hdl, class_driver_obj.dev_hdl, MIDI_INTERFACE_NB, 0);
            if (err == ESP_OK)
            {
                usb_device_info_t dev_info;
                if (usb_host_device_info(class_driver_obj.dev_hdl, &dev_info) == ESP_OK)
                {
                    const usb_str_desc_t *str_desc = dev_info.str_desc_product;
                    int n = 0;
                    for (int i = 0; i < str_desc->bLength / 2; i++)
                    {
                        /*
                        USB String descriptors of UTF-16.
                        Right now We just skip any character larger than 0xFF to
                        stay in BMP Basic Latin and Latin-1 Supplement range.
                        */
                        if (str_desc->wData[i] > 0xFF) continue;
                        dev_name[n++] = (char)str_desc->wData[i];
                        if (n >= sizeof(dev_name) - 1) break;
                    }
                    dev_name[n] = '\0';
                }
                ESP_LOGI(TAG, "Found MIDI interface on device: %s", dev_name);

                event_t event = {
                    .source = EVENT_SRC_USB_MIDI, .type = USB_MIDI_EVENT_CONNECTED, .data = (void *)dev_name};
                event_bus_publish(&event);

                transfer->device_handle = class_driver_obj.dev_hdl;
                transfer_active = true;
                usb_host_transfer_submit(transfer);
            }
            else
            {
                ESP_LOGI(TAG, "No MIDI endpoint found. Removing device");
                class_driver_obj.actions |= ACTION_CLOSE_DEV;
            }
        }
        if (class_driver_obj.actions & ACTION_CLOSE_DEV)
        {
            transfer_active = false;
            transfer->device_handle = NULL;
            vTaskDelay(pdMS_TO_TICKS(100));
            usb_host_interface_release(class_driver_obj.client_hdl, class_driver_obj.dev_hdl, MIDI_INTERFACE_NB);
            usb_host_device_close(class_driver_obj.client_hdl, class_driver_obj.dev_hdl);

            ESP_LOGI(TAG, "USB device disconnected");

            event_t event = {.source = EVENT_SRC_USB_MIDI, .type = USB_MIDI_EVENT_DISCONNECTED};
            event_bus_publish(&event);
        }

        class_driver_obj.actions = 0;
    }

    // Cleanup class driver
    usb_host_transfer_free(transfer);
    usb_host_client_deregister(class_driver_obj.client_hdl);
}
