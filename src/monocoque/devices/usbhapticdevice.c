#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "usbhapticdevice.h"
#include "../../helper/confighelper.h"
#include "../../simulatorapi/simapi/simapi/simdata.h"
#include "../../slog/slog.h"

bool hasTyreDiameter(SimData* simdata)
{
    if (simdata->tyrediameter[0] == -1 || simdata->tyrediameter[1] == -1 || simdata->tyrediameter[2] == -1 || simdata->tyrediameter[3] == -1)
    {
        return false;
    }
    return true;
}

void getTyreDiameter(SimData* simdata)
{
    if(simdata->velocity > 50 && simdata->brake <= 0 && simdata->gas <= 0)
    {
        if (simdata->velocityX/simdata->velocity < 0.001)
        {
            double Speedms = 0.277778 * simdata->velocity;
            for(int i = 0; i < 4; i++)
            {
                simdata->tyrediameter[i] = Speedms / simdata->tyreRPS[i] * 2;
            }
            slogi("Successfully set tyre diameters for wheel slip effects.");
        }

    }
}


int usbhapticdev_update(USBGenericHapticDevice* usbhapticdevice, SimData* simdata)
{
    double play = 0;
    double playthreshhold = 0;
    double wheelslip[4];
    wheelslip[0] = 0;
    wheelslip[1] = 0;
    wheelslip[2] = 0;
    wheelslip[3] = 0;


    switch (usbhapticdevice->effecttype)
    {
        case (EFFECT_TYRESLIP):
        case (EFFECT_TYRELOCK):
        case (EFFECT_ABSBRAKES):

            if(hasTyreDiameter(simdata)==false)
            {
                getTyreDiameter(simdata);
            }
            if(hasTyreDiameter(simdata)==true)
            {
                double Speedms = 0.277778 * simdata->velocity;
                if (Speedms > 0.5)
                {
                    for(int i = 0; i < 4; i++)
                    {
                        wheelslip[i] = (Speedms - simdata->tyrediameter[i] * simdata->tyreRPS[i] / 2) / Speedms;
                    }
                }
                else
                {
                    for(int i = 0; i < 4; i++)
                    {
                        wheelslip[i] = 0;
                    }
                }

            }
            break;
    }

    switch (usbhapticdevice->effecttype)
    {
        case (EFFECT_TYRESLIP):


            if (usbhapticdevice->tyre == FRONTLEFT || usbhapticdevice->tyre == FRONTS || usbhapticdevice->tyre == ALLFOUR)
            {
                if(wheelslip[0] < -.5)
                {
                    play++;
                }
            }
            if (usbhapticdevice->tyre == FRONTRIGHT || usbhapticdevice->tyre == FRONTS || usbhapticdevice->tyre == ALLFOUR)
            {
                if(wheelslip[1] < -.5)
                {
                    play++;
                }
            }
            if (usbhapticdevice->tyre == REARLEFT || usbhapticdevice->tyre == REARS || usbhapticdevice->tyre == ALLFOUR)
            {
                if(wheelslip[2] < -.5)
                {
                    play++;
                }
            }
            if (usbhapticdevice->tyre == REARRIGHT || usbhapticdevice->tyre == REARS || usbhapticdevice->tyre == ALLFOUR)
            {
                if(wheelslip[3] < -.5)
                {
                    play++;
                }
            }


            break;
        case (EFFECT_TYRELOCK):
            if (usbhapticdevice->tyre == FRONTLEFT || usbhapticdevice->tyre == FRONTS || usbhapticdevice->tyre == ALLFOUR)
            {
                if(wheelslip[0] > .75)
                {
                    play++;
                }
            }
            if (usbhapticdevice->tyre == FRONTRIGHT || usbhapticdevice->tyre == FRONTS || usbhapticdevice->tyre == ALLFOUR)
            {
                if(wheelslip[1] > .75)
                {
                    play++;
                }
            }
            if (usbhapticdevice->tyre == REARLEFT || usbhapticdevice->tyre == REARS || usbhapticdevice->tyre == ALLFOUR)
            {
                if(wheelslip[2] > .75)
                {
                    play++;
                }
            }
            if (usbhapticdevice->tyre == REARRIGHT || usbhapticdevice->tyre == REARS || usbhapticdevice->tyre == ALLFOUR)
            {
                if(wheelslip[3] > .75)
                {
                    play++;
                }
            }

            break;
        case (EFFECT_ABSBRAKES):
            break;
    }
    
    if (play != usbhapticdevice->state)
    {
        if(play > playthreshhold)
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
    if(ds->effect_type == EFFECT_TYRELOCK)
    {
        usbhapticdevice->effecttype = EFFECT_TYRELOCK;
    }

    return error;
}
