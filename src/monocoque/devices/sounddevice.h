#ifndef _SOUNDDEVICE_H
#define _SOUNDDEVICE_H

//#ifdef USE_PULSEAUDIO
#include <pulse/pulseaudio.h>
//#else
//#include "portaudio.h"
//#endif


#define MAX_TABLE_SIZE   (6000)
typedef struct
{
    uint8_t last_gear;
    int volume;
    double duration;
    double curr_frequency;
    uint32_t curr_amplitude;
    double curr_duration;
    double phase;
    double noise;
}
SoundData;

#endif
