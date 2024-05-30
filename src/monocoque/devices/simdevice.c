#include <stdio.h>
#include <stdlib.h>

#include "simdevice.h"
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../slog/slog.h"


int update(SimDevice* this, SimData* simdata)
{
    ((vtable*)this->vtable)->update(this, simdata);
}

int simdevfree(SimDevice* this)
{
    ((vtable*)this->vtable)->free(this);
}

int devupdate(SimDevice* this, SimData* simdata)
{

    return 0;
}

int devfree(SimDevice* simdevices, int numdevices)
{

    slogi("freeing %i simdevices...", numdevices);
    int devices = 0;

    int freed = 0;
    for (int j = 0; j < 1; j++)
    {
        SimDevice simdev = simdevices[j]; 
        if (simdev.initialized == true)
        {
            simdev.free(&simdev);
        }
    }

    //free(simdevices);

    return 0;
}

int devinit(SimDevice* simdevices, int numdevices, DeviceSettings* ds)
{
    slogi("initializing simdevices...");
    int devices = 0;

    for (int j = 0; j < numdevices; j++)
    {
        simdevices[j].initialized = false;

        if (ds[j].dev_type == SIMDEV_USB) {
            USBDevice* sim = new_usb_device(&ds[j]);
            if (sim != NULL)
            {
                simdevices[j] = sim->m;
                simdevices[j].initialized = true;
                simdevices[j].type = SIMDEV_USB;
                simdevices[j].tyre = ds[j].tyre;
                devices++;
            }
            else
            {
                slogw("Could not initialize USB Device");
            }
        }

        if (ds[j].dev_type == SIMDEV_SOUND) {

            SoundDevice* sim = new_sound_device(&ds[j]);
            if (sim != NULL)
            {

                simdevices[j] = sim->m;
                simdevices[j].initialized = true;
                simdevices[j].type = SIMDEV_SOUND;
                simdevices[j].tyre = ds[j].tyre;
                devices++;
            }
            else
            {
                slogw("Could not initialize Sound Device");
            }
        }

        if (ds[j].dev_type == SIMDEV_SERIAL) {

            SerialDevice* sim = new_serial_device(&ds[j]);
            if (sim != NULL)
            {
                simdevices[j] = sim->m;
                simdevices[j].initialized = true;
                simdevices[j].type = SIMDEV_SERIAL;
                devices++;

            }
            else
            {
                slogw("Could not initialize Serial Device");
            }
        }
    }

    return devices;
}
