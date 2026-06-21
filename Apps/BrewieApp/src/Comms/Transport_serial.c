#include "Transport_serial.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "Platform/Logging.h"

static speed_t transport_serial_baud_to_speed(int baud_rate)
{
    switch (baud_rate)
    {
    case 115200:
        return B115200;
    case 57600:
        return B57600;
    case 38400:
        return B38400;
    case 19200:
        return B19200;
    case 9600:
        return B9600;
    default:
        return B115200;
    }
}

bool transport_serial_open(transport_serial_t *port, const char *device_path, int baud_rate)
{
    struct termios tty;
    speed_t speed;

    if (port == NULL || device_path == NULL)
    {
        log_error("transport_serial_open: invalid arguments");
        return false;
    }

    port->fd = open(device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (port->fd < 0)
    {
        log_errorf("transport_serial_open: open(%s) failed: %s", device_path, strerror(errno));
        return false;
    }

    if (tcgetattr(port->fd, &tty) != 0)
    {
        log_errorf("transport_serial_open: tcgetattr(%s) failed: %s", device_path, strerror(errno));
        close(port->fd);
        port->fd = -1;
        return false;
    }

    cfmakeraw(&tty);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    speed = transport_serial_baud_to_speed(baud_rate);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    if (tcsetattr(port->fd, TCSANOW, &tty) != 0)
    {
        log_errorf("transport_serial_open: tcsetattr(%s) failed: %s", device_path, strerror(errno));
        close(port->fd);
        port->fd = -1;
        return false;
    }

    log_infof("serial_open: %s opened on fd %d", device_path, port->fd);
    return true;
}

void transport_serial_close(transport_serial_t *port)
{
    if (port == NULL)
    {
        return;
    }

    if (port->fd >= 0)
    {
        close(port->fd);
        port->fd = -1;
    }
}

ssize_t transport_serial_read(transport_serial_t *port, uint8_t *buffer, size_t buffer_size)
{
    if (port == NULL || buffer == NULL || buffer_size == 0U || port->fd < 0)
    {
        return -1;
    }

    return read(port->fd, buffer, buffer_size);
}

bool transport_serial_write_all(transport_serial_t *port, const uint8_t *buffer, size_t length)
{
    size_t offset;

    if (port == NULL || buffer == NULL || port->fd < 0)
    {
        return false;
    }

    offset = 0U;
    while (offset < length)
    {
        ssize_t written = write(port->fd, &buffer[offset], length - offset);
        if (written < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                usleep(1000);
                continue;
            }
            return false;
        }
        offset += (size_t)written;
    }

    return true;
}
