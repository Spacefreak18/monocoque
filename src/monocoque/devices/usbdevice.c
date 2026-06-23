#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "usbdevice.h"
#include "simdevice.h"
#include "../helper/parameters.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../slog/slog.h"

int usbdev_update(SimDevice* this, SimData* simdata)
{
    USBDevice* usbdevice = (void *) this->derived;

    switch ( usbdevice->type )
    {
        case USBDEV_UNKNOWN :
        case USBDEV_TACHOMETER :
            tachdev_update(usbdevice, simdata);
            break;
        case USBDEV_WHEEL_OR_PEDALS :
            // this should be directed via vtable to wheeldevice.c
            //wheeldev_update(usbdevice, simdata);
            break;
    }

    return 0;
}

int usbdev_free(SimDevice* this)
{
    USBDevice* usbdevice = (void *) this->derived;

    slogt("Usb device free");
    switch ( usbdevice->type )
    {
        case USBDEV_UNKNOWN :
        case USBDEV_TACHOMETER :
            tachdev_free(usbdevice);
            break;
        case USBDEV_WHEEL_OR_PEDALS :
            wheeldev_free(usbdevice);
            break;
    }

    free(usbdevice);

    return 0;
}

int usbdev_init(USBDevice* usbdevice, DeviceSettings* ds, SimInfo* siminfo)
{
    slogi("initializing usb device...");
    int error = 0;

    //usbdevice->type = USBDEV_TACHOMETER;
    switch ( usbdevice->type )
    {
        case USBDEV_UNKNOWN :
        case USBDEV_TACHOMETER :
            error = tachdev_init(usbdevice, ds);
            break;
        case USBDEV_WHEEL_OR_PEDALS :
            error = wheeldev_init(usbdevice, ds);
            break;
    }

    return error;
}


static const vtable usb_simdevice_vtable = { &usbdev_update, &usbdev_free };
static const vtable usb_wheeldevice_vtable = { &wheeldev_update, &usbdev_free };
static const vtable usb_wheelhaptic_vtable = { &wheelhapticdev_update, &usbdev_free };

USBDevice* new_usb_device(DeviceSettings* ds, MonocoqueSettings* ms, SimInfo* siminfo) {

    USBDevice* this = (USBDevice*) malloc(sizeof(USBDevice));
    int error = 0;

    this->m.update = &update;
    this->m.free = &simdevfree;
    this->m.derived = this;
    this->m.vtable = &usb_simdevice_vtable;

    // TODO: turn this into a switch when we get more devices
    this->type = USBDEV_TACHOMETER;
    if(ds->dev_subtype == SIMDEVTYPE_USBWHEEL)
    {
        this->type = USBDEV_WHEEL_OR_PEDALS;
        this->m.vtable = &usb_wheeldevice_vtable;
    }

    if(ds->has_haptic_effects == true)
    {
        if(siminfo->SimSupportsHapticEffects == false)
        {
            // if the user added haptic effects to the config and the sim does not support it, but still wishes to use features
            // just remove the haptic effect from the config?
            slogi("This sim does not support haptic effects");
            error = MONOCOQUE_ERROR_UNSUPPORTED_SIM_FEATURE;
        }
        else
        {
            int error = 0;
            initializeHapticEffect(&this->m.hapticeffect, &ds->hapticsettings, ms);
            this->m.vtable = &usb_wheelhaptic_vtable;
        }
    }

    error = usbdev_init(this, ds, siminfo);

    if (error != 0)
    {
        slogw("Did not initialize usb device due to error code %i", error);
        free(this);
        return NULL;
    }

    this->m.initialized = true;
    return this;
}
