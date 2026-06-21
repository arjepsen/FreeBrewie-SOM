#include "Platform.h"

#include <string.h>

bool platform_init(platform_t *platform)
{
    if (platform == NULL)
    {
        return false;
    }

    memset(platform, 0, sizeof(*platform));
    return true;
}

void platform_shutdown(platform_t *platform)
{
    (void)platform;
}
