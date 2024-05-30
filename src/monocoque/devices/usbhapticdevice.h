#ifndef _USBHAPTICDEVICE_H
#define _USBHAPTICDEVICE_H

#include <hidapi/hidapi.h>
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"


typedef enum
{
    HAPTIC_UNKNOWN     = 0,
    HAPTIC_GENERIC     = 1
}
HapticType;

typedef struct
{
    int id;
    HapticType type;
    double state;
    int value0;
    int value1;
    VibrationEffectType effecttype;
    MonocoqueTyreIdentifier tyre;
    FILE* handle;
    char* dev;
}
USBGenericHapticDevice;


int usbhapticdev_update(USBGenericHapticDevice* hapticdevice, SimData* simdata);
int usbhapticdev_init(USBGenericHapticDevice* hapticdevice, DeviceSettings* ds);
int usbhapticdev_free(USBGenericHapticDevice* hapticdevice);

#endif
