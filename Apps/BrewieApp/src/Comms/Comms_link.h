#ifndef FREEBREWIE_COMMS_LINK_H
#define FREEBREWIE_COMMS_LINK_H

/****************************************************************************************
 * @file Comms_link.h
 * @brief Compact link-health interpretation for the MCU connection.
 *
 * Responsibility: convert recent communication facts into a simple link state.
 * Owns: down/waiting/ok interpretation and timeout policy.
 * Does not own: frame parsing or UI label text.
 ****************************************************************************************/

#include <stdint.h>

#include "Comms_types.h"

#define COMMS_LINK_TIMEOUT_MS 3000U

void comms_link_update(comms_status_t *status, uint64_t now_ms);
const char *comms_link_state_name(comms_link_state_t state);

#endif
