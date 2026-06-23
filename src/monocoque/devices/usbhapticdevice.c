#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "usbhapticdevice.h"
#include "hapticeffect.h"
#include "usb/cslelitev3.h"
#include "usb/simagicp1000.h"
#include "usb/simnetpedals.h"

#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
#include "../../slog/slog.h"


int usbhapticdev_update(USBGenericHapticDevice* usbhapticdevice, SimData* simdata, int tyre, int useconfig, int* configcheck, char* configfile)
{

    double play = slipeffect(simdata, this->hapticeffect.effecttype, tyre, this->hapticeffect.threshold, useconfig, configcheck, configfile);

    if (play != usbhapticdevice->state)
    {
        int rplay = 0;
        if(play > 0)
        {
            rplay = 1;
            switch ( usbhapticdevice->type )
            {
                case USBHAPTIC_CSLELITEV3PEDALS:
                    cslelitev3_update(usbhapticdevice, this->hapticeffect.effecttype, rplay);
                    break;
                case USBHAPTIC_SIMAGICP1000PEDALS:
                    simagicp1000_update(usbhapticdevice, this->hapticeffect.effecttype, rplay);
                    break;
                case USBHAPTIC_SIMNETPEDALS:
                    simnetpedals_update(usbhapticdevice, this->hapticeffect.effecttype, rplay);
                    break;
            }
        }
        else
        {
            switch ( usbhapticdevice->type )
            {
                case USBHAPTIC_CSLELITEV3PEDALS:
                    cslelitev3_update(usbhapticdevice, this->hapticeffect.effecttype, rplay);
                    break;
                case USBHAPTIC_SIMAGICP1000PEDALS:
                    simagicp1000_update(usbhapticdevice, this->hapticeffect.effecttype, rplay);
                    break;
                case USBHAPTIC_SIMNETPEDALS:
                    simnetpedals_update(usbhapticdevice, this->hapticeffect.effecttype, rplay);
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
        case USBHAPTIC_SIMNETPEDALS:
            simnetpedals_free(usbhapticdevice);
            break;
    }

    return 0;
}

int usbhapticdev_init(USBGenericHapticDevice* usbhapticdevice, DeviceSettings* ds, SimInfo* siminfo)
{
    if(siminfo->SimSupportsHapticEffects == false)
    {
        slogi("This sim does not support haptic effects");
        return MONOCOQUE_ERROR_UNSUPPORTED_SIM_FEATURE;
    }

    int error = 0;
    initializeHapticEffect(&this->m.hapticeffect, &ds->hapticsettings, ms);

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
        case (SIMDEVSUBTYPE_SIMNETPEDALS):
            usbhapticdevice->type = USBHAPTIC_SIMNETPEDALS;
            error = simnetpedals_init(usbhapticdevice);
            break;
        default:
            slogw("Possibly unknown device");
    }

    return error;
}
