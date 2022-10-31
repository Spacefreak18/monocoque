#ifndef _USBDEVICE_H
#define _USBDEVICE_H

#include "tachdevice.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simdata.h"

typedef enum
{
    USBDEV_UNKNOWN       = 0,
    USBDEV_TACHOMETER    = 1
}
USBType;

typedef struct
{
    int id;
    USBType type;
    union
    {
        TachDevice tachdevice;
    } u;
}
USBDevice;

int usbdev_update(USBDevice* usbdevice, SimData* simdata);
int usbdev_init(USBDevice* usbdevice, DeviceSettings* ds);
int usbdev_free(USBDevice* usbdevice);

#endif
