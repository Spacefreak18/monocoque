#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <unistd.h>

#include "simdevice.h"
#include "sounddevice.h"
#include "sound/usb_generic_shaker.h"
#include "../simulatorapi/simdata.h"
#include "../helper/parameters.h"
#include "../slog/slog.h"

int gear_sound_set(SoundDevice* sounddevice, SimData* simdata)
{
    if (sounddevice->sounddata.last_gear != simdata->gear && simdata->gear != 0)
    {
        sounddevice->sounddata.gear_sound_data = 3.14;
    }
    sounddevice->sounddata.last_gear = simdata->gear;
}

// we could make a vtable for these different effects too
int sounddev_engine_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    gear_sound_set(sounddevice, simdata);

    sounddevice->sounddata.table_size = 44100/(simdata->rpms/60);
}

int sounddev_gearshift_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    gear_sound_set(sounddevice, simdata);
}

int sounddev_free(SimDevice* this)
{
    SoundDevice* sounddevice = (void *) this->derived;

    usb_generic_shaker_free(sounddevice);

    free(sounddevice);

    return 0;
}

int sounddev_init(SoundDevice* sounddevice)
{
    slogi("initializing standalone sound device...");

    sounddevice->sounddata.pitch = 1;
    sounddevice->sounddata.pitch = 261.626;
    sounddevice->sounddata.amp = 32;
    sounddevice->sounddata.left_phase = sounddevice->sounddata.right_phase = 0;
    sounddevice->sounddata.table_size = 44100/(100/60);
    sounddevice->sounddata.last_gear = 0;

    switch (sounddevice->effecttype) {
        case (SOUNDEFFECT_GEARSHIFT):

            sounddevice->sounddata.pitch = 500;
            sounddevice->sounddata.amp = 128;
            sounddevice->sounddata.left_phase = sounddevice->sounddata.right_phase = 0;
            sounddevice->sounddata.table_size = 44100/(1);
            break;
    }

    usb_generic_shaker_init(sounddevice);
}

static const vtable engine_sound_simdevice_vtable = { &sounddev_engine_update, &sounddev_free };
static const vtable gear_sound_simdevice_vtable = { &sounddev_gearshift_update, &sounddev_free };

SoundDevice* new_sound_device(DeviceSettings* ds) {

    SoundDevice* this = (SoundDevice*) malloc(sizeof(SoundDevice));

    this->m.update = &update;
    this->m.free = &simdevfree;
    this->m.derived = this;

    slogt("Attempting to configure sound device with subtype: %i", ds->dev_subtype);
    switch (ds->dev_subtype) {
        case (SIMDEVTYPE_ENGINESOUND):
            this->effecttype = SOUNDEFFECT_ENGINERPM;
            this->m.vtable = &engine_sound_simdevice_vtable;
            slogi("Initializing sound device for engine vibrations.");
            break;
        case (SIMDEVTYPE_GEARSOUND):
            this->effecttype = SOUNDEFFECT_GEARSHIFT;
            this->m.vtable = &gear_sound_simdevice_vtable;
            slogi("Initializing sound device for gear shift vibrations.");
            break;
    }

    int error = sounddev_init(this);
    if (error != 0)
    {
        free(this);
        return NULL;
    }

    return this;
}

