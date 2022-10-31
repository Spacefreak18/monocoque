#ifndef _SIMMAPPER_H
#define _SIMMAPPEE_H

#include "ac.h"

#include "simdata.h"
#include "../helper/confighelper.h"

#include "simapi/acdata.h"

typedef struct
{
    void* addr;
    int fd;
    union
    {
        ACMap ac;
    } d;
}
SimMap;

int siminit(SimData* simdata, SimMap* simmap, Simulator simulator);
int simdatamap(SimData* simdata, SimMap* simmap, Simulator simulator);

#endif
