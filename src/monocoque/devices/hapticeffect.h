#ifndef _HAPTICEFFECT_H
#define _HAPTICEFFECT_H

#include <stdio.h>
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"

typedef struct
{
    double threshold;
    VibrationEffectType effecttype;
}
HapticEffect;

int slipeffect(SimData* simdata, int effecttype, int tyre, double threshold, int useconfig, int* configcheck, char* configfile);

#endif
