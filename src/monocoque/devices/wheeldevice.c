#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb/wheels/cammusc5.h"
#include "usb/wheels/cammusc12.h"
#include "serial/moza.h"
#include "wheeldevice.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../slog/slog.h"

int wheeldev_update(WheelDevice* wheeldevice, SimData* simdata)
{
    switch ( wheeldevice->type )
    {
        case WHEELDEV_UNKNOWN :
        case WHEELDEV_CAMMUSC5 :
            cammusc5_update(wheeldevice, simdata->maxrpm, simdata->rpms, simdata->gear, simdata->velocity);
            break;
        case WHEELDEV_CAMMUSC12 :
            cammusc12_update(wheeldevice, simdata->maxrpm, simdata->rpms, simdata->gear, simdata->velocity);
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
        case WHEELDEV_CAMMUSC12 :
            cammusc12_update(wheeldevice, 0, 0, 0, 0);
            cammusc12_free(wheeldevice);
            break;
    }

    return 0;
}

int wheeldev_init(WheelDevice* wheeldevice, DeviceSettings* ds)
{
    slogi("initializing wheel device...");
    int error = 0;
    // detection of wheel model

    switch (ds->dev_subsubtype) {

        case SIMDEVSUBTYPE_CAMMUSC5:
            wheeldevice->type = WHEELDEV_CAMMUSC5;
            slogi("Attempting to initialize cammus C5");
            error = cammusc5_init(wheeldevice);
            break;
        case SIMDEVSUBTYPE_CAMMUSC12:
            wheeldevice->type = WHEELDEV_CAMMUSC12;
            slogi("Attempting to initialize cammus C12");
            error = cammusc12_init(wheeldevice);
            break;
        default:
            wheeldevice->type = WHEELDEV_UNKNOWN;
            slogw("Unknown cammus wheel detected, trying C5");
            error = cammusc5_init(wheeldevice);
            break;
    }

    return error;
}
