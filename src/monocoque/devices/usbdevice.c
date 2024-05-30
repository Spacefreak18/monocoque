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
        case USBDEV_GENERICHAPTIC :
            usbhapticdev_update(&usbdevice->u.hapticdevice, simdata);
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
        case USBDEV_GENERICHAPTIC :
            error = usbhapticdev_init(&usbdevice->u.hapticdevice, ds);
            break;
    }

    return error;
}

static const vtable usb_simdevice_vtable = { &usbdev_update, &usbdev_free };

USBDevice* new_usb_device(DeviceSettings* ds) {

    USBDevice* this = (USBDevice*) malloc(sizeof(USBDevice));

    this->m.update = &update;
    this->m.free = &simdevfree;
    this->m.derived = this;
    this->m.vtable = &usb_simdevice_vtable;

    this->type = USBDEV_TACHOMETER;
    if (ds->dev_subtype == SIMDEVTYPE_USBHAPTIC)
    {
        this->type = USBDEV_GENERICHAPTIC;
    }

    int error = usbdev_init(this, ds);

    if (error != 0)
    {
        free(this);
        return NULL;
    }
    return this;
}
