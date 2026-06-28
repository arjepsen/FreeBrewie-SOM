#ifndef FREEBREWIE_DISPLAY_H
#define FREEBREWIE_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

#include "lvgl.h"

typedef struct
{
    bool ready;
    bool simulator;
    uint64_t last_tick_ms;
} display_t;

bool display_init(display_t *display);
void display_update(display_t *display, uint64_t now_ms);

#endif
