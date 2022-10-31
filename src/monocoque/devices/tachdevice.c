#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tachdevice.h"
#include "revburner.h"
#include "../../helper/confighelper.h"
#include "../../simulatorapi/simdata.h"
#include "../../slog/slog.h"

int tachdev_update(TachDevice* tachdevice, SimData* simdata)
{
    // current plan is to just use the revburner xml format for other possible tachometer devices
    // with that assumption this same logic is assumed the same for other tachometer devices
    // the only difference then being in communication to the physical device
    int pulses = simdata->pulses;
    switch ( tachdevice->type )
    {
        case TACHDEV_UNKNOWN :
        case TACHDEV_REVBURNER :

            if (tachdevice->tachsettings.use_pulses == false)
            {
                slogt("Getting pulses for current tachometer revs");
                if (simdata->rpms < 500)
                {
                    pulses = tachdevice->tachsettings.pulses_array[0];
                }
                else
                {
                    slogt("Tach settings size %i",tachdevice->tachsettings.size);
                    int el = simdata->rpms / 1000;
                    if (tachdevice->tachsettings.granularity > 0)
                    {
                        el = simdata->rpms / (1000 / tachdevice->tachsettings.granularity);
                    }
                    if (el >= tachdevice->tachsettings.size - 1)
                    {
                        el = tachdevice->tachsettings.size - 1;
                    }
                    slogt("Retrieveing element %i", el);
                    pulses = tachdevice->tachsettings.pulses_array[el];
                }
            }
            slogt("Settings tachometer pulses to %i", pulses);
            revburner_update(tachdevice, pulses);
            break;
    }

    return 0;
}

int tachdev_free(TachDevice* tachdevice)
{
    switch ( tachdevice->type )
    {
        case TACHDEV_UNKNOWN :
        case TACHDEV_REVBURNER :
            revburner_update(tachdevice, 0);
            revburner_free(tachdevice);
            break;
    }

    return 0;
}

int tachdev_init(TachDevice* tachdevice, DeviceSettings* ds)
{
    slogi("initializing tachometer device...");
    int error = 0;
    // detection of tach device model
    tachdevice->type = TACHDEV_UNKNOWN;
    tachdevice->type = TACHDEV_REVBURNER;

    tachdevice->tachsettings = ds->tachsettings;

    error = revburner_init(tachdevice);

    return error;
}
