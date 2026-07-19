#ifndef FREEBREWIE_UI_SCROLL_H
#define FREEBREWIE_UI_SCROLL_H

/****************************************************************************************
 * @file UI_scroll.h
 * @brief Shared scrollbar/gutter styling for scrollable LVGL containers.
 *
 * Responsibility: keep scrollable screen bodies from drawing scrollbars over row content.
 * Owns: small shared scrollbar geometry/style helper.
 * Must not own: screen layout, row creation, navigation, or scrollable content semantics.
 ****************************************************************************************/

#include "lvgl.h"

void ui_scroll_apply_gutter(lv_obj_t *object);

#endif
