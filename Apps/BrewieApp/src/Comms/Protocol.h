#ifndef FREEBREWIE_PROTOCOL_H
#define FREEBREWIE_PROTOCOL_H

/****************************************************************************************
 * @file Protocol.h
 * @brief SOM-MCU wire frame encoder and byte-stream decoder.
 *
 * Responsibility: Encode and decode SOM-MCU protocol frames.
 * Owns: Sync bytes, message IDs, CRC, outgoing frame building, and receive state machine.
 * Does not own: Serial devices or appliance/UI decisions.
 ****************************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PROTOCOL_SYNC1            0xAAU
#define PROTOCOL_SYNC2            0x55U
#define PROTOCOL_MAX_DATA_SIZE    64U
#define PROTOCOL_MAX_FRAME_SIZE   (2U + 1U + 1U + 1U + PROTOCOL_MAX_DATA_SIZE + 1U)

/**
 * One wire frame is:
 *
 *   [SYNC1][SYNC2][TYPE][SEQ][LEN][DATA]....[CRC]
 *
 * Sync1/sync2 let the receiver find the start of a frame again after noise. 
 * Type tells what data the frame contains. 
 * Seq is a rolling message number. 
 * Len is the payload byte count. 
 * Crc protects type, seq, len, and data, but not the sync bytes.
 */
enum
{
    PROTOCOL_MSG_CONTROL_SNAPSHOT = 0x01,
    PROTOCOL_MSG_HEARTBEAT = 0x02,
    PROTOCOL_MSG_STATUS_REPORT = 0x03,
    PROTOCOL_MSG_FAULT_REPORT = 0x04,
    PROTOCOL_MSG_ACK = 0x05,
    PROTOCOL_MSG_NACK = 0x06,
    PROTOCOL_MSG_SHUTDOWN_REQUEST = 0x07,
    PROTOCOL_MSG_FAULT_CLEAR_REQUEST = 0x08
};

/** Fully decoded frame after the byte-by-byte receiver has validated the CRC. */
typedef struct
{
    uint8_t type;
    uint8_t seq;
    uint8_t len;
    uint8_t data[PROTOCOL_MAX_DATA_SIZE];
} protocol_frame_t;

/** Outgoing sequence number state for frames created by the SOM. */
typedef struct
{
    uint8_t next_seq;
} protocol_sender_t;

/**
 * Byte receiver state machine stages.
 *
 * UART data arrives as an endless stream of bytes, not as neat packets. The receiver walks
 * through these stages until it has a complete, CRC-checked frame.
 */
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

/** Runtime memory for the byte-by-byte protocol receiver. */
typedef struct
{
    protocol_rx_stage_t stage;
    protocol_frame_t frame;
    uint8_t data_index;
    uint8_t crc;
} protocol_rx_state_t;

void protocol_sender_init(protocol_sender_t *sender, uint8_t first_seq);
size_t protocol_build_frame(protocol_sender_t *sender,
                            uint8_t type,
                            const uint8_t *payload,
                            uint8_t payload_size,
                            uint8_t *buffer,
                            size_t buffer_size);
size_t protocol_build_heartbeat(protocol_sender_t *sender, uint8_t *buffer, size_t buffer_size);
void protocol_rx_init(protocol_rx_state_t *rx);
bool protocol_rx_consume(protocol_rx_state_t *rx, uint8_t byte, protocol_frame_t *out_frame);
uint8_t protocol_crc8_dallas_maxim(const uint8_t *data, size_t length);

#endif
