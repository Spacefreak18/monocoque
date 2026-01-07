#include <stdio.h>
#include <math.h>

#include <hidapi/hidapi.h>

#include "logitechg29.h"
#include "../../../slog/slog.h"

const int logitechg29_hidupdate_buf_size = 7;

int logitechg29_update(USBDevice* usbdevice, int maxrpm, int rpm, int gear, int velocity)
{

    int res = 0;

    uint8_t leds = 0;

    float percent = 0;
    if(maxrpm > 0)
    {
        percent = (float) rpm / (float) maxrpm;

        if (percent > .84)
        {
            leds = 0b11111;
        }
        else if (percent > .69)
        {
            leds = 0b1111;
        }
        else if (percent > .39)
        {
            leds = 0b111;
        }
        else if (percent > .19)
        {
            leds = 0b11;
        }
        else if (percent > .4)
        {
            leds = 0b1;
        }
        else
        {
            leds = 0b0;
        }
    }
    uint8_t bytes[7] = {
        0xF8,
        0x12,
        leds,
        0x00,
        0x00,
        0x00,
        0x01
    };

    slogt("writing bytes x%02xx%02xx%02xx%02xx%02x from rpm %i", bytes[0], bytes[1], bytes[2], bytes[3], bytes[6], rpm);
    if (usbdevice->handle)
    {
        res = hid_write(usbdevice->handle, bytes, logitechg29_hidupdate_buf_size);
    }
    else
    {
        slogd("no handle");
    }

    return res;
}

int logitechg29_free(USBDevice* usbdevice)
{
    int res = 0;

    hid_close(usbdevice->handle);
    res = hid_exit();

    return res;
}

int logitechg29_init(USBDevice* usbdevice)
{
    slogi("initializing Logitech G29 wheel...");

    int res = 0;

    res = hid_init();
    usbdevice->handle = hid_open(0x046D, 0xC24F, NULL);

    if (!usbdevice->handle)
    {
        sloge("Could not find attached Logitech G29 Wheel");
        res = hid_exit();
        return 1;
    }
    slogd("Found Logitech G29 Wheel...");
    return res;
}
