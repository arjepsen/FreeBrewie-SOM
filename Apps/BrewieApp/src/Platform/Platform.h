#ifndef FREEBREWIE_PLATFORM_H
#define FREEBREWIE_PLATFORM_H

#include <stdbool.h>

#include "Display.h"

typedef struct
{
    display_t display;
} platform_t;

bool platform_init(platform_t *platform);
void platform_shutdown(platform_t *platform);

#endif
