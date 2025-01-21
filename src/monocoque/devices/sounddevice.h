#ifndef _SOUNDDEVICE_H
#define _SOUNDDEVICE_H

#ifdef USE_PULSEAUDIO
#include <pulse/pulseaudio.h>
#else
#include "portaudio.h"
#endif


typedef enum
{
    SOUND_EFFECT_MODULATION_NONE            = 0,
    SOUND_EFFECT_MODULATION_FREQUENCY       = 1,
    SOUND_EFFECT_MODULATION_AMPLIFY         = 2,
}
SoundEffectModulationType;

#define MAX_TABLE_SIZE   (6000)
typedef struct
{
    int last_gear;
    int volume;
    uint32_t frequency;
    uint32_t frequencyMax;
    double duration;
    int curr_frequency;
    double curr_duration;
    double phase;
}
SoundData;

#endif
