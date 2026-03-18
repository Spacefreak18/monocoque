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
    // these are all pointers to pointers and freed independently, usually in releaseloop
    uv_work_t req;
    SimData* simdata;
    SimDevice* simdevice;
    MonocoqueSettings* ms;
    SimInfo siminfo;
} device_loop_data;

typedef struct loop_data
{
    uv_work_t req;
    bool use_udp;
    bool uion;
    bool releasing;
    bool started_tyre_calc_thread;
    int numdevices;
    SimInfo siminfo;
    // monocoque settings is a pointer from monocoque.c and freed there
    MonocoqueSettings* ms;
    // these are freed at the end of the main gameloop
    SimData* simdata;
    SimMap* simmap;
    // these all get malloced in looprun and freed in releaseloop
    SimDevice* simdevices;
    uv_timer_t* device_timers;
    device_loop_data* device_batons;
    device_loop_data* tyrebaton;
} loop_data;



#endif
