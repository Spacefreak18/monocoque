#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "usb/wheels/logitechg29.h"
#include "usb/wheels/cammusc5.h"
#include "usb/wheels/cammusc12.h"
#include "usb/wheels/gtneo.h"
#include "serial/moza.h"


#include "usb/cslelitev3.h"
#include "usb/simagicp1000.h"
#include "usb/simnetpedals.h"

#include "simdevice.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../slog/slog.h"

int wheelhapticdev_update(SimDevice* this, SimData* simdata)
{
    USBDevice* usbdevice = (void *) this->derived;
    WheelDevice* wheeldevice = &usbdevice->u.wheeldevice;

    double play = slipeffect(simdata, &this->hapticeffect, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);

    if (play != usbdevice->hapticstate)
    {
        int rplay = 0;
        if(play > 0)
        {
            rplay = 1;
            switch ( wheeldevice->type )
            {
                case WHEELDEV_CSLELITEV3PEDALS:
                    cslelitev3_update(usbdevice, this->hapticeffect.effecttype, rplay);
                    break;
                case WHEELDEV_SIMAGICP1000PEDALS:
                    simagicp1000_update(usbdevice, this->hapticeffect.effecttype, rplay);
                    break;
                case WHEELDEV_SIMNETPEDALS:
                    simnetpedals_update(usbdevice, this->hapticeffect.effecttype, rplay, this->hapticeffect.motorposition, this->hapticeffect.basefrequency, this->hapticeffect.baseamplitude);
                    break;
            }
        }
        else
        {
            switch ( wheeldevice->type )
            {
                case WHEELDEV_CSLELITEV3PEDALS:
                    cslelitev3_update(usbdevice, this->hapticeffect.effecttype, rplay);
                    break;
                case WHEELDEV_SIMAGICP1000PEDALS:
                    simagicp1000_update(usbdevice, this->hapticeffect.effecttype, rplay);
                    break;
                case WHEELDEV_SIMNETPEDALS:
                    simnetpedals_update(usbdevice, this->hapticeffect.effecttype, rplay, this->hapticeffect.motorposition, 0, 0);
                    break;
            }
        }
        usbdevice->hapticstate = play;
    }
    return 0;
}

int wheeldev_update(SimDevice* this, SimData* simdata)
{
    USBDevice* usbdevice = (void *) this->derived;
    WheelDevice* wheeldevice = &usbdevice->u.wheeldevice;

    switch ( wheeldevice->type )
    {
        case WHEELDEV_UNKNOWN :
        case WHEELDEV_CAMMUSC5 :
            cammusc5_update(usbdevice, simdata->maxrpm, simdata->rpms, simdata->gear, simdata->velocity);
            break;
        case WHEELDEV_LOGITECHG29 :
            logitechg29_update(usbdevice, simdata->maxrpm, simdata->rpms, simdata->gear, simdata->velocity);
            break;
        case WHEELDEV_CAMMUSC12 :
            if(usbdevice->u.wheeldevice.useLua == true)
            {
                cammusc12_customled_update(usbdevice, simdata);
            }
            else
            {
                cammusc12_update(usbdevice, simdata->maxrpm, simdata->rpms, simdata->gear, simdata->velocity);
            }
            break;
        case WHEELDEV_SIMAGICGTNEO :
            if(usbdevice->u.wheeldevice.useLua == true)
            {
                simagic_gtneo_customled_update(usbdevice, simdata);
            }
            else
            {
                sloge("GT Neo requires config file");
            }
            break;    
    }

    return 0;
}

int wheeldev_free(USBDevice* usbdevice)
{
    WheelDevice* wheeldevice = &usbdevice->u.wheeldevice;
    slogt("wheel device free");
    switch ( wheeldevice->type )
    {
        case WHEELDEV_UNKNOWN :
        case WHEELDEV_CAMMUSC5 :
            cammusc5_update(usbdevice, 0, 0, 0, 0);
            cammusc5_free(usbdevice);
            break;
        case WHEELDEV_LOGITECHG29 :
            logitechg29_update(usbdevice, 0, 0, 0, 0);
            logitechg29_free(usbdevice);
            break;
        case WHEELDEV_CAMMUSC12 :
            cammusc12_free(usbdevice);
            if(wheeldevice->useLua == true)
            {
                free(usbdevice->m.device_specific_config_file);
            }
            break;
        case WHEELDEV_CSLELITEV3PEDALS:
            cslelitev3_free(usbdevice);
            break;
        case WHEELDEV_SIMAGICP1000PEDALS:
            simagicp1000_free(usbdevice);
            break;
        case WHEELDEV_SIMNETPEDALS:
            simnetpedals_free(usbdevice);
            break;
    }

    return 0;
}

int wheeldev_init(USBDevice* usbdevice, DeviceSettings* ds)
{
    slogi("initializing wheel or pedals device...");
    int error = 0;
    usbdevice->hapticstate = 0;
    // detection of wheel model
    WheelDevice* wheeldevice = &usbdevice->u.wheeldevice;
    switch (ds->dev_subsubtype) {

        case SIMDEVSUBTYPE_CAMMUSC5:
            wheeldevice->type = WHEELDEV_CAMMUSC5;
            slogi("Attempting to initialize cammus C5");
            error = cammusc5_init(usbdevice);
            break;
        case SIMDEVSUBTYPE_LOGITECH_G29:
            wheeldevice->type = WHEELDEV_LOGITECHG29;
            slogi("Attempting to initialize Logitech G29");
            error = logitechg29_init(usbdevice);
            break;
        case SIMDEVSUBTYPE_CAMMUSC12:
            wheeldevice->type = WHEELDEV_CAMMUSC12;
            slogi("Attempting to initialize cammus C12");

            if(ds->has_config == true)
            {
                usbdevice->m.device_specific_config_file = strdup(ds->specific_config_file);
                error = cammusc12_init(usbdevice, usbdevice->m.device_specific_config_file);
                if(error != 0)
                {
                    if(usbdevice->m.device_specific_config_file != NULL)
                    {
                        free(usbdevice->m.device_specific_config_file);
                    }
                }
            }
            else
            {
                error = cammusc12_init(usbdevice, NULL);
            }
            break;
        case SIMDEVSUBTYPE_SIMAGICGTNEO:
            wheeldevice->type = WHEELDEV_SIMAGICGTNEO;
            slogi("Attempting to initialize Simagic GT Neo");

            if(ds->has_config == true)
            {
                usbdevice->m.device_specific_config_file = strdup(ds->specific_config_file);
                error = simagic_gtneo_init(usbdevice, usbdevice->m.device_specific_config_file);
                if(error != 0)
                {
                    if(usbdevice->m.device_specific_config_file != NULL)
                    {
                        free(usbdevice->m.device_specific_config_file);
                    }
                }
            }
            else
            {
                sloge("Simagic GT Neo requires lua config file to function");
                error = -1;
            }
            break;
        case (SIMDEVSUBTYPE_SIMAGICP1000PEDALS):
            wheeldevice->type = WHEELDEV_SIMAGICP1000PEDALS;
            error = simagicp1000_init(usbdevice);
            break;
        case (SIMDEVSUBTYPE_CSLELITEV3PEDALS):
            slogi("Attempting to initialize CSL Elite V3 Pedals");
            wheeldevice->type = WHEELDEV_CSLELITEV3PEDALS;
            error = cslelitev3_init(usbdevice);
            break;
        case (SIMDEVSUBTYPE_SIMNETPEDALS):
            wheeldevice->type = WHEELDEV_SIMNETPEDALS;
            error = simnetpedals_init(usbdevice);
            break;
        default:
            slogw("Possibly unknown wheel device");
    }



    return error;
}
