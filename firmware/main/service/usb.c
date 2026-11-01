/**
 * \file usb.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-10-30
 */

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include "usb.h"
#include "system/error.h"

#include "driver/usb_serial_jtag.h"
#include "system/event_bus.h"
#include "system/logging.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */
#define USB_DESCRIPTOR_MANUFACTURER "ARNAUD"
#define USB_DESCRIPTOR_PRODUCT      "SSTC V1"

#define TAG                         "usb"

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
// Interface counter
enum interface_count
{
#if CFG_TUD_MIDI
    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
#endif
    ITF_COUNT
};

// USB Endpoint numbers
enum usb_endpoints
{
    // Available USB Endpoints: 5 IN/OUT EPs and 1 IN EP
    EP_EMPTY = 0,
#if CFG_TUD_MIDI
    EPNUM_MIDI,
#endif
};

/** TinyUSB descriptors **/

#define TUSB_DESCRIPTOR_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_MIDI * TUD_MIDI_DESC_LEN)

/* --------------------------------------------------------------------------
 * Static Variables
 * -------------------------------------------------------------------------- */
static char s_serial[13] = "123456";

static const char *s_str_desc[5] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},        // 0: is supported language is English (0x0409)
    USB_DESCRIPTOR_MANUFACTURER, // 1: Manufacturer
    USB_DESCRIPTOR_PRODUCT,      // 2: Product
    s_serial,                    // 3: Serials, should use chip ID
    "Zeusaphone MIDI In",        // 4: MIDI
};

static const uint8_t s_midi_cfg_desc[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, TUSB_DESCRIPTOR_TOTAL_LEN, 0, 100),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI, (0x80 | EPNUM_MIDI), 64),
};

static void (*midi_event_cb)(uint8_t *packet, size_t size) = NULL;

// static struct
// {
// } g_state = {0};

/* --------------------------------------------------------------------------
 * Static Function Declarations
 * -------------------------------------------------------------------------- */
static void event_cb(tinyusb_event_t *event, void *arg);

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
int
usb_init(usb_ctx_t *ctx)
{
    if (ctx->mode == USB_MODE_SERIAL_JTAG)
    {
        usb_serial_jtag_driver_config_t sj_conf = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
        if (usb_serial_jtag_driver_install(&sj_conf) != ESP_OK)
        {
            return RET_ERR_FAIL;
        }
    }
    else if (ctx->mode == USB_MODE_OTG_DEVICE)
    {
        tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();

        snprintf(
            s_serial, sizeof(s_serial), "%02X%02X%02X%02X%02X%02X", ctx->mac[0], ctx->mac[1],
            ctx->mac[2], ctx->mac[3], ctx->mac[4], ctx->mac[5]
        );

        tusb_cfg.descriptor.string = s_str_desc;
        tusb_cfg.descriptor.string_count = sizeof(s_str_desc) / sizeof(s_str_desc[0]);
        tusb_cfg.descriptor.full_speed_config = s_midi_cfg_desc;
        tusb_cfg.event_cb = event_cb;

        if (tinyusb_driver_install(&tusb_cfg) != ESP_OK)
        {
            return RET_ERR_FAIL;
        }
    }

    LOG_INFO(TAG, "initialized");

    return RET_OK;
}

void usb_set_midi_event_cb(void (*cb)(uint8_t *packet, size_t size))
{
    midi_event_cb = cb;
}

void
tud_midi_rx_cb(uint8_t itf)
{
    uint8_t packet[4];
    // event_t app_event = {0};

    while (tud_midi_packet_read(packet))
    {
        LOG_DEBUG(
            TAG, "midi packet: %02X %02X %02X %02X", packet[0], packet[1], packet[2], packet[3]
        );

        // app_event.code = EVENT_USB_MIDI_MESSAGE;
        // app_event.payload.val = *(uint64_t *)(&packet);
        // event_bus_publish(&app_event);

        if (midi_event_cb != NULL)
        {
            midi_event_cb(packet, 4);
        }
    }
}

/* --------------------------------------------------------------------------
 * Static Function Definitions
 * -------------------------------------------------------------------------- */
static void
event_cb(tinyusb_event_t *event, void *arg)
{
    event_t app_event = {0};

    switch (event->id)
    {
        case TINYUSB_EVENT_ATTACHED:
            LOG_DEBUG(TAG, "host attached");
            app_event.code = EVENT_USB_HOST_ATTACHED;
            break;

        case TINYUSB_EVENT_DETACHED:
            LOG_DEBUG(TAG, "host detached");
            app_event.code = EVENT_USB_HOST_DETACHED;
            break;

        case TINYUSB_EVENT_SUSPENDED:
            LOG_DEBUG(TAG, "USB suspended");
            app_event.code = EVENT_USB_SUSPENDED;
            break;

        case TINYUSB_EVENT_RESUMED:
            LOG_DEBUG(TAG, "USB resumed");
            app_event.code = EVENT_USB_RESUMED;
            break;
    }

    event_bus_publish(&app_event);
}
