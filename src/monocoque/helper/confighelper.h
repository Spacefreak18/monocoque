#ifndef _CONFIGHELPER_H
#define _CONFIGHELPER_H

#include <pulse/channelmap.h>
#include <stdbool.h>
#include <stdint.h>

#include <libconfig.h>

#include "parameters.h"

typedef enum
{
    SIMDEV_UNKNOWN    = 0,
    SIMDEV_USB        = 1,
    SIMDEV_SOUND      = 2,
    SIMDEV_SERIAL     = 3
}
DeviceType;

typedef enum
{
    SIMDEVTYPE_UNKNOWN           = 0,
    SIMDEVTYPE_TACHOMETER        = 1,
    SIMDEVTYPE_USBHAPTIC         = 2,
    SIMDEVTYPE_SHIFTLIGHTS       = 3,
    SIMDEVTYPE_SIMWIND           = 4,
    SIMDEVTYPE_SERIALHAPTIC      = 5,
    SIMDEVTYPE_USBWHEEL          = 6
}
DeviceSubType;


typedef enum
{
    SIMULATOR_UPDATE_DEFAULT    = 0,
    SIMULATOR_UPDATE_RPMS       = 1,
    SIMULATOR_UPDATE_GEAR       = 2,
    SIMULATOR_UPDATE_PULSES     = 3,
    SIMULATOR_UPDATE_VELOCITY   = 4,
    SIMULATOR_UPDATE_ALTITUDE   = 5
}
SimulatorUpdate;

typedef enum
{
    EFFECT_ENGINERPM   = 0,
    EFFECT_GEARSHIFT   = 1,
    EFFECT_ABSBRAKES   = 2,
    EFFECT_TYRESLIP    = 3,
    EFFECT_TYRELOCK    = 4
}
VibrationEffectType;

typedef enum
{
    MOTOR_1       = 0,
    MOTOR_2       = 1,
    MOTOR_3       = 2,
    MOTOR_4       = 3,
    MOTOR_1_4     = 4,
    MOTOR_2_4     = 5,
    MOTOR_3_4     = 6,
    MOTOR_1_2     = 7,
    MOTOR_1_3     = 8,
    MOTOR_2_3     = 9,
    MOTOR_1_2_3_4 = 10,
    MOTOR_1_2_3   = 11,
    MOTOR_2_3_4   = 12,
    MOTOR_1_2_4   = 13,
    MOTOR_1_3_4   = 14
}
MotorPosition;

typedef enum
{
    MONOCOQUE_ERROR_NONE          = 0,
    MONOCOQUE_ERROR_UNKNOWN       = 1,
    MONOCOQUE_ERROR_INVALID_SIM   = 2,
    MONOCOQUE_ERROR_INVALID_DEV   = 3,
    MONOCOQUE_ERROR_NODATA        = 4,
    MONOCOQUE_ERROR_UNKNOWN_DEV   = 5
}
MonocoqueError;

typedef enum
{
    FRONTLEFT         = 0,
    FRONTRIGHT        = 1,
    REARLEFT          = 2,
    REARRIGHT         = 3,
    FRONTS            = 4,
    REARS             = 5,
    ALLFOUR           = 6
}
MonocoqueTyreIdentifier;

typedef struct
{
    ProgramAction program_action;
    Simulator sim_name;
    int configcheck;
    int useconfig;
    char* tyre_diameter_config;
}
MonocoqueSettings;

typedef struct
{
    int size;
    bool use_pulses;
    int granularity;
    uint32_t* rpms_array;
    uint32_t* pulses_array;
}
TachometerSettings;

typedef struct
{
    char* portdev;
    MotorPosition motorsposition;
    float ampfactor;
}
SerialDeviceSettings;

typedef struct
{
    int frequency;
    int volume;
    int lowbound_frequency;
    int upperbound_frequency;
    int pan;
    int channels;
    double duration;
    char* dev;
}
SoundDeviceSettings;

typedef struct
{
    int value0;
    int value1;
    char* dev;
}
USBDeviceSettings;

typedef struct
{
    bool is_valid;
    DeviceType dev_type;
    DeviceSubType dev_subtype;
    // to get really fancy move the effect information to it's own structure that would be a member of
    // any device settings member structure that can carry an effect
    VibrationEffectType effect_type;
    MonocoqueTyreIdentifier tyre;
    double threshold;
    // union?
    TachometerSettings tachsettings;
    SerialDeviceSettings serialdevsettings;
    SoundDeviceSettings sounddevsettings;
    USBDeviceSettings usbdevsettings;

}
DeviceSettings;

int strtogame(const char* game, MonocoqueSettings* ms);

int devsetup(const char* device_type, const char* device_subtype, const char* config_files, MonocoqueSettings* ms, DeviceSettings* ds, config_setting_t* device_settings);

int settingsfree(DeviceSettings ds);

int strcicmp(char const *a, char const *b);

#endif
