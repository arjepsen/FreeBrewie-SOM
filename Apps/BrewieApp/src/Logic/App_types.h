#ifndef FREEBREWIE_APP_TYPES_H
#define FREEBREWIE_APP_TYPES_H

#include <stdint.h>

typedef struct
{
    const char *display_text;
    const char *serial_text;
    const char *heartbeat_text;
    const char *last_rx_text;
    const char *link_text;
    uint32_t heartbeat_count;
} boot_screen_view_model_t;

void boot_screen_view_model_init(boot_screen_view_model_t *view_model);

#endif
