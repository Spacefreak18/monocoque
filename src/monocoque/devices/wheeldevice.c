#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wheels/cammusc5.h"
#include "wheeldevice.h"
#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
#include "../../slog/slog.h"

int wheeldev_update(WheelDevice* wheeldevice, SimData* simdata)
{
    switch ( wheeldevice->type )
    {
        case WHEELDEV_UNKNOWN :
        case WHEELDEV_CAMMUSC5 :
            cammusc5_update(wheeldevice, simdata->maxrpm, simdata->rpms, simdata->gear, simdata->velocity);
            break;

    }

    return 0;
}

int wheeldev_free(WheelDevice* wheeldevice)
{
    switch ( wheeldevice->type )
    {
        case WHEELDEV_UNKNOWN :
        case WHEELDEV_CAMMUSC5 :
            cammusc5_update(wheeldevice, 0, 0, 0, 0);
            cammusc5_free(wheeldevice);
            break;
    }

    return 0;
}

int wheeldev_init(WheelDevice* wheeldevice, DeviceSettings* ds)
{
    slogi("initializing wheel device...");
    int error = 0;
    // detection of wheel model
    wheeldevice->type = WHEELDEV_UNKNOWN;
    wheeldevice->type = WHEELDEV_CAMMUSC5;


    error = cammusc5_init(wheeldevice);

    return error;
}
