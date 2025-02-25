#ifndef _SIMDEVICE_H
#define _SIMDEVICE_H

#include <stdbool.h>

#include "lua5.4/lua.h"

#include "usbdevice.h"
#include "sounddevice.h"
#include "serialdevice.h"
#include "hapticeffect.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"

#include "../../arduino/simwind/simwind.h"
#include "../../arduino/simhaptic/simhaptic.h"
#include "../../arduino/shiftlights/shiftlights.h"

typedef struct SimDevice SimDevice;

struct SimDevice
{
    const void* vtable;
    int (*update)(SimDevice*, SimData*);
    int (*free)(SimDevice*);
    void* derived;
    int id;
    int fps;
    bool initialized;
    DeviceType type;
    HapticEffect hapticeffect;
};

typedef struct {
    int (*update)(SimDevice*, SimData*);
    int (*free)(SimDevice*);
} vtable;

/********* Serial Devices *****/
typedef enum
{
    SERIALDEV_UNKNOWN       = 0,
    SERIALDEV_ARDUINO       = 1,
    SERIALDEV_WHEEL         = 2,
}
SerialType;

typedef struct
{
    SimDevice m;
    int id;
    SerialType type;
    struct sp_port* port;
    lua_State* L;
    SerialDeviceType devicetype;
    // move these two they only apply to the haptic device
    int motorsposition;
    int numlights;
    int numleds;
    int startled;
    int endled;
    int baudrate;
    double ampfactor;
    double state;
    union
    {
        SimWindData simwinddata;
        SimHapticData simhapticdata;
        ShiftLightsData shiftlightsdata;
        WheelDevice wheeldevice;
    } u;
}
SerialDevice;

int arduino_shiftlights_update(SimDevice* this, SimData* simdata);
int arduino_simwind_update(SimDevice* this, SimData* simdata);
int arduino_simhaptic_update(SimDevice* this, SimData* simdata);
int serialdev_free(SimDevice* this);

SerialDevice* new_serial_device(DeviceSettings* ds, MonocoqueSettings* ms);

/********* USB HID Devices *****/
typedef enum
{
    USBDEV_UNKNOWN       = 0,
    USBDEV_TACHOMETER    = 1,
    USBDEV_GENERICHAPTIC = 2,
    USBDEV_WHEEL         = 3
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
        WheelDevice wheeldevice;
        USBGenericHapticDevice hapticdevice;
    } u;
}
USBDevice;

int usbdev_update(SimDevice* this, SimData* simdata);
int usbdev_free(SimDevice* this);

USBDevice* new_usb_device(DeviceSettings* ds, MonocoqueSettings* ms);


/********* Sound Devices *****/
typedef struct
{
    SimDevice m;
    int id;
    int configcheck;
    SoundEffectModulationType modulationType;
    SoundData sounddata;
#ifdef USE_PULSEAUDIO
    pa_stream *stream;
#else
    PaStreamParameters outputParameters;
    PaStream* stream;
#endif
}
SoundDevice;

int sounddev_engine_update(SimDevice* this, SimData* simdata);
int sounddev_gearshift_update(SimDevice* this, SimData* simdata);
int sounddev_tyreslip_update(SimDevice* this, SimData* simdata);
int sounddev_free(SimDevice* this);

SoundDevice* new_sound_device(DeviceSettings* ds, MonocoqueSettings* ms);

/***** Generic Methods *********/

int update(SimDevice* simdevice, SimData* simdata);

int devupdate(SimDevice* simdevice, SimData* simdata);

int devinit(SimDevice* simdevices, int numdevices, DeviceSettings* ds, MonocoqueSettings* ms);

int devfree(SimDevice* simdevices, int numdevices);

int simdevfree(SimDevice* this);

#endif
