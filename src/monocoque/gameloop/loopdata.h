#ifndef _LOOPDATA_H
#define _LOOPDATA_H

#include <uv.h>
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../devices/simdevice.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"


typedef struct loop_data
{
    uv_work_t req;
    SimulatorAPI sim;
    SimulatorAPI map;
    bool use_udp;
    bool simstate;
    bool uion;
    bool releasing;
    int numdevices;
    MonocoqueSettings* ms;
    SimData* simdata;
    SimMap* simmap;
    SimDevice* simdevices;
} loop_data;

#endif
