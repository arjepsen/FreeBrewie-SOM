#ifndef FREEBREWIE_SERIAL_H
#define FREEBREWIE_SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <sys/types.h>

typedef struct
{
    int fd;
} serial_port_t;

bool serial_open(serial_port_t *port, const char *device_path, int baud_rate);
void serial_close(serial_port_t *port);
ssize_t serial_read(serial_port_t *port, uint8_t *buffer, size_t buffer_size);
bool serial_write_all(serial_port_t *port, const uint8_t *buffer, size_t length);

#endif
