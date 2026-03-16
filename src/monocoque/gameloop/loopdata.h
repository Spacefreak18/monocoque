#ifndef _LOOPDATA_H
#define _LOOPDATA_H

#include <uv.h>
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../devices/simdevice.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"

typedef struct device_loop_data
{
    uv_work_t req;
    SimData* simdata;
    SimDevice* simdevice;
    SimInfo siminfo;
} device_loop_data;

typedef struct loop_data
{
    uv_work_t req;
    bool use_udp;
    bool uion;
    bool releasing;
    int numdevices;
    SimInfo siminfo;
    MonocoqueSettings* ms;
    SimData* simdata;
    SimMap* simmap;
    SimDevice* simdevices;
    uv_timer_t* device_timers;
    device_loop_data* device_batons;
} loop_data;



#endif
