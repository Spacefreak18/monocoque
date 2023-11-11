#ifndef _SOUNDDEVICE_H
#define _SOUNDDEVICE_H

#ifdef USE_PULSEAUDIO
#include <pulse/pulseaudio.h>
#else
#include "portaudio.h"
#endif

typedef enum
{
    SOUNDDEV_UNKNOWN       = 0,
    SOUNDDEV_SHAKER        = 1
}
SoundType;

typedef enum
{
    SOUNDEFFECT_ENGINERPM   = 0,
    SOUNDEFFECT_GEARSHIFT   = 1
}
VibrationEffectType;

#define MAX_TABLE_SIZE   (6000)
typedef struct
{
    int last_gear;
    int volume;
    int frequency;
    double duration;
    int curr_frequency;
    double curr_duration;
}
SoundData;

#endif
