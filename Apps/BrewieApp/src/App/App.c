#include "App.h"

#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Time.h"
#include "lvgl.h"

#define APP_SERIAL_DEVICE "/dev/ttyS1"
#define APP_HEARTBEAT_PERIOD_MS 1000U
#define APP_LINK_TIMEOUT_MS 3000U
#define APP_LOOP_WAIT_MS 10

static const char *protocol_type_name(uint8_t type);
static void app_process_serial_rx(app_t *app, uint64_t now_ms);
static void app_send_heartbeat(app_t *app, uint64_t now_ms);

bool app_init(app_t *app)
{
    if (app == NULL)
    {
        return false;
    }

    memset(app, 0, sizeof(*app));
    protocol_rx_init(&app->protocol_rx);
    protocol_sender_init(&app->protocol_sender, 1U);

    lv_init();

    fprintf(stderr, "app_init: headless mode\n");
    fprintf(stderr, "app_init: before serial_open\n");

    app->display_ok = false;
    app->serial_ok = serial_open(&app->serial, APP_SERIAL_DEVICE, 115200);

    fprintf(stderr, "app_init: after serial_open (%d)\n", app->serial_ok ? 1 : 0);

    app->last_heartbeat_ms = time_now_ms();
    app->last_ui_refresh_ms = app->last_heartbeat_ms;
    app->last_rx_ms = 0U;
    app->heartbeat_count = 0U;
    app->heartbeat_running = false;

    return true;
}

void app_update(app_t *app)
{
    uint64_t now_ms;
    struct pollfd poll_fd;
    int poll_result;

    if (app == NULL)
    {
        return;
    }

    now_ms = time_now_ms();

    if (app->serial_ok)
    {
        poll_fd.fd = app->serial.fd;
        poll_fd.events = POLLIN;
        poll_fd.revents = 0;

        poll_result = poll(&poll_fd, 1, APP_LOOP_WAIT_MS);
        if (poll_result > 0 && (poll_fd.revents & POLLIN) != 0)
        {
            app_process_serial_rx(app, now_ms);
        }
    }
    else
    {
        usleep(APP_LOOP_WAIT_MS * 1000U);
    }

    app_send_heartbeat(app, now_ms);
}

void app_shutdown(app_t *app)
{
    if (app == NULL)
    {
        return;
    }

    serial_close(&app->serial);
}

static const char *protocol_type_name(uint8_t type)
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

static void app_process_serial_rx(app_t *app, uint64_t now_ms)
{
    uint8_t buffer[128];
    ssize_t bytes_read;
    protocol_frame_t frame;

    bytes_read = serial_read(&app->serial, buffer, sizeof(buffer));
    if (bytes_read <= 0)
    {
        return;
    }

    for (ssize_t index = 0; index < bytes_read; ++index)
    {
        if (protocol_rx_consume(&app->protocol_rx, buffer[index], &frame))
        {
            fprintf(stderr,
                    "rx: %s seq=%u len=%u\n",
                    protocol_type_name(frame.type),
                    (unsigned int)frame.seq,
                    (unsigned int)frame.len);

            app->last_rx_ms = now_ms;
        }
    }
}

static void app_send_heartbeat(app_t *app, uint64_t now_ms)
{
    uint8_t frame_buffer[PROTOCOL_MAX_FRAME_SIZE];
    size_t frame_length;

    if (!app->serial_ok)
    {
        return;
    }

    if ((now_ms - app->last_heartbeat_ms) < APP_HEARTBEAT_PERIOD_MS)
    {
        return;
    }

    frame_length = protocol_build_heartbeat(&app->protocol_sender,
                                            frame_buffer,
                                            sizeof(frame_buffer));
    if (frame_length == 0U)
    {
        return;
    }

    if (serial_write_all(&app->serial, frame_buffer, frame_length))
    {
        app->heartbeat_count++;
        app->heartbeat_running = true;
        app->last_heartbeat_ms = now_ms;

        fprintf(stderr,
                "heartbeat sent %lu\n",
                (unsigned long)app->heartbeat_count);
    }
}