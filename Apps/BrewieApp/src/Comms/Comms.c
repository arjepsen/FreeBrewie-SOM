#include "Comms.h"

#include <poll.h>
#include <string.h>
#include <unistd.h>

#include "Platform/Logging.h"
#include "Platform/Time_base.h"

#define COMMS_HEARTBEAT_PERIOD_MS 1000U
#define COMMS_POLL_WAIT_MS 10
#define COMMS_STATUS_REPORT_PAYLOAD_LEN 27U
#define COMMS_FAULT_REPORT_PAYLOAD_LEN 5U

static const char *comms_protocol_type_name(uint8_t type);
static void comms_process_serial_rx(comms_t *comms, uint64_t now_ms);
static void comms_send_heartbeat(comms_t *comms, uint64_t now_ms);
static void comms_decode_frame(comms_t *comms, const protocol_frame_t *frame);
static bool comms_decode_status_report(comms_mcu_status_report_t *report, const protocol_frame_t *frame);
static bool comms_decode_fault_report(comms_mcu_fault_report_t *report, const protocol_frame_t *frame);
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

    comms->status.serial_ready = transport_serial_open(&comms->serial, device_path, baud_rate);
    comms->last_heartbeat_ms = time_base_now_ms();
    comms_link_update(&comms->status, comms->last_heartbeat_ms);

    return comms->status.serial_ready;
}

void comms_update(comms_t *comms, uint64_t now_ms)
{
    struct pollfd poll_fd;
    int poll_result;

    if (comms == NULL)
    {
        return;
    }

    if (comms->status.serial_ready)
    {
        poll_fd.fd = comms->serial.fd;
        poll_fd.events = POLLIN;
        poll_fd.revents = 0;

        poll_result = poll(&poll_fd, 1, COMMS_POLL_WAIT_MS);
        if (poll_result > 0 && (poll_fd.revents & POLLIN) != 0)
        {
            comms_process_serial_rx(comms, now_ms);
        }
    }
    else
    {
        usleep(COMMS_POLL_WAIT_MS * 1000U);
    }

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

static const char *comms_protocol_type_name(uint8_t type)
{
    switch (type)
    {
    case PROTOCOL_MSG_CONTROL_SNAPSHOT:
        return "CONTROL_SNAPSHOT";
    case PROTOCOL_MSG_HEARTBEAT:
        return "HEARTBEAT";
    case PROTOCOL_MSG_STATUS_REPORT:
        return "STATUS_REPORT";
    case PROTOCOL_MSG_FAULT_REPORT:
        return "FAULT_REPORT";
    case PROTOCOL_MSG_ACK:
        return "ACK";
    case PROTOCOL_MSG_NACK:
        return "NACK";
    case PROTOCOL_MSG_SHUTDOWN_REQUEST:
        return "SHUTDOWN_REQUEST";
    case PROTOCOL_MSG_FAULT_CLEAR_REQUEST:
        return "FAULT_CLEAR_REQUEST";
    default:
        return "UNKNOWN";
    }
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
        if (protocol_rx_consume(&comms->protocol_rx, buffer[index], &frame))
        {
            log_infof("rx: %s seq=%u len=%u",
                      comms_protocol_type_name(frame.type),
                      (unsigned int)frame.seq,
                      (unsigned int)frame.len);

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
        log_infof("heartbeat sent %lu", (unsigned long)comms->status.heartbeat_count);
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
}

static bool comms_decode_status_report(comms_mcu_status_report_t *report, const protocol_frame_t *frame)
{
    uint8_t index;
    uint8_t valve_index;

    if (report == NULL || frame == NULL || frame->len != COMMS_STATUS_REPORT_PAYLOAD_LEN)
    {
        return false;
    }

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

static uint16_t comms_decode_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8U);
}

static int16_t comms_decode_i16_le(const uint8_t *data)
{
    return (int16_t)comms_decode_u16_le(data);
}
