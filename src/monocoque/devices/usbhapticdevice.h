#ifndef _USBHAPTICDEVICE_H
#define _USBHAPTICDEVICE_H

#include <hidapi/hidapi.h>
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"


typedef enum
{
    USBHAPTIC_UNKNOWN                  = 0,
    USBHAPTIC_CSLELITEV3PEDALS         = 1,
    USBHAPTIC_SIMAGICP1000PEDALS       = 2
}
HapticType;

typedef struct
{
    int id;
    HapticType type;
    double state;
    double threshold;
    int value0;
    int value1;
    VibrationEffectType effecttype;
    FILE* filehandle;
    hid_device* handle;
    char* dev;
}
USBGenericHapticDevice;


int usbhapticdev_update(USBGenericHapticDevice* hapticdevice, SimData* simdata, int tyre, int useconfig, int* configcheck, char* configfile);
int usbhapticdev_init(USBGenericHapticDevice* hapticdevice, DeviceSettings* ds);
int usbhapticdev_free(USBGenericHapticDevice* hapticdevice);

#endif
