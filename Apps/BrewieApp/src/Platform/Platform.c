#include "Platform.h"

#include <string.h>

/****************************************************************************************
 * @brief Initialize shared platform resources.
 *
 * The platform layer is intentionally small right now. It gives the app one place to grow
 * Linux-owned resources such as display, input, storage, or future system services.
 ****************************************************************************************/
bool platform_init(platform_t *platform)
{
    if (platform == NULL)
    {
        return false;
    }

    memset(platform, 0, sizeof(*platform));
    return true;
}

/****************************************************************************************
 * @brief Shut down platform resources.
 *
 * There is no explicit display teardown yet because the service normally exits only during
 * reboot or replacement. This function exists so cleanup can be added without changing the
 * app layer.
 ****************************************************************************************/
void platform_shutdown(platform_t *platform)
{
    (void)platform;
}
