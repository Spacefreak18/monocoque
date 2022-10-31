#ifndef _TACHDEVICE_H
#define _TACHDEVICE_H

#include <hidapi/hidapi.h>
#include "../helper/confighelper.h"
#include "../simulatorapi/simdata.h"

//typedef int (*tachdev_update)(int revs);

typedef enum
{
    TACHDEV_UNKNOWN       = 0,
    TACHDEV_REVBURNER     = 1
}
TachType;

typedef struct
{
    int id;
    TachType type;
    bool use_pulses;
    hid_device* handle;
    TachometerSettings tachsettings;
}
TachDevice;

int tachdev_update(TachDevice* tachdevice, SimData* simdata);
int tachdev_init(TachDevice* tachdevice, DeviceSettings* ds);
int tachdev_free(TachDevice* tachdevice);

#endif
