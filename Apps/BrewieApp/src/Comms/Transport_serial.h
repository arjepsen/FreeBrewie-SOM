#ifndef FREEBREWIE_TRANSPORT_SERIAL_H
#define FREEBREWIE_TRANSPORT_SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct
{
    /*
     * Linux represents an open device as a small integer called a file descriptor. We keep
     * the conventional field name "fd" because it is the name used by open/read/write/poll,
     * but code that uses this struct should describe what the descriptor is for.
     */
    int fd;
} transport_serial_t;

bool transport_serial_open(transport_serial_t *port, const char *device_path, int baud_rate);
void transport_serial_close(transport_serial_t *port);
ssize_t transport_serial_read(transport_serial_t *port, uint8_t *buffer, size_t buffer_size);
bool transport_serial_write_all(transport_serial_t *port, const uint8_t *buffer, size_t length);

#endif
