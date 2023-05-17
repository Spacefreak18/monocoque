#ifndef _SIMDEVICE_H
#define _SIMDEVICE_H

#include <stdbool.h>

#include "usbdevice.h"
#include "sounddevice.h"
#include "serialdevice.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simdata.h"

typedef struct SimDevice SimDevice;

struct SimDevice
{
    const void* vtable;
    int (*update)(SimDevice*, SimData*);
    int (*free)(SimDevice*);
    void* derived;
    int id;
    bool initialized;
    DeviceType type;

};

typedef struct {
    int (*update)(SimDevice*, SimData*);
    int (*free)(SimDevice*);
} vtable;

/********* Serial Devices *****/
typedef enum
{
    SERIALDEV_UNKNOWN       = 0,
    SERIALDEV_ARDUINO       = 1
}
SerialType;

typedef struct
{
    SimDevice m;
    int id;
    SerialType type;
    struct sp_port* port;
}
SerialDevice;

int serialdev_update(SimDevice* this, SimData* simdata);
int serialdev_free(SimDevice* this);

SerialDevice* new_serial_device(DeviceSettings* ds);

/********* USB HID Devices *****/
typedef enum
{
    USBDEV_UNKNOWN       = 0,
    USBDEV_TACHOMETER    = 1
}
USBType;

typedef struct
{
    SimDevice m;
    int id;
    USBType type;
    union
    {
        TachDevice tachdevice;
    } u;
}
USBDevice;

int usbdev_update(SimDevice* this, SimData* simdata);
int usbdev_free(SimDevice* this);

USBDevice* new_usb_device(DeviceSettings* ds);


/********* Sound Devices *****/
typedef struct
{
    SimDevice m;
    int id;
    SoundType type;
    VibrationEffectType effecttype;
    PATestData sounddata;
    PaStreamParameters outputParameters;
    PaStream* stream;
}
SoundDevice;

int sounddev_engine_update(SimDevice* this, SimData* simdata);
int sounddev_gearshift_update(SimDevice* this, SimData* simdata);
int sounddev_free(SimDevice* this);

SoundDevice* new_sound_device(DeviceSettings* ds);

/***** Generic Methods *********/

int update(SimDevice* simdevice, SimData* simdata);

int devupdate(SimDevice* simdevice, SimData* simdata);

int devinit(SimDevice* simdevices, int numdevices, DeviceSettings* ds);

int devfree(SimDevice* simdevices, int numdevices);

int simdevfree(SimDevice* this);

#endif
