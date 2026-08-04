#include "Protocol.h"

#include <string.h>

/*
 * Protocol.c is intentionally byte-oriented. UART read() may return half a frame, one frame,
 * or several frames at once, so the receiver keeps its own small state machine instead of
 * assuming reads line up with packet boundaries.
 */
static uint8_t protocol_next_seq(protocol_sender_t *sender)
{
    uint8_t seq;

    seq = sender->next_seq;
    if (seq == 0U)
    {
        /* Sequence zero is reserved so "no sequence yet" stays easy to spot in logs. */
        seq = 1U;
    }

    sender->next_seq = (uint8_t)(seq + 1U);
    if (sender->next_seq == 0U)
    {
        sender->next_seq = 1U;
    }

    return seq;
}

void protocol_sender_init(protocol_sender_t *sender, uint8_t first_seq)
{
    if (sender == NULL)
    {
        return;
    }

    sender->next_seq = first_seq;
    if (sender->next_seq == 0U)
    {
        sender->next_seq = 1U;
    }
}

size_t protocol_build_frame(protocol_sender_t *sender,
                            uint8_t type,
                            const uint8_t *payload,
                            uint8_t payload_size,
                            uint8_t *buffer,
                            size_t buffer_size)
{
    uint8_t seq;
    uint8_t crc_input[3 + PROTOCOL_MAX_DATA_SIZE];
    size_t frame_size;
    size_t crc_input_size;

    frame_size = 2U + 1U + 1U + 1U + payload_size + 1U;
    if (sender == NULL || buffer == NULL || payload_size > PROTOCOL_MAX_DATA_SIZE ||
        buffer_size < frame_size || (payload_size > 0U && payload == NULL))
    {
        return 0U;
    }

    seq = protocol_next_seq(sender);

    /*
     * All outgoing frames share the same envelope. The CRC intentionally excludes the sync
     * bytes, matching the MCU receiver.
     */
    buffer[0] = PROTOCOL_SYNC1;
    buffer[1] = PROTOCOL_SYNC2;
    buffer[2] = type;
    buffer[3] = seq;
    buffer[4] = payload_size;
    if (payload_size > 0U)
    {
        memcpy(&buffer[5], payload, payload_size);
    }

    crc_input[0] = buffer[2];
    crc_input[1] = buffer[3];
    crc_input[2] = buffer[4];
    if (payload_size > 0U)
    {
        memcpy(&crc_input[3], payload, payload_size);
    }
    crc_input_size = 3U + payload_size;
    buffer[5U + payload_size] = protocol_crc8_dallas_maxim(crc_input, crc_input_size);

    return frame_size;
}

size_t protocol_build_heartbeat(protocol_sender_t *sender, uint8_t *buffer, size_t buffer_size)
{
    return protocol_build_frame(sender, PROTOCOL_MSG_HEARTBEAT, NULL, 0U, buffer, buffer_size);
}

void protocol_rx_init(protocol_rx_state_t *rx)
{
    if (rx == NULL)
    {
        return;
    }

    memset(rx, 0, sizeof(*rx));
    rx->stage = PROTOCOL_RX_WAIT_SYNC1;
}

bool protocol_rx_consume(protocol_rx_state_t *rx, uint8_t byte, protocol_frame_t *out_frame)
{
    if (rx == NULL || out_frame == NULL)
    {
        return false;
    }

    switch (rx->stage)
    {
    case PROTOCOL_RX_WAIT_SYNC1:
        /* Ignore all noise until the first sync byte appears. */
        if (byte == PROTOCOL_SYNC1)
        {
            rx->stage = PROTOCOL_RX_WAIT_SYNC2;
        }
        break;

    case PROTOCOL_RX_WAIT_SYNC2:
        if (byte == PROTOCOL_SYNC2)
        {
            rx->stage = PROTOCOL_RX_WAIT_TYPE;
        }
        else
        {
            /* Bad second sync byte: abandon this candidate frame and search again. */
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
        if (rx->frame.len > PROTOCOL_MAX_DATA_SIZE)
        {
            /* Oversized payload would overflow the frame buffer, so drop this frame. */
            rx->stage = PROTOCOL_RX_WAIT_SYNC1;
        }
        else if (rx->frame.len == 0U)
        {
            rx->stage = PROTOCOL_RX_WAIT_CRC;
        }
        else
        {
            rx->data_index = 0U;
            rx->stage = PROTOCOL_RX_WAIT_DATA;
        }
        break;

    case PROTOCOL_RX_WAIT_DATA:
        rx->frame.data[rx->data_index++] = byte;
        if (rx->data_index >= rx->frame.len)
        {
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

        if (rx->crc == byte)
        {
            /* The caller gets a copy so the receiver can immediately start finding more frames. */
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
    uint8_t crc;
    size_t index;

    crc = 0U;
    if (data == NULL)
    {
        return 0U;
    }

    for (index = 0U; index < length; ++index)
    {
        uint8_t in_byte;
        uint8_t bit;

        in_byte = data[index];
        for (bit = 0U; bit < 8U; ++bit)
        {
            uint8_t mix;

            mix = (uint8_t)((crc ^ in_byte) & 0x01U);
            crc >>= 1U;
            if (mix != 0U)
            {
                /* 0x8C is the reflected Dallas/Maxim CRC-8 polynomial. */
                crc ^= 0x8CU;
            }
            in_byte >>= 1U;
        }
    }

    return crc;
}
