#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "simdevice.h"
#include "serialdevice.h"
#include "serial/arduino.h"
#include "../helper/parameters.h"
#include "../simulatorapi/simdata.h"
#include "../slog/slog.h"

int serialdev_update(SimDevice* this, SimData* simdata)
{
    SerialDevice* serialdevice = (void *) this->derived;

    arduino_update(serialdevice, simdata);
    return 0;
}

int serialdev_free(SimDevice* this)
{
    SerialDevice* serialdevice = (void *) this->derived;

    arduino_free(serialdevice);

    free(serialdevice);
    return 0;
}

int serialdev_init(SerialDevice* serialdevice, const char* portdev)
{
    slogi("initializing serial device...");
    int error = 0;

    serialdevice->type = SERIALDEV_UNKNOWN;
    serialdevice->type = SERIALDEV_ARDUINO;

    error = arduino_init(serialdevice, portdev);

    return error;
}

static const vtable serial_simdevice_vtable = { &serialdev_update, &serialdev_free };

SerialDevice* new_serial_device(DeviceSettings* ds) {

    SerialDevice* this = (SerialDevice*) malloc(sizeof(SerialDevice));

    this->m.update = &update;
    this->m.free = &simdevfree;
    this->m.derived = this;
    this->m.vtable = &serial_simdevice_vtable;

    int error = serialdev_init(this, ds->serialdevsettings.portdev);

    if (error != 0)
    {
        free(this);
        return NULL;
    }

    return this;
}
