#include <stdio.h>

#include <hidapi/hidapi.h>

#include "cammusc5.h"
#include "../../../slog/slog.h"

const int cammusc5_hidupdate_buf_size = 14;
const int num_avail_leds = 9;

int cammusc5_update(WheelDevice* wheeldevice, int maxrpm, int rpm, int gear, int velocity)
{

    int res = 0;

    unsigned char bytes[cammusc5_hidupdate_buf_size];
   
    // byte 1 must be fc it seems
    bytes[0] = 0xFC;


    // byte 2 is number of lit leds, assuming 9 available leds,
    // if we send 10, all leds will blink singling a gear change
    // attempting to build in a margin before the maxrpm is achieved
    int rpmmargin = .05*maxrpm;
    int rpminterval = (maxrpm-rpmmargin) / (num_avail_leds+1);

    int litleds = 0;
    for (int x = 1; x <= (num_avail_leds+1); x++)
    {
        if(rpm >= (rpminterval * x))
        {
            litleds = x;
        }
    }
    bytes[1] = litleds;

    // bytes 2 and 3 are a 16 bit velocity
    if ( velocity > 0 )
    {
        bytes[3] = (velocity >> 8) & 0xFF;
        bytes[2] = velocity & 0xFF;
    }

    // byte 4 is gear
    bytes[4] = gear;


    if (wheeldevice->handle)
    {
        res = hid_write(wheeldevice->handle, bytes, cammusc5_hidupdate_buf_size);
    }
    else
    {
        slogd("no handle");
    }

    return res;
}

int cammusc5_free(WheelDevice* wheeldevice)
{
    int res = 0;

    hid_close(wheeldevice->handle);
    res = hid_exit();

    return res;
}

int cammusc5_init(WheelDevice* wheeldevice)
{
    slogi("initializing cammus c5 wheel...");

    int res = 0;

    res = hid_init();

    wheeldevice->handle = hid_open(0x3416, 0x0301, NULL);

    if (!wheeldevice->handle)
    {
        sloge("Could not find attached Cammus C5 Wheel");
        res = hid_exit();
        return 1;
    }
    slogd("Found Cammus C5 Wheel...");
    return res;
}
