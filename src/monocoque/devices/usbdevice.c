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
            tachdev_update(&usbdevice->u.tachdevice, simdata);
            break;
        case USBDEV_WHEEL :
            wheeldev_update(&usbdevice->u.wheeldevice, simdata);
            break;
        case USBDEV_GENERICHAPTIC :
            usbhapticdev_update(&usbdevice->u.hapticdevice, simdata, this->hapticeffect.tyre, this->hapticeffect.useconfig, this->hapticeffect.configcheck, this->hapticeffect.tyrediameterconfig);
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
            tachdev_free(&usbdevice->u.tachdevice);
            break;
        case USBDEV_WHEEL :
            wheeldev_free(&usbdevice->u.wheeldevice);
            break;
        case USBDEV_GENERICHAPTIC :
            usbhapticdev_free(&usbdevice->u.hapticdevice);
            break;
    }

    free(usbdevice);

    return 0;
}

int usbdev_init(USBDevice* usbdevice, DeviceSettings* ds)
{
    slogi("initializing usb device...");
    int error = 0;

    //usbdevice->type = USBDEV_TACHOMETER;
    switch ( usbdevice->type )
    {
        case USBDEV_UNKNOWN :
        case USBDEV_TACHOMETER :
            error = tachdev_init(&usbdevice->u.tachdevice, ds);
            break;
        case USBDEV_WHEEL :
            error = wheeldev_init(&usbdevice->u.wheeldevice, ds);
            break;
        case USBDEV_GENERICHAPTIC :
            error = usbhapticdev_init(&usbdevice->u.hapticdevice, ds);
            break;
    }

    return error;
}

static const vtable usb_simdevice_vtable = { &usbdev_update, &usbdev_free };

USBDevice* new_usb_device(DeviceSettings* ds, MonocoqueSettings* ms) {

    USBDevice* this = (USBDevice*) malloc(sizeof(USBDevice));

    this->m.update = &update;
    this->m.free = &simdevfree;
    this->m.derived = this;
    this->m.vtable = &usb_simdevice_vtable;



    // TODO: turn this into a switch when we get more devices
    this->type = USBDEV_TACHOMETER;
    if(ds->dev_subtype == SIMDEVTYPE_USBWHEEL)
    {
        this->type = USBDEV_WHEEL;
    }


    // really generic haptic isn't and shouldn't be it's own type
    // it's an attribute that is added to a device via composition
    // same if that haptic device is a serial device
    if (ds->dev_subtype == SIMDEVTYPE_USBHAPTIC)
    {
        this->m.hapticeffect.threshold = ds->threshold;
        this->m.hapticeffect.effecttype = ds->effect_type;
        this->m.hapticeffect.tyre = ds->tyre;
        this->m.hapticeffect.useconfig = ms->useconfig;
        this->m.hapticeffect.configcheck = &ms->configcheck;
        this->m.hapticeffect.tyrediameterconfig = ms->tyre_diameter_config;
        this->type = USBDEV_GENERICHAPTIC;
    }

    int error = usbdev_init(this, ds);

    if (error != 0)
    {
        free(this);
        return NULL;
    }

    this->m.initialized = true;
    return this;
}
