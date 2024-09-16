#include <stdio.h>
#include <math.h>

#include <hidapi/hidapi.h>

#include "cammusc12.h"
#include "../../../slog/slog.h"

const int cammusc12_hidupdate_buf_size = 16;

int cammusc12_update(WheelDevice* wheeldevice, int maxrpm, int rpm, int gear, int velocity)
{

    int res = 0;

    unsigned char bytes[cammusc12_hidupdate_buf_size];
   
    for (int x = 0; x < cammusc12_hidupdate_buf_size; x++)
    {
        bytes[x] = 0x00;
    }
    // byte 1 must be fc it seems
    bytes[0] = 0xFA;
    bytes[1] = 0xFB;
    bytes[2] = 0xD4;

    int perct = 0;
    if(rpm > 0 && maxrpm > 0)
    {
        double rpmperct = (double) rpm / (double) maxrpm;
        perct = trunc(nearbyint( rpmperct * 100 ));
    }
    bytes[3] = perct;

    // bytes 2 and 3 are a 16 bit velocity
    if ( velocity > 0 )
    {
        bytes[4] = (velocity >> 8) & 0xFF;
        bytes[5] = velocity & 0xFF;
    }

    // byte 4 is gear
    bytes[6] = gear-1;

    slogt("writing bytes x%02xx%02xx%02xx%02xx%02x%02x%02x from rpm %i velocity %i gear %i", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], rpm, velocity, gear);
    if (wheeldevice->handle)
    {
        res = hid_write(wheeldevice->handle, bytes, cammusc12_hidupdate_buf_size);
    }
    else
    {
        slogd("no handle");
    }

    return res;
}

int cammusc12_free(WheelDevice* wheeldevice)
{
    int res = 0;

    hid_close(wheeldevice->handle);
    res = hid_exit();

    return res;
}

int cammusc12_init(WheelDevice* wheeldevice)
{
    slogi("initializing cammus c12 wheel...");

    int res = 0;

    res = hid_init();

    wheeldevice->handle = hid_open(0x3416, 0x1023, NULL);

    if (!wheeldevice->handle)
    {
        sloge("Could not find attached Cammus C12 Wheel");
        res = hid_exit();
        return 1;
    }
    slogd("Found Cammus C12 Wheel...");
    return res;
}
