#include <stdio.h>

#include "sounddevice.h"
#include "sound/usb_generic_shaker.h"
#include "../simulatorapi/simdata.h"
#include "../helper/parameters.h"
#include "../slog/slog.h"

int sounddev_update(SoundDevice* sounddevice, SimData* simdata)
{
    sounddevice->sounddata.table_size = 44100/(simdata->rpms/60);

    sounddevice->sounddata.gear_sound_data = 0;
    if (sounddevice->sounddata.last_gear != simdata->gear)
    {
        sounddevice->sounddata.gear_sound_data = sounddevice->sounddata.amp;
    }
    sounddevice->sounddata.last_gear = simdata->gear;
}

int sounddev_free(SoundDevice* sounddevice)
{
    return usb_generic_shaker_free(sounddevice);
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

    usb_generic_shaker_init(sounddevice);
}
