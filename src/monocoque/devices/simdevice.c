#include <stdio.h>

#include "simdevice.h"
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simdata.h"
#include "../slog/slog.h"

int devupdate(SimDevice* simdevice, SimData* simdata)
{
    if (simdevice->initialized==false)
    {
        return 0;
    }
    switch ( simdevice->type )
    {
        case SIMDEV_USB :
            usbdev_update(&simdevice->d.usbdevice, simdata);
            break;
        case SIMDEV_SOUND :
            sounddev_update(&simdevice->d.sounddevice, simdata);
            break;
        case SIMDEV_SERIAL :
            serialdev_update(&simdevice->d.serialdevice, simdata);
            break;
    }
    return 0;
}

int devfree(SimDevice* simdevice)
{

    if (simdevice->initialized==false)
    {
        slogw("Attempt to free an uninitialized device");
        return MONOCOQUE_ERROR_INVALID_DEV;
    }
    switch ( simdevice->type )
    {
        case SIMDEV_USB :
            usbdev_free(&simdevice->d.usbdevice);
            break;
        case SIMDEV_SOUND :
            sounddev_free(&simdevice->d.sounddevice);
            break;
        case SIMDEV_SERIAL :
            serialdev_free(&simdevice->d.serialdevice);
            break;
    }
    return 0;
}

int devinit(SimDevice* simdevice, DeviceSettings* ds)
{
    slogi("initializing simdevice...");
    simdevice->initialized = false;
    int err = 0;

    switch ( ds->dev_type )
    {
        case SIMDEV_USB :
            simdevice->type = SIMDEV_USB;
            simdevice->d.usbdevice.type = USBDEV_UNKNOWN;
            err = usbdev_init(&simdevice->d.usbdevice, ds);
            break;
        case SIMDEV_SOUND :
            simdevice->type = SIMDEV_SOUND;
            err = sounddev_init(&simdevice->d.sounddevice);
            break;
        case SIMDEV_SERIAL :
            simdevice->type = SIMDEV_SERIAL;
            err = serialdev_init(&simdevice->d.serialdevice);
            break;
        default :
            sloge("Unknown device type");
            err = MONOCOQUE_ERROR_UNKNOWN_DEV;
            break;
    }

    if (err==0)
    {
        simdevice->initialized = true;
    }
    return err;
}
