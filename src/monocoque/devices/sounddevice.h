#ifndef _SOUNDDEVICE_H
#define _SOUNDDEVICE_H

#include "portaudio.h"

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
    float sine[MAX_TABLE_SIZE];
    float pitch;
    int last_gear;
    int left_phase;
    int right_phase;
    int n;
    int table_size;
    int amp;
    int gear_sound_data;
}
PATestData;

#endif
