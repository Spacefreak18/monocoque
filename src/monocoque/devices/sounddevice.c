#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <unistd.h>
#include <math.h>

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
        sounddevice->sounddata.curr_amplitude = sounddevice->sounddata.amplitude;
        sounddevice->sounddata.curr_duration = 0;
    }
    sounddevice->sounddata.last_gear = simdata->gear;

    slogt("set gear frequency to %i", sounddevice->sounddata.curr_frequency);
}


double modulate(SoundDevice* sounddevice, double raw_effect, SoundEffectModulationType modulation)
{
    double modulated_effect = raw_effect;
    switch (modulation)
    {
        case SOUND_EFFECT_MODULATION_FREQUENCY:
            modulated_effect = ((sounddevice->sounddata.frequencyMax - sounddevice->sounddata.frequency) * raw_effect) + sounddevice->sounddata.frequency;
            sounddevice->sounddata.curr_frequency = trunc(modulated_effect);
            sounddevice->sounddata.curr_amplitude = sounddevice->sounddata.amplitude;
            slogt("set curr frequency to %i from raw effect %f and base frequency %i", sounddevice->sounddata.curr_frequency, raw_effect, sounddevice->sounddata.frequency);
            break;
        case SOUND_EFFECT_MODULATION_AMPLIFY:
            modulated_effect = ((sounddevice->sounddata.amplitudeMax - sounddevice->sounddata.amplitude) * raw_effect) + sounddevice->sounddata.amplitude;
            sounddevice->sounddata.curr_amplitude = modulated_effect;
            sounddevice->sounddata.curr_frequency = sounddevice->sounddata.frequency;
            slogt("set curr amplitude to %i from raw effect %f and base amplitude %i", sounddevice->sounddata.curr_amplitude, raw_effect, sounddevice->sounddata.amplitude);
            break;
        case SOUND_EFFECT_MODULATION_NONE:
        default:
            sounddevice->sounddata.curr_frequency = sounddevice->sounddata.frequency;
            sounddevice->sounddata.curr_amplitude = sounddevice->sounddata.amplitude;
            break;
    }

    return modulated_effect;
}

// we could make a vtable for these different effects too
int sounddev_engine_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double effect = ((double)simdata->rpms/60)/((double)simdata->maxrpm/60);
    slogt("Set base effect of %f from rpms of %i", effect, simdata->rpms);
    modulate(sounddevice, effect, sounddevice->modulationType);
}

int sounddev_tyreslip_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double effect = slipeffect(simdata, this->hapticeffect.effecttype, this->hapticeffect.tyre, this->hapticeffect.threshold, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    slogt("Updating sound device frequency from original tyre slip effect %f", effect);

    if (effect > 0)
    {
        modulate(sounddevice, effect, sounddevice->modulationType);
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0;
        sounddevice->sounddata.curr_amplitude = 0;
        sounddevice->sounddata.curr_duration = 0;
    }

}

int sounddev_tyrelock_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double play = slipeffect(simdata, this->hapticeffect.effecttype, this->hapticeffect.tyre, this->hapticeffect.threshold, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    slogt("Updating sound device frequency from original tyre lock effect %f", play);

    if (play > 0)
    {
        modulate(sounddevice, play, sounddevice->modulationType);
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0;
        sounddevice->sounddata.curr_amplitude = 0;
        sounddevice->sounddata.curr_duration = 0;
    }
}

int sounddev_absbrakes_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;
    double play = slipeffect(simdata, this->hapticeffect.effecttype, this->hapticeffect.tyre, this->hapticeffect.threshold, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    slogt("Updating sound device frequency from original abs effect %f", play);

    if (play > 0)
    {
        modulate(sounddevice, play, sounddevice->modulationType);
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0;
        sounddevice->sounddata.curr_amplitude = 0;
        sounddevice->sounddata.curr_duration = 0;
    }
}

int sounddev_suspension_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double effect = slipeffect(simdata, this->hapticeffect.effecttype, this->hapticeffect.tyre, this->hapticeffect.threshold, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    effect = effect * 10;
    slogt("Updating sound device output from original suspension travel effect %f", effect);

    if (effect > 0)
    {
        effect = modulate(sounddevice, effect, sounddevice->modulationType);
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0;
        sounddevice->sounddata.curr_amplitude = 0;
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

    usb_generic_shaker_free(sounddevice, mainloop);
    free(sounddevice);

    return 0;
}

int sounddev_init(SoundDevice* sounddevice, const char* devname, MonocoqueTyreIdentifier tyre, SoundDeviceSettings sds)
{
    slogi("initializing standalone sound device...");


    slogi("volume is: %i", sds.volume);
    slogi("Modulation type: %i", sds.modulation);
    slogi("frequency is: %i", sds.frequency);
    slogi("frequency Max is: %i", sds.frequencyMax);
    slogi("amplitude is: %i", sds.amplitude);
    slogi("amplitude Max is: %i", sds.amplitudeMax);
    slogi("pan is: %i", sds.pan);
    slogi("channels is: %i", sds.channels);
    slogi("duration is: %f", sds.duration);


    sounddevice->modulationType = sds.modulation;
    sounddevice->sounddata.frequency = sds.frequency;
    sounddevice->sounddata.frequencyMax = sds.frequencyMax;
    sounddevice->sounddata.amplitude = sds.amplitude;
    sounddevice->sounddata.amplitudeMax = sds.amplitudeMax;
    sounddevice->sounddata.curr_duration = 0;

    sounddevice->sounddata.phase = 0;

    sounddevice->sounddata.curr_amplitude = 0;
    sounddevice->sounddata.curr_frequency = 0;


    const char* streamname= "Engine";
    switch (sounddevice->m.hapticeffect.effecttype) {
        case (EFFECT_GEARSHIFT):
            sounddevice->sounddata.last_gear = 0;
            sounddevice->sounddata.duration = sds.duration;
            streamname = "Gear";
            break;
        case (EFFECT_TYRESLIP):
            streamname = "TyreSlip";
            break;
        case (EFFECT_TYRELOCK):
            streamname = "TyreLock";
            break;
        case (EFFECT_ABSBRAKES):
            streamname = "ABS";
            break;
        case (EFFECT_SUSPENSION):
            streamname = "Suspension";
            break;
        case (EFFECT_ENGINERPM):
        default:
            streamname = "Engine";
            break;
    }


    usb_generic_shaker_init(sounddevice, mainloop, context, devname, sds.volume, sds.pan, sds.channels, streamname);
    //usb_generic_shaker_init(sounddevice);
}

static const vtable engine_sound_simdevice_vtable = { &sounddev_engine_update, &sounddev_free };
static const vtable gear_sound_simdevice_vtable = { &sounddev_gearshift_update, &sounddev_free };
static const vtable tyreslip_sound_simdevice_vtable = { &sounddev_tyreslip_update, &sounddev_free };
static const vtable tyrelock_sound_simdevice_vtable = { &sounddev_tyrelock_update, &sounddev_free };
static const vtable absbrakes_sound_simdevice_vtable = { &sounddev_absbrakes_update, &sounddev_free };
static const vtable suspension_sound_simdevice_vtable = { &sounddev_suspension_update, &sounddev_free };

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

        case (EFFECT_SUSPENSION):
            this->m.hapticeffect.effecttype = EFFECT_SUSPENSION;
            this->m.hapticeffect.threshold = ds->threshold;

            this->m.hapticeffect.threshold = ds->threshold;
            slogt("Haptic effect: %i %i on tyre %i", this->m.hapticeffect.effecttype, ds->effect_type, ds->tyre);
            this->m.hapticeffect.tyre = ds->tyre;
            this->m.hapticeffect.useconfig = ms->useconfig;
            this->m.hapticeffect.configcheck = &ms->configcheck;
            this->m.hapticeffect.tyrediameterconfig = ms->tyre_diameter_config;

            this->m.vtable = &suspension_sound_simdevice_vtable;
            slogi("Initializing sound device for suspension vibrations.");
            break;
    }

    slogt("Attempting to use sound device %s", ds->sounddevsettings.dev);

    int error = sounddev_init(this, ds->sounddevsettings.dev, ds->tyre, ds->sounddevsettings);
    if (error != 0)
    {
        free(this);
        return NULL;
    }

    return this;
}
