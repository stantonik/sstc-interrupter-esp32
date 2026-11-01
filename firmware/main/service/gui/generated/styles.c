#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: home_knob_arc
//

void init_style_home_knob_arc_INDICATOR_DEFAULT(lv_style_t *style) {
    lv_style_set_arc_color(style, lv_color_hex(0xffffff));
    lv_style_set_arc_rounded(style, false);
    lv_style_set_arc_width(style, 2);
};

lv_style_t *get_style_home_knob_arc_INDICATOR_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_home_knob_arc_INDICATOR_DEFAULT(style);
    }
    return style;
};

void init_style_home_knob_arc_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_arc_opa(style, 0);
};

lv_style_t *get_style_home_knob_arc_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_home_knob_arc_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_home_knob_arc_KNOB_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xffffff));
    lv_style_set_radius(style, 2);
    lv_style_set_pad_top(style, 1);
    lv_style_set_pad_bottom(style, 1);
    lv_style_set_pad_left(style, 1);
    lv_style_set_pad_right(style, 1);
    lv_style_set_bg_opa(style, 0);
};

lv_style_t *get_style_home_knob_arc_KNOB_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_home_knob_arc_KNOB_DEFAULT(style);
    }
    return style;
};

void init_style_home_knob_arc_KNOB_FOCUSED(lv_style_t *style) {
    lv_style_set_border_width(style, 1);
    lv_style_set_border_color(style, lv_color_hex(0xffffff));
};

lv_style_t *get_style_home_knob_arc_KNOB_FOCUSED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_home_knob_arc_KNOB_FOCUSED(style);
    }
    return style;
};

void init_style_home_knob_arc_KNOB_EDITED(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xffffff));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_home_knob_arc_KNOB_EDITED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_home_knob_arc_KNOB_EDITED(style);
    }
    return style;
};

void add_style_home_knob_arc(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_home_knob_arc_INDICATOR_DEFAULT(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_home_knob_arc_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_home_knob_arc_KNOB_DEFAULT(), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_home_knob_arc_KNOB_FOCUSED(), LV_PART_KNOB | LV_STATE_FOCUSED);
    lv_obj_add_style(obj, get_style_home_knob_arc_KNOB_EDITED(), LV_PART_KNOB | LV_STATE_EDITED);
};

void remove_style_home_knob_arc(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_home_knob_arc_INDICATOR_DEFAULT(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_home_knob_arc_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_home_knob_arc_KNOB_DEFAULT(), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_home_knob_arc_KNOB_FOCUSED(), LV_PART_KNOB | LV_STATE_FOCUSED);
    lv_obj_remove_style(obj, get_style_home_knob_arc_KNOB_EDITED(), LV_PART_KNOB | LV_STATE_EDITED);
};

//
// Style: mode_label
//

void init_style_mode_label_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &ui_font_unscii_8);
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
};

lv_style_t *get_style_mode_label_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_mode_label_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_mode_label_MAIN_CHECKED(lv_style_t *style) {
    lv_style_set_bg_opa(style, 255);
    lv_style_set_text_color(style, lv_color_hex(0x000000));
    lv_style_set_pad_left(style, 2);
    lv_style_set_pad_right(style, 2);
};

lv_style_t *get_style_mode_label_MAIN_CHECKED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_mode_label_MAIN_CHECKED(style);
    }
    return style;
};

void add_style_mode_label(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_mode_label_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_mode_label_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

void remove_style_mode_label(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_mode_label_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_mode_label_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_home_knob_arc,
        add_style_mode_label,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_home_knob_arc,
        remove_style_mode_label,
    };
    remove_style_funcs[styleIndex](obj);
}