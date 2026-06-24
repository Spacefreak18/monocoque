#ifndef _HAPTICEFFECT_H
#define _HAPTICEFFECT_H

#include <stdio.h>
#include "../simulatorapi/simapi/simapi/simdata.h"


typedef enum
{
    HAPTIC_EFFECT_MODULATION_NONE            = 0,
    HAPTIC_EFFECT_MODULATION_FREQUENCY       = 1,
    HAPTIC_EFFECT_MODULATION_AMPLIFY         = 2,
}
HapticEffectModulationType;


typedef struct
{
    uint32_t curr_frequency;
    uint32_t curr_amplitude;
    double curr_duration;
    uint32_t last_gear;
}
CurrentEffectData;

typedef struct
{
    VibrationEffectType effecttype;
    MonocoqueTyreIdentifier tyre;
    HapticEffectModulationType modulationType;

    uint32_t volume;
    double duration;
    double threshold;
    uint32_t motorposition;
    uint32_t basefrequency;
    uint32_t frequencyMax;
    uint32_t baseamplitude;
    uint32_t amplitudeMax;
   
    CurrentEffectData live_effect;

    int useconfig;
    int* configcheck;
    char* tyrediameterconfig;
}
HapticEffect;

int initializeHapticEffect(HapticEffect* h, HapticEffectSettings* hs, MonocoqueSettings* ms);
double slipeffect(SimData* simdata, HapticEffect* h, int useconfig, int* configcheck, char* configfile);
bool hasTyreDiameter(SimData* simdata);
int loadtyreconfig(SimData* simdata, char* configfile, bool setDiameters);
int savetyreconfig(SimData* simdata, char* configfile);
void getTyreDiameter(SimData* simdata);

#endif
