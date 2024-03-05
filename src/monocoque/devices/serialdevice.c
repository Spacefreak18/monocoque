#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "simdevice.h"
#include "serialdevice.h"
#include "serial/arduino.h"
#include "../helper/parameters.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../slog/slog.h"

int serialdev_update(SimDevice* this, SimData* simdata)
{
    SerialDevice* serialdevice = (void *) this->derived;



    arduino_update(serialdevice, simdata, sizeof(SimData));
    return 0;
}

int arduino_shiftlights_update(SimDevice* this, SimData* simdata)
{
    SerialDevice* serialdevice = (void *) this->derived;
    int result = 1;


    serialdevice->u.shiftlightsdata.maxrpm = simdata->maxrpm;
    serialdevice->u.shiftlightsdata.rpm = simdata->rpms;
    slogt("Updating arduino device rpms to %i", serialdevice->u.shiftlightsdata.rpm);
    // we can add configs to set all the colors
    // i can move the size to the initialization since it should not change
    size_t size = sizeof(ShiftLightsData);

    arduino_update(serialdevice, &serialdevice->u.shiftlightsdata, size);
    return result;
}

int arduino_simwind_update(SimDevice* this, SimData* simdata)
{
    SerialDevice* serialdevice = (void *) this->derived;
    int result = 1;

    serialdevice->u.simwinddata.velocity = simdata->velocity;
    // this can be added to the config, all config dependent can be added to init
    serialdevice->u.simwinddata.fanpower = 0.6;
    size_t size = sizeof(SimWindData);

    arduino_update(serialdevice, &serialdevice->u.simwinddata, size);

    return result;
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
static const vtable arduino_shiftlights_vtable = { &arduino_shiftlights_update, &serialdev_free };
static const vtable arduino_simwind_vtable = { &arduino_simwind_update, &serialdev_free };

SerialDevice* new_serial_device(DeviceSettings* ds) {

    SerialDevice* this = (SerialDevice*) malloc(sizeof(SerialDevice));

    this->m.update = &update;
    this->m.free = &simdevfree;
    this->m.derived = this;
    this->m.vtable = &serial_simdevice_vtable;

    slogt("Attempting to configure arduino device with subtype: %i", ds->dev_subtype);
    switch (ds->dev_subtype) {
        case (SIMDEVTYPE_SHIFTLIGHTS):
            this->devicetype = ARDUINODEV__SHIFTLIGHTS;
            this->m.vtable = &arduino_shiftlights_vtable;
            slogi("Initializing arduino device for shiftlights.");
            break;
        case (SIMDEVTYPE_SIMWIND):
            this->devicetype = ARDUINODEV__SIMWIND;
            this->m.vtable = &arduino_simwind_vtable;
            slogi("Initializing arduino devices for sim wind.");
            break;
    }

    int error = serialdev_init(this, ds->serialdevsettings.portdev);

    if (error != 0)
    {
        free(this);
        return NULL;
    }

    return this;
}
