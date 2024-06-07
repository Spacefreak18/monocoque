#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "usbhapticdevice.h"
#include "hapticeffect.h"
#include "usb/cslelitev3.h"
#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
#include "../../slog/slog.h"


int usbhapticdev_update(USBGenericHapticDevice* usbhapticdevice, SimData* simdata, int tyre, int* configcheck)
{

    int play = slipeffect(simdata, usbhapticdevice->effecttype, tyre, usbhapticdevice->threshold, configcheck);
    
    if (play != usbhapticdevice->state)
    {
        if(play > 0)
        {
            cslelitev3_update(usbhapticdevice, usbhapticdevice->effecttype, play);
        }
        else
        {
            cslelitev3_update(usbhapticdevice, usbhapticdevice->effecttype, play);
        }
        usbhapticdevice->state = play;
    }
    return 0;
}

int usbhapticdev_free(USBGenericHapticDevice* usbhapticdevice)
{
    slogt("closing usb haptic device");
    cslelitev3_free(usbhapticdevice);
    return 0;
}

int usbhapticdev_init(USBGenericHapticDevice* usbhapticdevice, DeviceSettings* ds)
{
    int error = 0;
    usbhapticdevice->state = 0;
    usbhapticdevice->value0 = ds->usbdevsettings.value0;
    usbhapticdevice->value1 = ds->usbdevsettings.value1;
    usbhapticdevice->state = usbhapticdevice->value0;
    usbhapticdevice->effecttype = ds->effect_type;
    usbhapticdevice->threshold = ds->threshold;

    if(ds->effect_type == EFFECT_TYRESLIP)
    {
        usbhapticdevice->effecttype = EFFECT_TYRESLIP;
    }
    if(ds->effect_type == EFFECT_TYRELOCK)
    {
        usbhapticdevice->effecttype = EFFECT_TYRELOCK;
    }
    if(ds->effect_type == EFFECT_ABSBRAKES)
    {
        usbhapticdevice->effecttype = EFFECT_ABSBRAKES;
    }

    slogi("initializing standalone usb haptic device...");
    // detection of usb device model
    usbhapticdevice->type = HAPTIC_UNKNOWN;
    usbhapticdevice->type = HAPTIC_CSLELITEV3;
    error = cslelitev3_init(usbhapticdevice);

    if(usbhapticdevice->handle == 0)
    {
        error = MONOCOQUE_ERROR_INVALID_DEV;
        return error;
    }

    return error;
}
