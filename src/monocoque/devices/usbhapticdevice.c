#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "usbhapticdevice.h"
#include "hapticeffect.h"
#include "usb/cslelitev3.h"
#include "usb/simagicp1000.h"

#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
#include "../../slog/slog.h"


int usbhapticdev_update(USBGenericHapticDevice* usbhapticdevice, SimData* simdata, int tyre, int useconfig, int* configcheck, char* configfile)
{

    double play = slipeffect(simdata, usbhapticdevice->effecttype, tyre, usbhapticdevice->threshold, useconfig, configcheck, configfile);

    if (play != usbhapticdevice->state)
    {
        int rplay = 0;
        if(play > 0)
        {
            rplay = 1;
            switch ( usbhapticdevice->type )
            {
                case USBHAPTIC_CSLELITEV3PEDALS:
                    cslelitev3_update(usbhapticdevice, usbhapticdevice->effecttype, rplay);
                    break;
                case USBHAPTIC_SIMAGICP1000PEDALS:
                    simagicp1000_update(usbhapticdevice, usbhapticdevice->effecttype, rplay);
                    break;
            }
        }
        else
        {
            switch ( usbhapticdevice->type )
            {
                case USBHAPTIC_CSLELITEV3PEDALS:
                    cslelitev3_update(usbhapticdevice, usbhapticdevice->effecttype, rplay);
                    break;
                case USBHAPTIC_SIMAGICP1000PEDALS:
                    simagicp1000_update(usbhapticdevice, usbhapticdevice->effecttype, rplay);
                    break;
            }
        }
        usbhapticdevice->state = play;
    }
    return 0;
}

int usbhapticdev_free(USBGenericHapticDevice* usbhapticdevice)
{
    slogt("closing usb haptic device");
    switch ( usbhapticdevice->type )
    {
        case USBHAPTIC_CSLELITEV3PEDALS:
            cslelitev3_free(usbhapticdevice);
            break;
        case USBHAPTIC_SIMAGICP1000PEDALS:
            simagicp1000_free(usbhapticdevice);
            break;
    }

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
    switch (ds->dev_subsubtype) {
        case (SIMDEVSUBTYPE_SIMAGICP1000PEDALS):
            usbhapticdevice->type = USBHAPTIC_SIMAGICP1000PEDALS;
            error = simagicp1000_init(usbhapticdevice);
            break;
        case (SIMDEVSUBTYPE_CSLELITEV3PEDALS):
            usbhapticdevice->type = USBHAPTIC_CSLELITEV3PEDALS;
            error = cslelitev3_init(usbhapticdevice);
            break;
        default:
            slogw("Possibly unknown device");
    }

    return error;
}
