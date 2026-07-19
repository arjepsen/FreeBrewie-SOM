#include "UI_scroll.h"

#define UI_SCROLL_GUTTER_PX 6
#define UI_SCROLLBAR_WIDTH_PX 4

/****************************************************************************************
 * @brief Give a scrollable object a clear right-side lane for its scrollbar.
 *
 * LVGL scrollbars are drawn over the scrollable object. On a 272 px wide portrait screen,
 * that can make the scrollbar visually overlap full-width list rows. This helper reserves
 * a small right gutter in the content area and gives the scrollbar a slim, consistent look.
 ****************************************************************************************/
void ui_scroll_apply_gutter(lv_obj_t *object)
{
    if (object == NULL)
    {
        return;
    }

    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_right(object, UI_SCROLL_GUTTER_PX, 0);
    lv_obj_set_style_width(object, UI_SCROLLBAR_WIDTH_PX, LV_PART_SCROLLBAR);
    lv_obj_set_style_radius(object, UI_SCROLLBAR_WIDTH_PX, LV_PART_SCROLLBAR);
    lv_obj_set_style_pad_right(object, 0, LV_PART_SCROLLBAR);
    lv_obj_set_style_pad_left(object, 0, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(object, lv_color_hex(0xE67526), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(object, LV_OPA_80, LV_PART_SCROLLBAR);
    lv_obj_set_style_border_width(object, 0, LV_PART_SCROLLBAR);
}
