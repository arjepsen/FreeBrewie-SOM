#ifndef FREEBREWIE_COMMS_H
#define FREEBREWIE_COMMS_H

/****************************************************************************************
 * @file Comms.h
 * @brief High-level SOM to MCU communication runtime.
 *
 * Responsibility: run the live SOM-MCU communication path.
 * Owns: serial link state, heartbeat sending, protocol receive state, and latest decoded
 * MCU facts.
 * Does not own: raw UART configuration or wire-frame format details.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "Comms_link.h"
#include "Protocol.h"
#include "Transport_serial.h"

typedef struct
{
    transport_serial_t serial;
    protocol_rx_state_t protocol_rx;
    protocol_sender_t protocol_sender;
    comms_status_t status;
    uint64_t last_heartbeat_ms;
} comms_t;

bool comms_init(comms_t *comms, const char *device_path, int baud_rate);
void comms_update(comms_t *comms, uint64_t now_ms);
void comms_shutdown(comms_t *comms);
const comms_status_t *comms_get_status(const comms_t *comms);
bool comms_is_serial_ready(const comms_t *comms);
bool comms_send_control_snapshot(comms_t *comms, const uint8_t *payload, uint8_t payload_size);

#endif
