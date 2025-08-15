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

int devinit(SimDevice* simdevices, int numdevices, DeviceSettings* ds, MonocoqueSettings* ms)
{
    slogi("initializing simdevices...");
    int devices = 0;

    for (int j = 0; j < numdevices; j++)
    {
        simdevices[j].initialized = false;

        if (ds[j].dev_type == SIMDEV_USB) {
            USBDevice* sim = new_usb_device(&ds[j], ms);
            if (sim != NULL)
            {
                simdevices[j] = sim->m;
                simdevices[j].initialized = true;
                simdevices[j].type = SIMDEV_USB;
                simdevices[j].fps = ds[j].fps;
                devices++;
            }
            else
            {
                slogw("Could not initialize USB Device");
            }
        }

        if (ds[j].dev_type == SIMDEV_SOUND) {
            if(ms->disable_audio == true)
            {
                slogi("skipping configured sound device due to disable_audio being specified...");
            }
            else
            {
                SoundDevice* sim = new_sound_device(&ds[j], ms);
                if (sim != NULL)
                {

                    simdevices[j] = sim->m;
                    simdevices[j].initialized = true;
                    simdevices[j].type = SIMDEV_SOUND;
                    simdevices[j].fps = ds[j].fps;
                    devices++;
                }
                else
                {
                    slogw("Could not initialize Sound Device");
                }
            }
        }

        if (ds[j].dev_type == SIMDEV_SERIAL) {

            SerialDevice* sim = new_serial_device(&ds[j], ms);
            if (sim != NULL)
            {
                simdevices[j] = sim->m;
                simdevices[j].initialized = true;
                simdevices[j].type = SIMDEV_SERIAL;
                simdevices[j].fps = ds[j].fps;
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
