#ifndef FREEBREWIE_COMMS_TYPES_H
#define FREEBREWIE_COMMS_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    COMMS_LINK_DOWN = 0,
    COMMS_LINK_WAITING,
    COMMS_LINK_OK
} comms_link_state_t;

typedef struct
{
    bool serial_ready;
    bool heartbeat_running;
    uint32_t heartbeat_count;
    uint64_t last_rx_ms;
    comms_link_state_t link_state;
    uint8_t last_rx_type;
    uint8_t last_rx_seq;
    uint8_t last_rx_len;
} comms_status_t;

#endif
