#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

#include "usbhapticdevice.h"

#include "../../helper/confighelper.h"
#include "../slog/slog.h"

#define SIMNETPEDALS_BUFSIZE 9

//Byte[0] : 0x01 (fixed) = Report ID
//Byte[1] : 0x00 (fixed)
//Byte[2] : motor number (0x01, 0x2 or 0x4)
//Byte[3]: motor 1 frequency in hz
//Byte[4] : motor 1 amplitude (0 to 100%)
//Byte[5] : motor 2 frequency in hz
//Byte[6]: motor 2 amplitude (0 to 100%)
//Byte[7]: motor 3 frequency in hz
//Byte[8]: motor 3 amplitude (0 to 100%)


int simnetpedals_update(USBGenericHapticDevice* usbhapticdevice, int effecttype, int play)
{

    int res = 0;

    unsigned char bytes[SIMNETPEDALS_BUFSIZE];
    for (int x = 0; x < SIMNETPEDALS_BUFSIZE; x++)
    {
        bytes[x] = 0x00;
    }
    bytes[0] = 0x01;
    bytes[1] = 0x00;
    bytes[2] = usbhapticdevice->motorposition;
    uint8_t motormultiple = usbhapticdevice->motorposition - 1;
    if(usbhapticdevice->motorposition == 0x04)
    {
        motormultiple = 2;
    }
    bytes[3 + (motormultiple * 2)] = usbhapticdevice->frequency;
    bytes[4 + (motormultiple * 2)] = usbhapticdevice->amplitude;

    
    if (usbhapticdevice->handle)
    {
        slogt("writing bytes x%02xx%02xx%02xx%02xx%02xx%02xx%02xx%02xx%02x", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], bytes[8]);
        res = hid_write(usbhapticdevice->handle, bytes, SIMNETPEDALS_BUFSIZE);
    }
    else
    {
        slogd("no handle");
    }

    return res;
}

int simnetpedals_free(USBGenericHapticDevice* usbhapticdevice)
{
    int res = 0;

    hid_close(usbhapticdevice->handle);
    res = hid_exit();

    return res;
}

int simnetpedals_init(USBGenericHapticDevice* usbhapticdevice)
{
    slogi("initializing simnet pedals...");

    int res = 0;

    res = hid_init();

    usbhapticdevice->handle = hid_open(0xcafe, 0xa301, NULL);

    if (!usbhapticdevice->handle)
    {
        sloge("Could not find attached Simnet Pedals");
        res = hid_exit();
        return 1;
    }
    slogd("Found Simnet Pedals handle %i...", usbhapticdevice->handle);
    return res;
}
