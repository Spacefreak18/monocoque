#ifndef _WHEELDEVICE_H
#define _WHEELDEVICE_H

#include <hidapi/hidapi.h>
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"

//typedef int (*tachdev_update)(int revs);

typedef enum
{
    WHEELDEV_UNKNOWN       = 0,
    WHEELDEV_CAMMUSC5      = 1
}
WheelType;

typedef struct
{
    int id;
    WheelType type;
    hid_device* handle;
}
WheelDevice;

int wheeldev_update(WheelDevice* wheeldevice, SimData* simdata);
int wheeldev_init(WheelDevice* wheeldevice, DeviceSettings* ds);
int wheeldev_free(WheelDevice* wheeldevice);

#endif
