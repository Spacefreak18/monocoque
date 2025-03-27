#ifndef _SERIALADAPTER_H
#define _SERIALADAPTER_H

#include <stdint.h>
#include <stdbool.h>
#include <libserialport.h>

#include "simdevice.h"

typedef struct
{
    char* portname;
    struct sp_port* port;
    uint8_t refs;
    bool open;
    bool openfail;
    bool busy;
}
monocoque_serial_device;

static monocoque_serial_device monocoque_serial_devices[20];

int monocoque_serial_write(uint8_t serialdevicenum, void* data, size_t size, int timeout);
int monocoque_serial_write_block(uint8_t serialdevicenum, void* data, size_t size, int timeout);
int monocoque_serial_read_block(uint8_t serialdevicenum, void* data, size_t size, int timeout);
int monocoque_serial_open(SerialDevice* serialdevice, const char* port);
int monocoque_serial_free(SerialDevice* serialdevice);

#endif
