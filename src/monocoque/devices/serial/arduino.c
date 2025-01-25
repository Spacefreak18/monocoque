#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "arduino.h"
#include "../serialadapter.h"
#include "../../slog/slog.h"

#define arduino_timeout 5000

int arduino_update(SerialDevice* serialdevice, void* data, size_t size)
{
    int result = 1;
    slogt("copying %i bytes to arduino device", size);
    result = monocoque_serial_write(serialdevice->id, data, size, arduino_timeout);

    return result;
}

int arduino_init(SerialDevice* serialdevice, const char* portdev)
{
    serialdevice->id = monocoque_serial_open(serialdevice, portdev);
    return serialdevice->id;
}
