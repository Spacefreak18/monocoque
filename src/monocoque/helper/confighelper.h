#ifndef _CONFIGHELPER_H
#define _CONFIGHELPER_H

#include <pulse/channelmap.h>
#include <stdbool.h>
#include <stdint.h>

#include <libconfig.h>

#include "parameters.h"

#include "../devices/sounddevice.h"

typedef enum
{
    SIMDEV_USB        = 0,
    SIMDEV_SOUND      = 1,
    SIMDEV_SERIAL     = 2,
    SIMDEV_UNKNOWN    = 3,
}
DeviceType;

typedef enum
{
    SIMDEVTYPE_UNKNOWN           = 0,
    SIMDEVTYPE_SOUNDHAPTIC       = 1,
    SIMDEVTYPE_TACHOMETER        = 2,
    SIMDEVTYPE_USBHAPTIC         = 3,
    SIMDEVTYPE_USBWHEEL          = 4,
    SIMDEVTYPE_SHIFTLIGHTS       = 5,
    SIMDEVTYPE_SIMWIND           = 6,
    SIMDEVTYPE_SERIALHAPTIC      = 7,
    SIMDEVTYPE_SERIALWHEEL       = 8,
    SIMDEVTYPE_SIMLED            = 9,
    SIMDEVTYPE_ARDUINOCUSTOM     = 10,
}
DeviceSubType;

#define SerialDevicesOffset 5
#define USBDevicesOffset 2

typedef enum
{
    SIMDEVSUBTYPE_UNKNOWN                 = 0,
    SIMDEVSUBTYPE_CAMMUSC5                = 1,
    SIMDEVSUBTYPE_CAMMUSC12               = 2,
    SIMDEVSUBTYPE_MOZAR5                  = 3,
    SIMDEVSUBTYPE_CSLELITEV3PEDALS        = 4,
    SIMDEVSUBTYPE_SIMAGICP1000PEDALS      = 5,
    SIMDEVSUBTYPE_SIMAGICGTNEO            = 6,
    SIMDEVSUBTYPE_MOZA_NEW                = 7,
    SIMDEVSUBTYPE_LOGITECH_G29            = 8,
    SIMDEVSUBTYPE_MOZA_KS_PRO_WHEEL       = 9,
    SIMDEVSUBTYPE_SIMNETPEDALS            = 10,
    SIMDEVSUBTYPE_REVBURNERTACHOMETER     = 11
}
DeviceSubSubType;


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
    EFFECT_TYRELOCK    = 4,
    EFFECT_SUSPENSION  = 5
}
VibrationEffectType;

typedef enum
{
    EFFECT_MODULATION_NONE            = 0,
    EFFECT_MODULATION_FREQUENCY       = 1,
    EFFECT_MODULATION_AMPLIFY         = 2,
}
EffectModulationType;

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
    MONOCOQUE_GEAR_REVERSE = 0,
    MONOCOQUE_GEAR_NEUTRAL = 1,
    MONOCOQUE_GEAR_ONE     = 2,
    MONOCOQUE_GEAR_TWO     = 3,
    MONOCOQUE_GEAR_THREE   = 4,
    MONOCOQUE_GEAR_FOUR    = 5,
    MONOCOQUE_GEAR_FIVE    = 6,
    MONOCOQUE_GEAR_SIX     = 7,
    MONOCOQUE_GEAR_SEVEN   = 8,
    MONOCOQUE_GEAR_EIGHT   = 9,
}
MonocoqueGear;

typedef enum
{
    MONOCOQUE_ERROR_NONE                     = 0,
    MONOCOQUE_ERROR_UNKNOWN                  = 1,
    MONOCOQUE_ERROR_INVALID_SIM              = 2,
    MONOCOQUE_ERROR_INVALID_DEV              = 3,
    MONOCOQUE_ERROR_NODATA                   = 4,
    MONOCOQUE_ERROR_UNKNOWN_DEV              = 5,
    MONOCOQUE_ERROR_UNSUPPORTED_SIM_FEATURE  = 6,
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
    SimulatorAPI sim_name;
    int configcheck;
    int useconfig;
    int verbosity_count;
    int fps;
    bool  force_udp_mode;
    bool  disable_audio;
    char* tyre_diameter_config;
    char* config_str;
    char* log_filename_str;
    char* log_dirname_str;
    config_t* cfg;
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
    MotorPosition motorsposition;
    uint32_t numlights;
    uint32_t numleds;
    uint32_t startled;
    uint32_t endled;
    float ampfactor;
    float fanpower;
    // baud is the only serial thing
    uint32_t baud;
}
SerialDeviceSettings;

typedef struct
{
    uint32_t volume;
    uint32_t pan;
    uint32_t channels;
    uint32_t noise;
}
SoundDeviceSettings;

typedef struct
{
    uint32_t frequency;
    uint32_t amplitude;
    uint32_t frequencyMax;
    uint32_t amplitudeMax;

    double threshold;
    double duration;

    MotorPosition motorposition;
    VibrationEffectType effect_type;
    MonocoqueTyreIdentifier tyre;
    EffectModulationType modulation;
}
HapticEffectSettings;

typedef struct
{
    char* dev;
}
USBDeviceSettings;

typedef struct
{
    bool is_valid;

    DeviceType dev_type;
    DeviceSubType dev_subtype;
    DeviceSubSubType dev_subsubtype;
    bool has_haptic_effects;
    bool has_led_effects;

    char* dev;
    bool has_config;
    char* specific_config_file;
    uint32_t fps;
    // union?
    HapticEffectSettings hapticsettings;
    TachometerSettings tachsettings;
    SerialDeviceSettings serialdevsettings;
    SoundDeviceSettings sounddevsettings;
    USBDeviceSettings usbdevsettings;
}
DeviceSettings;

int strtogame(const char* game, MonocoqueSettings* ms);

int strtodevsubsubtype(const char* device_subsubtype, DeviceSettings* ds);

int devsetup(const char* device_type, const char* device_subtype, const char* config_files, MonocoqueSettings* ms, DeviceSettings* ds, config_setting_t* device_settings);

int settingsfree(DeviceSettings ds);

int monocoquesettingsfree(MonocoqueSettings* ms);

int strcicmp(char const *a, char const *b);

int getconfigtouse2(const char* config_file_str, char* car, int sim);
int getconfigtouse1(const char* config_file_str, char* car, int sim);
int getconfigtouse(const char* config_file_str, char* car, int sim);

config_t *open_monocoque_config(const char *filename);
void close_monocoque_config(config_t *cfg);

int delete_device_config(config_t *cfg, const char *configfile, int confignum, int devicenum);
int save_device_config(config_t *cfg, const char* configfile, int confignum, int devicenum, const DeviceSettings *ds);
int configcheck(const char* config_file_str, int confignum, int* devices);

int uiloadconfig(const char* config_file_str, int confignum, int configureddevices, MonocoqueSettings* ms, DeviceSettings* ds);

int getsingledevice(const char* config_file_str, int confignum, int devicenum, MonocoqueSettings* ms, DeviceSettings* ds);

int getNumberOfConfigs(const char* config_file_str);
#endif
