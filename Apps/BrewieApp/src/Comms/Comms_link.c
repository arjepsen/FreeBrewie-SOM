#include "Comms_link.h"
#include <stddef.h>

/****************************************************************************************
 * @brief Refresh the high-level SOM-to-MCU link state.
 *
 * The serial port can be open even when the MCU has not yet answered. This helper turns
 * those lower-level facts into one simple state for the UI and app logic:
 *
 * - DOWN: the serial device could not be opened.
 * - WAITING: the serial device is open, but no recent MCU frame has arrived.
 * - OK: a frame has arrived recently enough that the link is considered alive.
 *
 * The function only writes link_state when the state actually changes.
 ****************************************************************************************/
void comms_link_update(comms_status_t *status, uint64_t now_ms)
{
    comms_link_state_t next_state;

    if (status == NULL)
    {
        return;
    }

    next_state = COMMS_LINK_OK;
    if (!status->serial_ready)
    {
        next_state = COMMS_LINK_DOWN;
    }
    else if (status->last_rx_ms == 0U)
    {
        next_state = COMMS_LINK_WAITING;
    }
    else if ((now_ms - status->last_rx_ms) > COMMS_LINK_TIMEOUT_MS)
    {
        next_state = COMMS_LINK_WAITING;
    }

    if (status->link_state != next_state)
    {
        status->link_state = next_state;
    }
}

/****************************************************************************************
 * @brief Convert a link state enum into short UI/debug text.
 ****************************************************************************************/
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
