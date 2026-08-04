#include "Comms.h"

#include <poll.h>
#include <string.h>
#include <unistd.h>

#include "Platform/Logging.h"
#include "Platform/Time_base.h"

#define COMMS_HEARTBEAT_PERIOD_MS 1000U
#define COMMS_POLL_WAIT_MS 10
#define COMMS_CONTROL_SNAPSHOT_PAYLOAD_LEN 16U
#define COMMS_STATUS_REPORT_PAYLOAD_LEN 27U
#define COMMS_FAULT_REPORT_PAYLOAD_LEN 5U
#define COMMS_ACK_PAYLOAD_LEN 2U
#define COMMS_NACK_PAYLOAD_LEN 3U

/*
 * The comms layer owns the SOM side of the serial protocol. It deliberately exposes only
 * a decoded status snapshot to the rest of the app, so UI code never has to know about
 * frame bytes, CRCs, or UART read/write details.
 */
static void comms_process_serial_rx(comms_t *comms, uint64_t now_ms);
static void comms_send_heartbeat(comms_t *comms, uint64_t now_ms);
static void comms_decode_frame(comms_t *comms, const protocol_frame_t *frame);
static bool comms_decode_status_report(comms_mcu_status_report_t *report, const protocol_frame_t *frame);
static bool comms_decode_fault_report(comms_mcu_fault_report_t *report, const protocol_frame_t *frame);
static bool comms_decode_ack(comms_mcu_command_response_t *response, const protocol_frame_t *frame);
static bool comms_decode_nack(comms_mcu_command_response_t *response, const protocol_frame_t *frame);
static uint16_t comms_decode_u16_le(const uint8_t *data);
static int16_t comms_decode_i16_le(const uint8_t *data);

bool comms_init(comms_t *comms, const char *device_path, int baud_rate)
{
    if (comms == NULL)
    {
        return false;
    }

    memset(comms, 0, sizeof(*comms));
    protocol_rx_init(&comms->protocol_rx);
    protocol_sender_init(&comms->protocol_sender, 1U);

    /*
     * /dev/ttyS1 is the SOM's serial device connected to the MCU. Opening it does not mean
     * the MCU is alive yet; link state becomes OK only after valid frames arrive.
     */
    comms->status.serial_ready = transport_serial_open(&comms->serial, device_path, baud_rate);
    comms->last_heartbeat_ms = time_base_now_ms();
    comms_link_update(&comms->status, comms->last_heartbeat_ms);

    return comms->status.serial_ready;
}

void comms_update(comms_t *comms, uint64_t now_ms)
{
    struct pollfd serial_poll_request;
    int poll_result;

    if (comms == NULL)
    {
        return;
    }

    if (comms->status.serial_ready)
    {
        /*
         * Linux exposes serial ports as file descriptors. poll() lets us wait briefly for
         * incoming bytes instead of spinning the CPU while the MCU is quiet.
         */
        serial_poll_request.fd = comms->serial.fd;
        serial_poll_request.events = POLLIN;
        serial_poll_request.revents = 0;

        poll_result = poll(&serial_poll_request, 1, COMMS_POLL_WAIT_MS);
        if (poll_result > 0 && (serial_poll_request.revents & POLLIN) != 0)
        {
            comms_process_serial_rx(comms, now_ms);
        }
    }
    else
    {
        /*
         * If the serial port is missing, still sleep briefly so a failed comms setup does
         * not make the app spin at 100% CPU.
         */
        usleep(COMMS_POLL_WAIT_MS * 1000U);
    }

    /*
     * Heartbeats are sent by the SOM so the MCU can tell whether the higher-level app is
     * alive. Link state is then derived from what the MCU has sent back recently.
     */
    comms_send_heartbeat(comms, now_ms);
    comms_link_update(&comms->status, now_ms);
}

void comms_shutdown(comms_t *comms)
{
    if (comms == NULL)
    {
        return;
    }

    transport_serial_close(&comms->serial);
}

const comms_status_t *comms_get_status(const comms_t *comms)
{
    if (comms == NULL)
    {
        return NULL;
    }

    return &comms->status;
}

bool comms_is_serial_ready(const comms_t *comms)
{
    return (comms != NULL) && comms->status.serial_ready;
}

bool comms_send_control_snapshot(comms_t *comms, const uint8_t *payload, uint8_t payload_size)
{
    uint8_t frame_buffer[PROTOCOL_MAX_FRAME_SIZE];
    size_t frame_length;

    if (comms == NULL || payload == NULL ||
        payload_size != COMMS_CONTROL_SNAPSHOT_PAYLOAD_LEN)
    {
        return false;
    }

    if (!comms->status.serial_ready)
    {
        log_error("comms_send_control_snapshot: serial not ready");
        return false;
    }

    /*
     * The app/orchestrator decides whether a snapshot is safe to send. Comms only frames
     * the exact payload and writes it to the MCU serial link.
     */
    frame_length = protocol_build_frame(&comms->protocol_sender,
                                        PROTOCOL_MSG_CONTROL_SNAPSHOT,
                                        payload,
                                        payload_size,
                                        frame_buffer,
                                        sizeof(frame_buffer));
    if (frame_length == 0U)
    {
        log_error("comms_send_control_snapshot: frame build failed");
        return false;
    }

    if (!transport_serial_write_all(&comms->serial, frame_buffer, frame_length))
    {
        log_error("comms_send_control_snapshot: serial write failed");
        return false;
    }

    log_info("tx: CONTROL_SNAPSHOT len=16");
    return true;
}

static void comms_process_serial_rx(comms_t *comms, uint64_t now_ms)
{
    uint8_t buffer[128];
    ssize_t bytes_read;
    protocol_frame_t frame;
    ssize_t index;

    bytes_read = transport_serial_read(&comms->serial, buffer, sizeof(buffer));
    if (bytes_read <= 0)
    {
        return;
    }

    for (index = 0; index < bytes_read; ++index)
    {
        /*
         * The protocol parser consumes one byte at a time and only returns true when a full
         * frame has been found and its CRC has passed.
         */
        if (protocol_rx_consume(&comms->protocol_rx, buffer[index], &frame))
        {
            comms->status.last_rx_ms = now_ms;
            comms->status.last_rx_type = frame.type;
            comms->status.last_rx_seq = frame.seq;
            comms->status.last_rx_len = frame.len;
            comms_decode_frame(comms, &frame);
        }
    }
}

static void comms_send_heartbeat(comms_t *comms, uint64_t now_ms)
{
    uint8_t frame_buffer[PROTOCOL_MAX_FRAME_SIZE];
    size_t frame_length;

    if (!comms->status.serial_ready)
    {
        return;
    }

    if ((now_ms - comms->last_heartbeat_ms) < COMMS_HEARTBEAT_PERIOD_MS)
    {
        return;
    }

    /* Build a tiny no-payload frame: sync bytes, type, sequence, length zero, and CRC. */
    frame_length = protocol_build_heartbeat(&comms->protocol_sender, frame_buffer, sizeof(frame_buffer));
    if (frame_length == 0U)
    {
        return;
    }

    if (transport_serial_write_all(&comms->serial, frame_buffer, frame_length))
    {
        comms->status.heartbeat_count++;
        comms->status.heartbeat_running = true;
        comms->last_heartbeat_ms = now_ms;
    }
}

static void comms_decode_frame(comms_t *comms, const protocol_frame_t *frame)
{
    if (comms == NULL || frame == NULL)
    {
        return;
    }

    if (frame->type == PROTOCOL_MSG_STATUS_REPORT)
    {
        if (!comms_decode_status_report(&comms->status.mcu_status, frame))
        {
            comms->status.mcu_status.valid = false;
            log_errorf("rx: bad STATUS_REPORT len=%u", (unsigned int)frame->len);
        }
    }
    else if (frame->type == PROTOCOL_MSG_FAULT_REPORT)
    {
        if (!comms_decode_fault_report(&comms->status.mcu_faults, frame))
        {
            comms->status.mcu_faults.valid = false;
            log_errorf("rx: bad FAULT_REPORT len=%u", (unsigned int)frame->len);
        }
    }
    else if (frame->type == PROTOCOL_MSG_ACK)
    {
        if (!comms_decode_ack(&comms->status.command_response, frame))
        {
            comms->status.command_response.valid = false;
            log_errorf("rx: bad ACK len=%u", (unsigned int)frame->len);
        }
    }
    else if (frame->type == PROTOCOL_MSG_NACK)
    {
        if (!comms_decode_nack(&comms->status.command_response, frame))
        {
            comms->status.command_response.valid = false;
            log_errorf("rx: bad NACK len=%u", (unsigned int)frame->len);
        }
    }
    /*
     * Unknown frame types are ignored for now. As the protocol grows, each accepted message
     * should get an explicit branch here so unsupported traffic stays harmless.
     */
}

static bool comms_decode_status_report(comms_mcu_status_report_t *report, const protocol_frame_t *frame)
{
    uint8_t index;
    uint8_t valve_index;

    if (report == NULL || frame == NULL || frame->len != COMMS_STATUS_REPORT_PAYLOAD_LEN)
    {
        return false;
    }

    /*
     * Decode fields in the exact order the MCU writes them. The index variable is a cursor
     * through the payload, which keeps the protocol layout visible in one compact block.
     */
    index = 0U;
    report->mash_target_c = frame->data[index++];
    report->boil_target_c = frame->data[index++];
    report->mash_temp_c_x10 = comms_decode_i16_le(&frame->data[index]);
    index = (uint8_t)(index + 2U);
    report->boil_temp_c_x10 = comms_decode_i16_le(&frame->data[index]);
    index = (uint8_t)(index + 2U);
    report->mash_pump_setpoint = frame->data[index++];
    report->boil_pump_setpoint = frame->data[index++];
    report->mash_pump_running = frame->data[index++] != 0U;
    report->boil_pump_running = frame->data[index++] != 0U;
    report->pressure_count = comms_decode_u16_le(&frame->data[index]);
    index = (uint8_t)(index + 2U);
    report->solenoid_state_bits = frame->data[index++];
    report->status_bits = frame->data[index++];
    report->fault_flags = comms_decode_u16_le(&frame->data[index]);
    index = (uint8_t)(index + 2U);

    for (valve_index = 0U; valve_index < 11U; valve_index++)
    {
        report->valve_state[valve_index] = frame->data[index++];
    }

    report->valid = true;
    return true;
}

static bool comms_decode_fault_report(comms_mcu_fault_report_t *report, const protocol_frame_t *frame)
{
    if (report == NULL || frame == NULL || frame->len != COMMS_FAULT_REPORT_PAYLOAD_LEN)
    {
        return false;
    }

    report->active_fault_flags = comms_decode_u16_le(&frame->data[0]);
    report->latched_fault_flags = comms_decode_u16_le(&frame->data[2]);
    report->primary_reason = frame->data[4];
    report->valid = true;
    return true;
}

static bool comms_decode_ack(comms_mcu_command_response_t *response, const protocol_frame_t *frame)
{
    if (response == NULL || frame == NULL || frame->len != COMMS_ACK_PAYLOAD_LEN)
    {
        return false;
    }

    response->valid = true;
    response->accepted = true;
    response->response_seq = frame->seq;
    response->referenced_type = frame->data[0];
    response->referenced_seq = frame->data[1];
    response->nack_reason = 0U;

    log_infof("rx: ACK type=%u seq=%u",
              (unsigned int)response->referenced_type,
              (unsigned int)response->referenced_seq);
    return true;
}

static bool comms_decode_nack(comms_mcu_command_response_t *response, const protocol_frame_t *frame)
{
    if (response == NULL || frame == NULL || frame->len != COMMS_NACK_PAYLOAD_LEN)
    {
        return false;
    }

    response->valid = true;
    response->accepted = false;
    response->response_seq = frame->seq;
    response->referenced_type = frame->data[0];
    response->referenced_seq = frame->data[1];
    response->nack_reason = frame->data[2];

    log_errorf("rx: NACK type=%u seq=%u reason=%u",
               (unsigned int)response->referenced_type,
               (unsigned int)response->referenced_seq,
               (unsigned int)response->nack_reason);
    return true;
}

static uint16_t comms_decode_u16_le(const uint8_t *data)
{
    /* The wire protocol stores 16-bit values least-significant byte first. */
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8U);
}

static int16_t comms_decode_i16_le(const uint8_t *data)
{
    return (int16_t)comms_decode_u16_le(data);
}
