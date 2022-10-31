#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usbdevice.h"
#include "../helper/parameters.h"
#include "../simulatorapi/simdata.h"
#include "../slog/slog.h"

int usbdev_update(USBDevice* usbdevice, SimData* simdata)
{
    switch ( usbdevice->type )
    {
        case USBDEV_UNKNOWN :
        case USBDEV_TACHOMETER :
            tachdev_update(&usbdevice->u.tachdevice, simdata);
            break;
    }

    return 0;
}

int usbdev_free(USBDevice* usbdevice)
{
    switch ( usbdevice->type )
    {
        case USBDEV_UNKNOWN :
        case USBDEV_TACHOMETER :
            tachdev_free(&usbdevice->u.tachdevice);
            break;
    }

    return 0;
}

int usbdev_init(USBDevice* usbdevice, DeviceSettings* ds)
{
    slogi("initializing usb device...");
    int error = 0;
    switch ( usbdevice->type )
    {
        case USBDEV_UNKNOWN :
        case USBDEV_TACHOMETER :
            error = tachdev_init(&usbdevice->u.tachdevice, ds);
            break;
    }

    return error;
}
