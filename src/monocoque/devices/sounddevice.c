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

int gear_sound_set(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    if (sounddevice->sounddata.last_gear != simdata->gear && simdata->gear > 1)
    {
        //sounddevice->sounddata.gear_sound_data = 3.14;
        sounddevice->sounddata.curr_frequency = this->hapticeffect.basefrequency;
        sounddevice->sounddata.curr_amplitude = this->hapticeffect.baseamplitude;
        sounddevice->sounddata.curr_duration = 0;
    }
    sounddevice->sounddata.last_gear = simdata->gear;

    slogt("set gear frequency to %f", sounddevice->sounddata.curr_frequency);
}


double modulate(SimDevice* this, double raw_effect, HapticEffectModulationType modulation)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double modulated_effect = raw_effect;
    switch (modulation)
    {
        case HAPTIC_EFFECT_MODULATION_FREQUENCY:
            modulated_effect = ((this->hapticeffect.frequencyMax - this->hapticeffect.basefrequency) * raw_effect) + this->hapticeffect.basefrequency;
            sounddevice->sounddata.curr_frequency = modulated_effect;
            sounddevice->sounddata.curr_amplitude = this->hapticeffect.baseamplitude;
            slogt("set curr frequency to %f from raw effect %f and base frequency %i", sounddevice->sounddata.curr_frequency, raw_effect, this->hapticeffect.basefrequency);
            break;
        case HAPTIC_EFFECT_MODULATION_AMPLIFY:
            modulated_effect = ((this->hapticeffect.amplitudeMax - this->hapticeffect.baseamplitude) * raw_effect) + this->hapticeffect.baseamplitude;
            sounddevice->sounddata.curr_amplitude = modulated_effect;
            sounddevice->sounddata.curr_frequency = this->hapticeffect.basefrequency;
            slogt("set curr amplitude to %i from raw effect %f and base amplitude %i", sounddevice->sounddata.curr_amplitude, raw_effect, this->hapticeffect.baseamplitude);
            break;
        case HAPTIC_EFFECT_MODULATION_NONE:
        default:
            sounddevice->sounddata.curr_frequency = this->hapticeffect.basefrequency;
            sounddevice->sounddata.curr_amplitude = this->hapticeffect.baseamplitude;
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
    modulate(this, effect, this->hapticeffect.modulationType);
}

int sounddev_tyreslip_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double effect = slipeffect(simdata, &this->hapticeffect, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    slogt("Updating sound device frequency from original %i tyre slip effect %f", this->hapticeffect.effecttype, effect);

    if (effect > 0)
    {
        modulate(this, effect, this->hapticeffect.modulationType);
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0.0;
        sounddevice->sounddata.curr_amplitude = 0;
        sounddevice->sounddata.curr_duration = 0;
    }

}

int sounddev_tyrelock_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double play = slipeffect(simdata, &this->hapticeffect, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    slogt("Updating sound device frequency from original tyre lock effect %f", play);

    if (play > 0)
    {
        modulate(this, play, this->hapticeffect.modulationType);
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0.0;
        sounddevice->sounddata.curr_amplitude = 0;
        sounddevice->sounddata.curr_duration = 0;
    }
}

int sounddev_absbrakes_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;
    double play = slipeffect(simdata, &this->hapticeffect, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    slogt("Updating sound device frequency from original abs effect %f", play);

    if (play > 0)
    {
        modulate(this, play, this->hapticeffect.modulationType);
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0.0;
        sounddevice->sounddata.curr_amplitude = 0;
        sounddevice->sounddata.curr_duration = 0;
    }
}

int sounddev_suspension_update(SimDevice* this, SimData* simdata)
{
    SoundDevice* sounddevice = (void *) this->derived;

    double effect = slipeffect(simdata, &this->hapticeffect, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
    effect = effect * 10;
    slogt("Updating sound device output from original suspension travel effect %f", effect);

    if (effect > 0)
    {
        effect = modulate(this, effect, this->hapticeffect.modulationType);
    }
    else
    {
        sounddevice->sounddata.curr_frequency = 0.0;
        sounddevice->sounddata.curr_amplitude = 0;
        sounddevice->sounddata.curr_duration = 0;
    }

}

int sounddev_gearshift_update(SimDevice* this, SimData* simdata)
{
    gear_sound_set(this, simdata);
}


int sounddev_free(SimDevice* this)
{
    SoundDevice* sounddevice = (void *) this->derived;

    usb_generic_shaker_free(sounddevice, mainloop);
    free(sounddevice);

    return 0;
}

int sounddev_init(SoundDevice* sounddevice, const char* devname, SoundDeviceSettings sds)
{
    slogi("initializing standalone sound device...");


    slogi("volume is: %i", sds.volume);
    slogi("pan is: %i", sds.pan);
    slogi("channels is: %i", sds.channels);
    slogi("noise is: %i", sds.noise);


    sounddevice->sounddata.noise = (double)sds.noise;
    sounddevice->sounddata.curr_duration = 0;

    sounddevice->sounddata.phase = 0;

    sounddevice->sounddata.curr_amplitude = 0;
    sounddevice->sounddata.curr_frequency = 0.0;


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

SoundDevice* new_sound_device(DeviceSettings* ds, MonocoqueSettings* ms, SimInfo* siminfo) {

    SoundDevice* this = (SoundDevice*) malloc(sizeof(SoundDevice));

    this->m.update = &update;
    this->m.free = &simdevfree;
    this->m.derived = this;
    int error = 0;

    switch (ds->hapticsettings.effect_type)
    {
        case (EFFECT_TYRESLIP):
        case (EFFECT_TYRELOCK):
        case (EFFECT_ABSBRAKES):
        case (EFFECT_SUSPENSION):
            if(siminfo->SimSupportsHapticEffects == false)
            {
                slogw("Skipping sound effect setup because sim does not support haptic effects");
                error = MONOCOQUE_ERROR_UNSUPPORTED_SIM_FEATURE;
            }
        defaut:
            error = 0;
    }


    if(error == 0)
    {
        initializeHapticEffect(&this->m.hapticeffect, &ds->hapticsettings, ms);
        slogt("Attempting to configure sound device with subtype: %i", ds->hapticsettings.effect_type);
        switch (ds->hapticsettings.effect_type)
        {
            case (EFFECT_ENGINERPM):
                this->m.vtable = &engine_sound_simdevice_vtable;
                slogi("Initializing sound device for engine vibrations.");
                break;
            case (EFFECT_GEARSHIFT):
                this->m.vtable = &gear_sound_simdevice_vtable;
                slogi("Initializing sound device for gear shift vibrations.");
                break;
            case (EFFECT_TYRESLIP):
                this->m.vtable = &tyreslip_sound_simdevice_vtable;
                slogi("Initializing sound device for tyre slip vibrations.");
                break;
            case (EFFECT_TYRELOCK):
                this->m.vtable = &tyrelock_sound_simdevice_vtable;
                slogi("Initializing sound device for tyre lock vibrations.");
                break;
            case (EFFECT_ABSBRAKES):
                this->m.vtable = &absbrakes_sound_simdevice_vtable;
                slogi("Initializing sound device for abs vibrations.");
                break;

            case (EFFECT_SUSPENSION):
                this->m.vtable = &suspension_sound_simdevice_vtable;
                slogi("Initializing sound device for suspension vibrations.");
                break;
        }
    }

    if(error == 0)
    {
        slogt("Attempting to use sound device %s", ds->sounddevsettings.dev);
        error = sounddev_init(this, ds->sounddevsettings.dev, ds->sounddevsettings);
    }

    if (error != 0)
    {
        free(this);
        return NULL;
    }

    return this;
}
