#ifndef FREEBREWIE_PROTOCOL_H
#define FREEBREWIE_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PROTOCOL_SYNC1            0xAAU
#define PROTOCOL_SYNC2            0x55U
#define PROTOCOL_MAX_DATA_SIZE    64U
#define PROTOCOL_MAX_FRAME_SIZE   (2U + 1U + 1U + 1U + PROTOCOL_MAX_DATA_SIZE + 1U)

enum
{
    PROTOCOL_MSG_CONTROL_SNAPSHOT   = 0x01,
    PROTOCOL_MSG_HEARTBEAT          = 0x02,
    PROTOCOL_MSG_STATUS_REPORT      = 0x03,
    PROTOCOL_MSG_FAULT_REPORT       = 0x04,
    PROTOCOL_MSG_ACK                = 0x05,
    PROTOCOL_MSG_NACK               = 0x06,
    PROTOCOL_MSG_SHUTDOWN_REQUEST   = 0x07,
    PROTOCOL_MSG_FAULT_CLEAR_REQUEST= 0x08
};

typedef struct
{
    uint8_t type;
    uint8_t seq;
    uint8_t len;
    uint8_t data[PROTOCOL_MAX_DATA_SIZE];
} protocol_frame_t;

typedef struct
{
    uint8_t next_seq;
} protocol_sender_t;

typedef enum
{
    PROTOCOL_RX_WAIT_SYNC1 = 0,
    PROTOCOL_RX_WAIT_SYNC2,
    PROTOCOL_RX_WAIT_TYPE,
    PROTOCOL_RX_WAIT_SEQ,
    PROTOCOL_RX_WAIT_LEN,
    PROTOCOL_RX_WAIT_DATA,
    PROTOCOL_RX_WAIT_CRC
} protocol_rx_stage_t;

typedef struct
{
    protocol_rx_stage_t stage;
    protocol_frame_t frame;
    uint8_t data_index;
    uint8_t crc;
} protocol_rx_state_t;

void protocol_sender_init(protocol_sender_t *sender, uint8_t first_seq);
size_t protocol_build_heartbeat(protocol_sender_t *sender, uint8_t *buffer, size_t buffer_size);

void protocol_rx_init(protocol_rx_state_t *rx);
bool protocol_rx_consume(protocol_rx_state_t *rx, uint8_t byte, protocol_frame_t *out_frame);
uint8_t protocol_crc8_dallas_maxim(const uint8_t *data, size_t length);

#endif
