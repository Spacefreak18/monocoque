#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "usbhapticdevice.h"
#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
#include "../../slog/slog.h"

int usbhapticdev_update(USBGenericHapticDevice* usbhapticdevice, SimData* simdata)
{
    double play = 0;
    switch (usbhapticdevice->effecttype)
    {
        case (EFFECT_TYRESLIP):

            if (usbhapticdevice->effecttype == EFFECT_TYRESLIP)
            {
                if (usbhapticdevice->tyre == FRONTLEFT || usbhapticdevice->tyre == FRONTS || usbhapticdevice->tyre == ALLFOUR)
                {
                    play += simdata->wheelslip[0];
                }
                if (usbhapticdevice->tyre == FRONTRIGHT || usbhapticdevice->tyre == FRONTS || usbhapticdevice->tyre == ALLFOUR)
                {
                    play += simdata->wheelslip[1];
                }
                if (usbhapticdevice->tyre == REARLEFT || usbhapticdevice->tyre == REARS || usbhapticdevice->tyre == ALLFOUR)
                {
                    play += simdata->wheelslip[2];
                }
                if (usbhapticdevice->tyre == REARRIGHT || usbhapticdevice->tyre == REARS || usbhapticdevice->tyre == ALLFOUR)
                {
                    play += simdata->wheelslip[3];
                }
            }
            break;
        case (EFFECT_ABSBRAKES):
            play += simdata->abs;
            break;
    }
    
    if (play != usbhapticdevice->state)
    {
        if(play > 0)
        {
            fprintf(usbhapticdevice->handle, "%i\n", usbhapticdevice->value1);
            fflush(usbhapticdevice->handle);
        }
        else
        {
            fprintf(usbhapticdevice->handle, "%i\n", usbhapticdevice->value0);
            fflush(usbhapticdevice->handle);

        }
        usbhapticdevice->state = play;
    }
    return 0;
}

int usbhapticdev_free(USBGenericHapticDevice* usbhapticdevice)
{
    slogt("closing usb haptic device");
    fflush(usbhapticdevice->handle);
    fclose(usbhapticdevice->handle);
    free(usbhapticdevice->dev);
    return 0;
}

int usbhapticdev_init(USBGenericHapticDevice* usbhapticdevice, DeviceSettings* ds)
{
    int error = 0;
    usbhapticdevice->state = 0;
    usbhapticdevice->dev = strdup(ds->usbdevsettings.dev);
    usbhapticdevice->value0 = ds->usbdevsettings.value0;
    usbhapticdevice->value1 = ds->usbdevsettings.value1;
    usbhapticdevice->state = usbhapticdevice->value0;
    usbhapticdevice->tyre = ds->tyre;
    usbhapticdevice->effecttype = ds->effect_type;

    usbhapticdevice->handle = fopen(usbhapticdevice->dev, "w");
    if(usbhapticdevice->handle == 0)
    {
        error = MONOCOQUE_ERROR_INVALID_DEV;
        return error;
    }

    slogt("Initializing standalone usb haptic device");
    if(ds->effect_type == EFFECT_TYRESLIP)
    {
        usbhapticdevice->effecttype = EFFECT_TYRESLIP;
    }
    if(ds->effect_type == EFFECT_WHEELLOCK)
    {
        usbhapticdevice->effecttype = EFFECT_WHEELLOCK;
    }

    return error;
}
