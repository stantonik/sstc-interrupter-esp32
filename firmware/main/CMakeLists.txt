set(SRCS
    main.c

    # BSP
    bsp.c

    # Drivers
    driver/rotary_encoder.c
    driver/synth.c
    driver/display_ssd1306.c
    driver/lvgl_timer.c
    driver/button.c

    # Services
    service/usb.c
    service/gui.c
    service/gui/generated/images.c
    service/gui/generated/screens.c
    service/gui/generated/styles.c
    service/gui/generated/ui.c
    service/gui/generated/ui_font_unscii_8.c

    # System
    system/event_bus.c

    # App
    app.c
    app/midi_synth.c
)

option(RELEASE_BUILD "Build release configuration" OFF)
if(RELEASE_BUILD)
    set(CONFIG_INCLUDE_DIR "bsp/release")
else()
    set(CONFIG_INCLUDE_DIR "bsp/dev")
endif()

idf_component_register(SRCS ${SRCS}
                    INCLUDE_DIRS "." "${CONFIG_INCLUDE_DIR}")
