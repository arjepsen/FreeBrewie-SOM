#include "Comms_link.h"
#include <stddef.h>

void comms_link_update(comms_status_t *status, uint64_t now_ms)
{
    if (status == NULL)
    {
        return;
    }

    if (!status->serial_ready)
    {
        status->link_state = COMMS_LINK_DOWN;
        return;
    }

    if (status->last_rx_ms == 0U)
    {
        status->link_state = COMMS_LINK_WAITING;
        return;
    }

    if ((now_ms - status->last_rx_ms) > COMMS_LINK_TIMEOUT_MS)
    {
        status->link_state = COMMS_LINK_WAITING;
        return;
    }

    status->link_state = COMMS_LINK_OK;
}

const char *comms_link_state_name(comms_link_state_t state)
{
    switch (state)
    {
    case COMMS_LINK_DOWN:
        return "down";
    case COMMS_LINK_WAITING:
        return "waiting";
    case COMMS_LINK_OK:
        return "ok";
    default:
        return "unknown";
    }
}
