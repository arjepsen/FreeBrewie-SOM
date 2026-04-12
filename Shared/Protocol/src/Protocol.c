#include "Protocol.h"

#include <string.h>

static uint8_t protocol_next_seq(protocol_sender_t *sender)
{
    uint8_t seq;

    seq = sender->next_seq;
    if(seq == 0U) {
        seq = 1U;
    }

    sender->next_seq = (uint8_t)(seq + 1U);
    if(sender->next_seq == 0U) {
        sender->next_seq = 1U;
    }

    return seq;
}

void protocol_sender_init(protocol_sender_t *sender, uint8_t first_seq)
{
    if(sender == NULL) {
        return;
    }

    sender->next_seq = first_seq;
    if(sender->next_seq == 0U) {
        sender->next_seq = 1U;
    }
}

size_t protocol_build_heartbeat(protocol_sender_t *sender, uint8_t *buffer, size_t buffer_size)
{
    uint8_t seq;
    uint8_t crc_input[3];

    if(sender == NULL || buffer == NULL || buffer_size < 6U) {
        return 0U;
    }

    seq = protocol_next_seq(sender);

    buffer[0] = PROTOCOL_SYNC1;
    buffer[1] = PROTOCOL_SYNC2;
    buffer[2] = PROTOCOL_MSG_HEARTBEAT;
    buffer[3] = seq;
    buffer[4] = 0U;

    crc_input[0] = buffer[2];
    crc_input[1] = buffer[3];
    crc_input[2] = buffer[4];
    buffer[5] = protocol_crc8_dallas_maxim(crc_input, sizeof(crc_input));

    return 6U;
}

void protocol_rx_init(protocol_rx_state_t *rx)
{
    if(rx == NULL) {
        return;
    }

    memset(rx, 0, sizeof(*rx));
    rx->stage = PROTOCOL_RX_WAIT_SYNC1;
}

bool protocol_rx_consume(protocol_rx_state_t *rx, uint8_t byte, protocol_frame_t *out_frame)
{
    if(rx == NULL || out_frame == NULL) {
        return false;
    }

    switch(rx->stage) {
        case PROTOCOL_RX_WAIT_SYNC1:
            if(byte == PROTOCOL_SYNC1) {
                rx->stage = PROTOCOL_RX_WAIT_SYNC2;
            }
            break;

        case PROTOCOL_RX_WAIT_SYNC2:
            if(byte == PROTOCOL_SYNC2) {
                rx->stage = PROTOCOL_RX_WAIT_TYPE;
            }
            else {
                rx->stage = PROTOCOL_RX_WAIT_SYNC1;
            }
            break;

        case PROTOCOL_RX_WAIT_TYPE:
            rx->frame.type = byte;
            rx->stage = PROTOCOL_RX_WAIT_SEQ;
            break;

        case PROTOCOL_RX_WAIT_SEQ:
            rx->frame.seq = byte;
            rx->stage = PROTOCOL_RX_WAIT_LEN;
            break;

        case PROTOCOL_RX_WAIT_LEN:
            rx->frame.len = byte;
            if(rx->frame.len > PROTOCOL_MAX_DATA_SIZE) {
                rx->stage = PROTOCOL_RX_WAIT_SYNC1;
            }
            else if(rx->frame.len == 0U) {
                rx->stage = PROTOCOL_RX_WAIT_CRC;
            }
            else {
                rx->data_index = 0U;
                rx->stage = PROTOCOL_RX_WAIT_DATA;
            }
            break;

        case PROTOCOL_RX_WAIT_DATA:
            rx->frame.data[rx->data_index++] = byte;
            if(rx->data_index >= rx->frame.len) {
                rx->stage = PROTOCOL_RX_WAIT_CRC;
            }
            break;

        case PROTOCOL_RX_WAIT_CRC:
        {
            uint8_t crc_buffer[3 + PROTOCOL_MAX_DATA_SIZE];
            size_t crc_length;

            crc_buffer[0] = rx->frame.type;
            crc_buffer[1] = rx->frame.seq;
            crc_buffer[2] = rx->frame.len;
            memcpy(&crc_buffer[3], rx->frame.data, rx->frame.len);
            crc_length = 3U + rx->frame.len;
            rx->crc = protocol_crc8_dallas_maxim(crc_buffer, crc_length);
            rx->stage = PROTOCOL_RX_WAIT_SYNC1;

            if(rx->crc == byte) {
                *out_frame = rx->frame;
                return true;
            }
            break;
        }
    }

    return false;
}

uint8_t protocol_crc8_dallas_maxim(const uint8_t *data, size_t length)
{
    uint8_t crc = 0U;

    if(data == NULL) {
        return 0U;
    }

    for(size_t index = 0; index < length; ++index) {
        uint8_t in_byte = data[index];
        for(uint8_t bit = 0; bit < 8U; ++bit) {
            uint8_t mix = (uint8_t)((crc ^ in_byte) & 0x01U);
            crc >>= 1U;
            if(mix != 0U) {
                crc ^= 0x8CU;
            }
            in_byte >>= 1U;
        }
    }

    return crc;
}
