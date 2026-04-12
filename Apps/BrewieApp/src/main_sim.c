#include <stdio.h>
#include <unistd.h>

#include "lvgl.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "UI/UI.h"

static void log_cb(lv_log_level_t level, const char *buf)
{
    (void)level;
    fputs(buf, stderr);
    fputc('\n', stderr);
}

int main()
{
    ui_state_t ui_state;

    lv_init();
#if LV_USE_LOG
    lv_log_register_print_cb(log_cb);
#endif

    lv_display_t *display = lv_sdl_window_create(480, 272);
    if(display == NULL) {
        fprintf(stderr, "Failed to create SDL window\n");
        return 1;
    }

    lv_sdl_mouse_create();
    lv_sdl_mousewheel_create();
    lv_sdl_keyboard_create();

    ui_init(&ui_state);
    ui_set_status(&ui_state, "simulator", "running");
    ui_set_status(&ui_state, "serial", "not used in sim");
    ui_set_status(&ui_state, "heartbeat", "simulator only");
    ui_set_status(&ui_state, "last rx", "none");
    ui_set_status(&ui_state, "link", "n/a");

    for(;;) {
        lv_timer_handler();
        lv_tick_inc(5);
        usleep(5000);
    }

    return 0;
}
