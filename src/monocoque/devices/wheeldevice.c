#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb/wheels/cammusc5.h"
#include "usb/wheels/cammusc12.h"
#include "serial/moza.h"

#include "simdevice.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../slog/slog.h"

int wheeldev_update(USBDevice* usbdevice, SimData* simdata)
{
    WheelDevice* wheeldevice = &usbdevice->u.wheeldevice;
    switch ( wheeldevice->type )
    {
        case WHEELDEV_UNKNOWN :
        case WHEELDEV_CAMMUSC5 :
            cammusc5_update(usbdevice, simdata->maxrpm, simdata->rpms, simdata->gear, simdata->velocity);
            break;
        case WHEELDEV_CAMMUSC12 :
            cammusc12_update(usbdevice, simdata->maxrpm, simdata->rpms, simdata->gear, simdata->velocity);
            break;

    }

    return 0;
}

int wheeldev_free(USBDevice* usbdevice)
{
    WheelDevice* wheeldevice = &usbdevice->u.wheeldevice;
    switch ( wheeldevice->type )
    {
        case WHEELDEV_UNKNOWN :
        case WHEELDEV_CAMMUSC5 :
            cammusc5_update(usbdevice, 0, 0, 0, 0);
            cammusc5_free(usbdevice);
            break;
        case WHEELDEV_CAMMUSC12 :
            cammusc12_update(usbdevice, 0, 0, 0, 0);
            cammusc12_free(usbdevice);
            break;
    }

    return 0;
}

int wheeldev_init(USBDevice* usbdevice, DeviceSettings* ds)
{
    slogi("initializing wheel device...");
    int error = 0;
    // detection of wheel model
    WheelDevice* wheeldevice = &usbdevice->u.wheeldevice;
    switch (ds->dev_subsubtype) {

        case SIMDEVSUBTYPE_CAMMUSC5:
            wheeldevice->type = WHEELDEV_CAMMUSC5;
            slogi("Attempting to initialize cammus C5");
            error = cammusc5_init(usbdevice);
            break;
        case SIMDEVSUBTYPE_CAMMUSC12:
            wheeldevice->type = WHEELDEV_CAMMUSC12;
            slogi("Attempting to initialize cammus C12");

            if(ds->has_config == true)
            {
                error = cammusc12_init(usbdevice, ds->specific_config_file);
            }
            else
            {
                error = cammusc12_init(usbdevice, NULL);
            }
            break;
        default:
            wheeldevice->type = WHEELDEV_UNKNOWN;
            slogw("Unknown cammus wheel detected, trying C5");
            error = cammusc5_init(usbdevice);
            break;
    }

    return error;
}
