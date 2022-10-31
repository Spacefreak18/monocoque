#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serialdevice.h"
#include "serial/arduino.h"
#include "../helper/parameters.h"
#include "../simulatorapi/simdata.h"
#include "../slog/slog.h"

int serialdev_update(SerialDevice* serialdevice, SimData* simdata)
{
    arduino_update(serialdevice, simdata);
    return 0;
}

int serialdev_free(SerialDevice* serialdevice)
{
    arduino_free(serialdevice);
    return 0;
}

int serialdev_init(SerialDevice* serialdevice)
{
    slogi("initializing serial device...");
    int error = 0;

    serialdevice->type = SERIALDEV_UNKNOWN;
    serialdevice->type = SERIALDEV_ARDUINO;

    error = arduino_init(serialdevice);

    return error;
}
