#include "app.h"
#include "bsp.h"
#include "bsp/pins.h"
#include "service/gui.h"
#include "service/usb.h"
#include "system/error.h"
#include "system/event_bus.h"

#include "driver/gpio.h"
#include "esp_log.h"

#define TAG                "main"

#define BOOT_MODE_NORMAL   0
#define BOOT_MODE_DOWNLOAD 1

void
app_main(void)
{
    gui_ctx_t gui_ctx = {0};
    usb_ctx_t usb_ctx = {0};
    app_ctx_t app_ctx = {0};
    int boot_mode = BOOT_MODE_NORMAL;

    /* BOOT */
    gpio_set_direction(ROTARY_ENCODER_SW_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(ROTARY_ENCODER_SW_PIN);

    if (gpio_get_level(ROTARY_ENCODER_SW_PIN) == 0)
    {
        boot_mode = BOOT_MODE_DOWNLOAD;
        ESP_LOGW(TAG, "booted in download mode");
    }

    RETURN_VOID_ON_ERROR(event_bus_init());

    /* BSP */
    RETURN_VOID_ON_ERROR(bsp_shared_init());
    RETURN_VOID_ON_ERROR(bsp_synth_init());
    RETURN_VOID_ON_ERROR(bsp_input_init());
    RETURN_VOID_ON_ERROR(bsp_display_init());

    /* SERVICES */
    gui_ctx.display = &bsp.devs.display;
    gui_ctx.re = &bsp.devs.re;
    gui_ctx.trigger = &bsp.devs.trigger;
    RETURN_VOID_ON_ERROR(gui_init(&gui_ctx));

    usb_ctx.mac = bsp.info.mac;
    usb_ctx.mode = boot_mode == BOOT_MODE_DOWNLOAD ? USB_MODE_SERIAL_JTAG : USB_MODE_OTG_DEVICE;
    RETURN_VOID_ON_ERROR(usb_init(&usb_ctx));

    /* APP */
    app_ctx.synth = bsp.devs.synth;
    RETURN_VOID_ON_ERROR(app_init(&app_ctx));
}
