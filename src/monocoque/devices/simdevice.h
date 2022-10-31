#ifndef _SIMDEVICE_H
#define _SIMDEVICE_H

#include <stdbool.h>

#include "usbdevice.h"
#include "sounddevice.h"
#include "serialdevice.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simdata.h"


typedef struct
{
    int id;
    bool initialized;
    DeviceType type;

    union
    {
        USBDevice usbdevice;
        SoundDevice sounddevice;
        SerialDevice serialdevice;
    } d;
}
SimDevice;

int devupdate(SimDevice* simdevice, SimData* simdata);

int devinit(SimDevice* simdevice, DeviceSettings* ds);

int devfree(SimDevice* simdevice);

#endif
