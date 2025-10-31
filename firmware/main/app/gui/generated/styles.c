#include "styles.h"
#include "images.h"

#include "ui.h"
#include "screens.h"

//
// Style: range_value_style
//

void init_style_range_value_style_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_font(style, &lv_font_montserrat_10);
    lv_style_set_align(style, LV_ALIGN_CENTER);
};

lv_style_t *get_style_range_value_style_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_range_value_style_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_range_value_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_range_value_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_range_value_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_range_value_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: range_name_style
//

void init_style_range_name_style_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_font(style, &lv_font_montserrat_10);
    lv_style_set_align(style, LV_ALIGN_TOP_MID);
};

lv_style_t *get_style_range_name_style_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_range_name_style_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_range_name_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_range_name_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_range_name_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_range_name_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: range_arc_style
//

void init_style_range_arc_style_INDICATOR_DEFAULT(lv_style_t *style) {
    lv_style_set_arc_width(style, 2);
    lv_style_set_arc_color(style, lv_color_hex(0xffffffff));
};

lv_style_t *get_style_range_arc_style_INDICATOR_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_range_arc_style_INDICATOR_DEFAULT(style);
    }
    return style;
};

void init_style_range_arc_style_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_arc_width(style, 4);
    lv_style_set_arc_color(style, lv_color_hex(0xff000000));
    lv_style_set_arc_opa(style, 0);
};

lv_style_t *get_style_range_arc_style_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_range_arc_style_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_range_arc_style_KNOB_DEFAULT(lv_style_t *style) {
    lv_style_set_pad_top(style, 1);
    lv_style_set_pad_bottom(style, 1);
    lv_style_set_pad_left(style, 1);
    lv_style_set_pad_right(style, 1);
    lv_style_set_bg_opa(style, 0);
    lv_style_set_bg_color(style, lv_color_hex(0xffffffff));
};

lv_style_t *get_style_range_arc_style_KNOB_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_range_arc_style_KNOB_DEFAULT(style);
    }
    return style;
};

void init_style_range_arc_style_KNOB_EDITED(lv_style_t *style) {
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_range_arc_style_KNOB_EDITED() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_range_arc_style_KNOB_EDITED(style);
    }
    return style;
};

void add_style_range_arc_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_range_arc_style_INDICATOR_DEFAULT(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_range_arc_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_range_arc_style_KNOB_DEFAULT(), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_range_arc_style_KNOB_EDITED(), LV_PART_KNOB | LV_STATE_EDITED);
};

void remove_style_range_arc_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_range_arc_style_INDICATOR_DEFAULT(), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_range_arc_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_range_arc_style_KNOB_DEFAULT(), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_range_arc_style_KNOB_EDITED(), LV_PART_KNOB | LV_STATE_EDITED);
};

//
// Style: header_label_style
//

void init_style_header_label_style_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &lv_font_montserrat_10);
};

lv_style_t *get_style_header_label_style_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_header_label_style_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_header_label_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_header_label_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_header_label_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_header_label_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: range_selector_style
//

void init_style_range_selector_style_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_align(style, LV_ALIGN_BOTTOM_MID);
    lv_style_set_img_opa(style, 0);
};

lv_style_t *get_style_range_selector_style_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_range_selector_style_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_range_selector_style_MAIN_FOCUSED(lv_style_t *style) {
    lv_style_set_img_opa(style, 255);
};

lv_style_t *get_style_range_selector_style_MAIN_FOCUSED() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_range_selector_style_MAIN_FOCUSED(style);
    }
    return style;
};

void add_style_range_selector_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_range_selector_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_range_selector_style_MAIN_FOCUSED(), LV_PART_MAIN | LV_STATE_FOCUSED);
};

void remove_style_range_selector_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_range_selector_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_range_selector_style_MAIN_FOCUSED(), LV_PART_MAIN | LV_STATE_FOCUSED);
};

//
// Style: header_item_style
//

void init_style_header_item_style_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_layout(style, LV_LAYOUT_FLEX);
    lv_style_set_flex_cross_place(style, LV_FLEX_ALIGN_CENTER);
    lv_style_set_pad_column(style, 4);
};

lv_style_t *get_style_header_item_style_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_header_item_style_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_header_item_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_header_item_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_header_item_style(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_header_item_style_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_range_value_style,
        add_style_range_name_style,
        add_style_range_arc_style,
        add_style_header_label_style,
        add_style_range_selector_style,
        add_style_header_item_style,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_range_value_style,
        remove_style_range_name_style,
        remove_style_range_arc_style,
        remove_style_header_label_style,
        remove_style_range_selector_style,
        remove_style_header_item_style,
    };
    remove_style_funcs[styleIndex](obj);
}

