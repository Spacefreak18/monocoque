#include <stdio.h>

#include <hidapi/hidapi.h>

#include "../simdevice.h"
#include "../../slog/slog.h"

const size_t REVBURNER_BUFSIZE = 8;


int revburner_update(USBDevice* tachdevice, int pulses)
{

    int res = 0;

    unsigned char bytes[REVBURNER_BUFSIZE];
    for (int x = 0; x < REVBURNER_BUFSIZE; x++)
    {
        bytes[x] = 0x00;
    }
    if ( pulses > 0 )
    {
        bytes[3] = (pulses >> 8) & 0xFF;
        bytes[2] = pulses & 0xFF;
    }


    if (tachdevice->handle)
    {
        res = hid_write(tachdevice->handle, bytes, REVBURNER_BUFSIZE);
    }
    else
    {
        slogd("no handle");
    }

    return res;
}

int revburner_free(USBDevice* tachdevice)
{
    int res = 0;

    hid_close(tachdevice->handle);
    res = hid_exit();

    return res;
}

int revburner_init(USBDevice* tachdevice)
{
    slogi("initializing revburner tachometer...");
    //tachdevice->update_tachometer = revburner_device_update;

    int res = 0;

    res = hid_init();

    tachdevice->handle = hid_open(0x4d8, 0x102, NULL);

    if (!tachdevice->handle)
    {
        sloge("Could not find attached RevBurner tachometer");
        res = hid_exit();
        return 1;
    }
    slogd("Found RevBurner Tachometer handle %i...", tachdevice->handle);
    return res;
}
