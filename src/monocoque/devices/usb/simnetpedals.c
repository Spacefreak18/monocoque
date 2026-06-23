#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

#include "simnetpedals.h"

#include "../../helper/confighelper.h"
#include "../slog/slog.h"

#define SIMNETPEDALS_BUFSIZE 64

//Byte[0] : 0x01 (fixed) = Report ID
//Byte[1] : 0x00 (fixed)
//Byte[2] : motor number (0x01, 0x2 or 0x4)
//Byte[3]: motor 1 frequency in hz
//Byte[4] : motor 1 amplitude (0 to 100%)
//Byte[5] : motor 2 frequency in hz
//Byte[6]: motor 2 amplitude (0 to 100%)
//Byte[7]: motor 3 frequency in hz
//Byte[8]: motor 3 amplitude (0 to 100%)


int simnetpedals_update(USBDevice* usbdevice, int effecttype, int play, int motorposition, int frequency, int amplitude)
{

    int res = 0;

    unsigned char bytes[SIMNETPEDALS_BUFSIZE];
    for (int x = 0; x < SIMNETPEDALS_BUFSIZE; x++)
    {
        bytes[x] = 0x00;
    }
    bytes[0] = 0x01;
    bytes[1] = 0x00;
    bytes[2] = motorposition;
    uint8_t motormultiple = motorposition - 1;
    if(motorposition == 0x04)
    {
        motormultiple = 2;
    }
    if(play > 0)
    {
        bytes[3 + (motormultiple * 2)] = frequency;
        bytes[4 + (motormultiple * 2)] = amplitude;
    }

    
    if (usbdevice->handle)
    {
        slogt("writing bytes x%02xx%02xx%02xx%02xx%02xx%02xx%02xx%02xx%02x", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], bytes[8]);
        res = hid_write(usbdevice->handle, bytes, SIMNETPEDALS_BUFSIZE);
    }
    else
    {
        slogd("no handle");
    }

    return res;
}

int simnetpedals_free(USBDevice* usbdevice)
{
    int res = 0;

    hid_close(usbdevice->handle);
    res = hid_exit();

    return res;
}

int simnetpedals_init(USBDevice* usbdevice)
{
    slogi("initializing simnet pedals...");

    int res = 0;

    res = hid_init();

    usbdevice->handle = hid_open(0xcafe, 0xa301, NULL);

    if (!usbdevice->handle)
    {
        sloge("Could not find attached Simnet Pedals");
        res = hid_exit();
        return 1;
    }
    slogd("Found Simnet Pedals handle %i...", usbdevice->handle);
    return res;
}
