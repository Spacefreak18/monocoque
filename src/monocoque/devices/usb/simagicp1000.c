#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hidapi/hidapi.h>

#include "usbhapticdevice.h"

#include "../../helper/confighelper.h"
#include "../slog/slog.h"

#define SIMAGIC_VENDOR_ID          0x0483
#define SIMAGIC_PRODUCT_ID_P1000   0x0525

const size_t SIMAGIC_P1000_BUFSIZE = 49;
const char* SIMAGICP1000DEVSTRING = "SIMAGIC P1000 Pedals";

int senddevreport(USBGenericHapticDevice* usbhapticdevice, unsigned char* buf, size_t bufsize)
{
    int res = 0;
    if (usbhapticdevice->handle)
    {
        res = hid_send_feature_report(usbhapticdevice->handle, buf, SIMAGIC_P1000_BUFSIZE);
        slogd("sent %i bytes to %s", bufsize, SIMAGICP1000DEVSTRING);
        slogt("sent bytes %02x %02x %02x %02x %02x %02x %02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
    }
    else
    {
        slogd("no handle");
    }
    return res != bufsize;
}

int simagicp1000_update(USBGenericHapticDevice* usbhapticdevice, int effecttype, int play)
{

    int res = 0;
    int value = 0;

    unsigned char bytes[SIMAGIC_P1000_BUFSIZE];
    for (int x = 0; x < SIMAGIC_P1000_BUFSIZE; x++)
    {
        bytes[x] = 0x00;
    }

    if (play > 0)
    {
        bytes[0] = 241;
        bytes[1] = 0xEC;
        // we can add config settings for these values
        bytes[3] = 0x01;
        bytes[4] = 0x0A;
        bytes[5] = 0xFF;
        switch (effecttype)
        {
            case (EFFECT_TYRESLIP):
                bytes[2] = 0x02;
                senddevreport(usbhapticdevice, bytes, SIMAGIC_P1000_BUFSIZE);
                break;
            case (EFFECT_TYRELOCK):
                bytes[2] = 0x01;
                senddevreport(usbhapticdevice, bytes, SIMAGIC_P1000_BUFSIZE);
                break;
            case (EFFECT_ABSBRAKES):
                bytes[2] = 0x02;
                senddevreport(usbhapticdevice, bytes, SIMAGIC_P1000_BUFSIZE);
                bytes[2] = 0x01;
                senddevreport(usbhapticdevice, bytes, SIMAGIC_P1000_BUFSIZE);
                break;
        }
    }
    else
    {
        bytes[0] = 241;
        bytes[2] = 0x01;
        senddevreport(usbhapticdevice, bytes, SIMAGIC_P1000_BUFSIZE);
        bytes[2] = 0x02;
        senddevreport(usbhapticdevice, bytes, SIMAGIC_P1000_BUFSIZE);
    }

    return res;
}

int simagicp1000_free(USBGenericHapticDevice* usbhapticdevice)
{
    int res = 0;

    hid_close(usbhapticdevice->handle);
    res = hid_exit();

    return res;

    return res;
}

int simagicp1000_init(USBGenericHapticDevice* usbhapticdevice)
{
    slogi("initializing %s...", SIMAGICP1000DEVSTRING);

    int res = 0;

    res = hid_init();

    usbhapticdevice->handle = hid_open(SIMAGIC_VENDOR_ID, SIMAGIC_PRODUCT_ID_P1000, NULL);

    if (!usbhapticdevice->handle)
    {
        sloge("Could not find attached %s", SIMAGICP1000DEVSTRING);
        res = hid_exit();
        return 1;
    }
    else
    {
        unsigned char bytes[SIMAGIC_P1000_BUFSIZE];
        for (int x = 0; x < SIMAGIC_P1000_BUFSIZE; x++)
        {
            bytes[x] = 0x00;
        }
        bytes[0] = 241;
        bytes[1] = 0xF1;
        bytes[2] = 0x17;
        bytes[3] = 0x00;
        bytes[4] = 0x00;
        bytes[5] = 0x00;
        bytes[6] = 0x01;
        bytes[7] = 0x02;
        res = senddevreport(usbhapticdevice, bytes, SIMAGIC_P1000_BUFSIZE);
        slogd("Initialization returned %i", res);
        if(res != 0)
        {
            slogw("Problem with initialization of %s", SIMAGICP1000DEVSTRING);
        }
    }

    slogd("Found %s...", SIMAGICP1000DEVSTRING);
    return res;
}
