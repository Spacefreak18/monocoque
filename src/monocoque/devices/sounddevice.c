#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <unistd.h>

#include "sound.h"
#include "simdevice.h"
#include "sounddevice.h"
#include "hapticeffect.h"
#include "sound/usb_generic_shaker.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../helper/parameters.h"
#include "../slog/slog.h"

int gear_sound_set(SoundDevice* sounddevice, SimData* simdata)
{
    if (sounddevice->sounddata.last_gear != simdata->gear && simdata->gear > 1)
    {
        //sounddevice->sounddata.gear_sound_data = 3.14;
        sounddevice->sounddata.curr_frequency = sounddevice->sounddata.frequency;
        sounddevice->sounddata.curr_duration = 0;
    }
    sounddevice->sounddata.last_gear = simdata->gear;

    slogt("set gear frequency to %i", sounddevice->sounddata.frequency);
}

// we could make a vtable for these different effects too
int sounddev_engine_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    sounddevice->sounddata.curr_frequency = simdata->rpms/60;
    //sounddevice->sounddata.table_size = 48000/(sounddevice->sounddata.frequency);

    slogt("set engine frequency to %i", sounddevice->sounddata.frequency);
}

int sounddev_tyreslip_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double play = slipeffect(simdata, this->hapticeffect.effecttype, this->hapticeffect.tyre, this->hapticeffect.threshold, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    slogt("Updating sound device frequency from original effect %f", play);

    if (play > 0)
    {
        sounddevice->sounddata.curr_frequency = sounddevice->sounddata.frequency * play;
        sounddevice->sounddata.curr_duration = sounddevice->sounddata.duration;
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0;
        sounddevice->sounddata.curr_duration = 0;
    }

}

int sounddev_tyrelock_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double play = slipeffect(simdata, this->hapticeffect.effecttype, this->hapticeffect.tyre, this->hapticeffect.threshold, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);

    if (play > 0)
    {
        sounddevice->sounddata.curr_frequency = sounddevice->sounddata.frequency * play;
        sounddevice->sounddata.curr_duration = sounddevice->sounddata.duration;
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0;
        sounddevice->sounddata.curr_duration = 0;
    }
}

int sounddev_absbrakes_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;
    double play = slipeffect(simdata, this->hapticeffect.effecttype, this->hapticeffect.tyre, this->hapticeffect.threshold, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);

    if (play > 0)
    {
        sounddevice->sounddata.curr_frequency = sounddevice->sounddata.frequency * play;
        sounddevice->sounddata.curr_duration = sounddevice->sounddata.duration;
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0;
        sounddevice->sounddata.curr_duration = 0;
    }
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

int sounddev_init(SoundDevice* sounddevice, const char* devname, int volume, int frequency, int pan, int channels, double duration, double threshold)
{
    slogi("initializing standalone sound device...");


    slogi("volume is: %i", volume);
    slogi("frequency is: %i", frequency);
    slogi("pan is: %i", pan);
    slogi("channels is: %i", channels);
    slogi("duration is: %f", duration);


    sounddevice->sounddata.frequency = frequency;
    //sounddevice->sounddata.pitch = 1;
    //sounddevice->sounddata.pitch = 261.626;
    //sounddevice->sounddata.amp = 32;
    //sounddevice->sounddata.left_phase = sounddevice->sounddata.right_phase = 0;
    //sounddevice->sounddata.table_size = 48000/(100/60);
    sounddevice->sounddata.curr_frequency = 0;
    sounddevice->sounddata.curr_duration = 0;



    const char* streamname= "Engine";
    switch (sounddevice->m.hapticeffect.effecttype) {
        case (EFFECT_GEARSHIFT):

            sounddevice->sounddata.last_gear = 0;
            //sounddevice->sounddata.pitch = 500;
            //sounddevice->sounddata.amp = 128;
            //sounddevice->sounddata.left_phase = sounddevice->sounddata.right_phase = 0;
            //sounddevice->sounddata.table_size = 48000/(1);
            sounddevice->sounddata.duration = duration;
            streamname = "Gear";
            break;
        case (EFFECT_TYRESLIP):
            sounddevice->sounddata.duration = duration;
            streamname = "TyreSlip";
            break;
        case (EFFECT_TYRELOCK):
            sounddevice->sounddata.duration = duration;
            streamname = "TyreLock";
            break;
        case (EFFECT_ABSBRAKES):
            sounddevice->sounddata.duration = duration;
            streamname = "ABS";
            break;
    }

#ifdef USE_PULSEAUDIO


    //pa_threaded_mainloop* mainloop;
    //pa_context* context;
    usb_generic_shaker_init(sounddevice, mainloop, context, devname, volume, pan, channels, streamname);
#else
    usb_generic_shaker_init(sounddevice);
#endif
}

static const vtable engine_sound_simdevice_vtable = { &sounddev_engine_update, &sounddev_free };
static const vtable gear_sound_simdevice_vtable = { &sounddev_gearshift_update, &sounddev_free };
static const vtable tyreslip_sound_simdevice_vtable = { &sounddev_tyreslip_update, &sounddev_free };
static const vtable tyrelock_sound_simdevice_vtable = { &sounddev_tyrelock_update, &sounddev_free };
static const vtable absbrakes_sound_simdevice_vtable = { &sounddev_absbrakes_update, &sounddev_free };

SoundDevice* new_sound_device(DeviceSettings* ds, MonocoqueSettings* ms) {

    SoundDevice* this = (SoundDevice*) malloc(sizeof(SoundDevice));

    this->m.update = &update;
    this->m.free = &simdevfree;
    this->m.derived = this;

    slogt("Attempting to configure sound device with subtype: %i", ds->effect_type);
    switch (ds->effect_type) {
        case (EFFECT_ENGINERPM):
            this->m.hapticeffect.effecttype = EFFECT_ENGINERPM;
            this->m.vtable = &engine_sound_simdevice_vtable;
            slogi("Initializing sound device for engine vibrations.");
            break;
        case (EFFECT_GEARSHIFT):
            this->m.hapticeffect.effecttype = EFFECT_GEARSHIFT;
            this->m.vtable = &gear_sound_simdevice_vtable;
            slogi("Initializing sound device for gear shift vibrations.");
            break;
        case (EFFECT_TYRESLIP):
            this->m.hapticeffect.effecttype = EFFECT_TYRESLIP;
            this->m.hapticeffect.threshold = ds->threshold;

            this->m.hapticeffect.threshold = ds->threshold;
            slogt("Haptic effect: %i %i", this->m.hapticeffect.effecttype, ds->effect_type);
            this->m.hapticeffect.tyre = ds->tyre;
            this->m.hapticeffect.useconfig = ms->useconfig;
            this->m.hapticeffect.configcheck = &ms->configcheck;
            this->m.hapticeffect.tyrediameterconfig = ms->tyre_diameter_config;

            this->m.vtable = &tyreslip_sound_simdevice_vtable;
            slogi("Initializing sound device for tyre slip vibrations.");
            break;
        case (EFFECT_TYRELOCK):
            this->m.hapticeffect.effecttype = EFFECT_TYRELOCK;
            this->m.hapticeffect.threshold = ds->threshold;

            this->m.hapticeffect.threshold = ds->threshold;
            slogt("Haptic effect: %i %i", this->m.hapticeffect.effecttype, ds->effect_type);
            this->m.hapticeffect.tyre = ds->tyre;
            this->m.hapticeffect.useconfig = ms->useconfig;
            this->m.hapticeffect.configcheck = &ms->configcheck;
            this->m.hapticeffect.tyrediameterconfig = ms->tyre_diameter_config;

            this->m.vtable = &tyrelock_sound_simdevice_vtable;
            slogi("Initializing sound device for tyre slip vibrations.");
            break;
        case (EFFECT_ABSBRAKES):
            this->m.hapticeffect.effecttype = EFFECT_ABSBRAKES;
            this->m.hapticeffect.threshold = ds->threshold;

            this->m.hapticeffect.threshold = ds->threshold;
            slogt("Haptic effect: %i %i", this->m.hapticeffect.effecttype, ds->effect_type);
            this->m.hapticeffect.tyre = ds->tyre;
            this->m.hapticeffect.useconfig = ms->useconfig;
            this->m.hapticeffect.configcheck = &ms->configcheck;
            this->m.hapticeffect.tyrediameterconfig = ms->tyre_diameter_config;

            this->m.vtable = &absbrakes_sound_simdevice_vtable;
            slogi("Initializing sound device for abs vibrations.");
            break;
    }

    slogt("Attempting to use sound device %s", ds->sounddevsettings.dev);

    int error = sounddev_init(this, ds->sounddevsettings.dev, ds->sounddevsettings.volume, ds->sounddevsettings.frequency, ds->sounddevsettings.pan, ds->sounddevsettings.channels, ds->sounddevsettings.duration, ds->threshold);
    if (error != 0)
    {
        free(this);
        return NULL;
    }

    return this;
}

