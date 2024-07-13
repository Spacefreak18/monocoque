#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "simdevice.h"
#include "serialdevice.h"
#include "hapticeffect.h"
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
    slogt("Updating arduino device speed to %i", serialdevice->u.simwinddata.velocity);
    // this can be added to the config, all config dependent can be added to init
    serialdevice->u.simwinddata.fanpower = 0.6;
    size_t size = sizeof(SimWindData);

    arduino_update(serialdevice, &serialdevice->u.simwinddata, size);

    return result;
}

int arduino_simhaptic_update(SimDevice* this, SimData* simdata)
{
    SerialDevice* serialdevice = (void *) this->derived;

    int result = 1;

    slogt("arduino haptic device updating");

    double play = slipeffect(simdata, this->hapticeffect.effecttype, this->hapticeffect.tyre, this->hapticeffect.threshold, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);

    double rplay = play;
    if(play > 1.0)
    {
        play = 1.0;
    }
    int effectspeed = ceil( 255 * play );

    int motor = serialdevice->motorsposition;

    if (play != serialdevice->state)
    {
        if (motor == 0 || motor == 4 || motor == 7 || motor == 8 || motor == 10 || motor == 11 || motor == 13 || motor == 14)
        {
            serialdevice->u.simhapticdata.effect1 = effectspeed;
            serialdevice->u.simhapticdata.motor1 = 1;
            slogt("Updating arduino haptic device speed motor speed %i on motor %i from original effect %f", serialdevice->u.simhapticdata.effect1, serialdevice->motorsposition, rplay);
        }
        if (motor == 2 || motor == 6 || motor == 8 || motor == 9 || motor == 10 || motor == 11 || motor == 12 || motor == 14)
        {
            serialdevice->u.simhapticdata.effect3 = effectspeed;
            serialdevice->u.simhapticdata.motor3 = 1;
            slogt("Updating arduino haptic device speed motor speed %i on motor %i from original effect %f", serialdevice->u.simhapticdata.effect3, serialdevice->motorsposition, rplay);
        }
        serialdevice->state = play;
    }

    size_t size = sizeof(SimHapticData);

    arduino_update(serialdevice, &serialdevice->u.simhapticdata, size);

    return result;
}

int serialdev_free(SimDevice* this)
{
    SerialDevice* serialdevice = (void *) this->derived;

    arduino_free(serialdevice);

    free(serialdevice);
    return 0;
}

int serialdev_init(SerialDevice* serialdevice, const char* portdev, int motorsposition)
{
    slogi("initializing serial device...");
    int error = 0;

    serialdevice->type = SERIALDEV_UNKNOWN;
    serialdevice->type = SERIALDEV_ARDUINO;

    serialdevice->motorsposition = motorsposition;

    error = arduino_init(serialdevice, portdev);

    return error;
}

static const vtable serial_simdevice_vtable = { &serialdev_update, &serialdev_free };
static const vtable arduino_shiftlights_vtable = { &arduino_shiftlights_update, &serialdev_free };
static const vtable arduino_simwind_vtable = { &arduino_simwind_update, &serialdev_free };
static const vtable arduino_simhaptic_vtable = { &arduino_simhaptic_update, &serialdev_free };

SerialDevice* new_serial_device(DeviceSettings* ds, MonocoqueSettings* ms) {

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
        case (SIMDEVTYPE_SERIALHAPTIC):
            this->devicetype = ARDUINODEV__HAPTIC;
            this->m.vtable = &arduino_simhaptic_vtable;
            this->u.simhapticdata.motor1 = 0;
            this->u.simhapticdata.motor2 = 0;
            this->u.simhapticdata.motor3 = 0;
            this->u.simhapticdata.motor4 = 0;
            this->u.simhapticdata.effect1 = 0;
            this->u.simhapticdata.effect2 = 0;
            this->u.simhapticdata.effect3 = 0;
            this->u.simhapticdata.effect4 = 0;
            this->state = 0;
            slogi("Initializing arduino device for haptic effects.");
            break;
    }

    if(this->devicetype == ARDUINODEV__HAPTIC)
    {
        this->m.hapticeffect.threshold = ds->threshold;
        this->m.hapticeffect.effecttype = ds->effect_type;
        slogt("Haptic effect: %i %i", this->m.hapticeffect.effecttype, ds->effect_type);
        this->m.hapticeffect.tyre = ds->tyre;
        this->m.hapticeffect.useconfig = ms->useconfig;
        this->m.hapticeffect.configcheck = &ms->configcheck;
        this->m.hapticeffect.tyrediameterconfig = ms->tyre_diameter_config;
    }

    int error = serialdev_init(this, ds->serialdevsettings.portdev, ds->serialdevsettings.motorsposition);

    if (error != 0)
    {
        free(this);
        return NULL;
    }

    return this;
}
