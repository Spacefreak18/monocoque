#ifndef _HAPTICEFFECT_H
#define _HAPTICEFFECT_H

#include <stdio.h>
#include "../simulatorapi/simapi/simapi/simdata.h"

int slipeffect(SimData* simdata, int effecttype, int tyre, double threshold, int useconfig, int* configcheck, char* configfile);

#endif
