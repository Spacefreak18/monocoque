#ifndef _SOUNDDEVICE_H
#define _SOUNDDEVICE_H

#include "portaudio.h"

#include "../simulatorapi/simdata.h"
#include "../helper/parameters.h"

typedef enum
{
    SOUNDDEV_UNKNOWN       = 0,
    SOUNDDEV_SHAKER        = 1
}
SoundType;

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

typedef struct
{
    int id;
    SoundType type;
    PATestData sounddata;
    PaStreamParameters outputParameters;
    PaStream* stream;
}
SoundDevice;

int sounddev_update(SoundDevice* sounddevice, SimData* simdata);
int sounddev_init(SoundDevice* sounddevice);
int sounddev_free(SoundDevice* sounddevice);

#endif
