#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

#include "usbhapticdevice.h"

#include "../slog/slog.h"



int cslelitev3_update(USBGenericHapticDevice* usbhapticdevice, int value)
{

    int res = 0;

    fprintf(usbhapticdevice->handle, "%i\n", value);
    fflush(usbhapticdevice->handle);

    return res;
}

int cslelitev3_free(USBGenericHapticDevice* usbhapticdevice)
{
    int res = 0;

    free(usbhapticdevice->dev);

    fflush(usbhapticdevice->handle);
    fclose(usbhapticdevice->handle);

    return res;
}

int cslelitev3_init(USBGenericHapticDevice* usbhapticdevice)
{
    slogi("initializing CSL Elite V3 Pedals...");

    int res = 0;


    glob_t globlist;
    int i = 0;
    if (glob("/sys/module/hid_fanatec/drivers/hid:fanatec/0003:0EB7:0005.*/rumble", GLOB_PERIOD, NULL, &globlist) == GLOB_NOSPACE || glob("/sys/module/hid_fanatec/drivers/hid:fanatec/0003:0EB7:0005.*/rumble", GLOB_PERIOD, NULL, &globlist) == GLOB_NOMATCH)
    {
        res = 1;
    }
    if (glob("/sys/module/hid_fanatec/drivers/hid:fanatec/0003:0EB7:0005.*/rumble", GLOB_PERIOD, NULL, &globlist) == GLOB_ABORTED)
    {
        res = 2;
    }
    if (res == 0)
    {
        while (globlist.gl_pathv[i])
        {
            if (i == 0)
            {
                usbhapticdevice->dev = strdup(globlist.gl_pathv[i]);
            }
            i++;
        }
    }
    globfree(&globlist);

    if (res == 1) {
        sloge("Could not find attach Club Sport Elite V3 Pedals");
        return res;
    }
    if (res == 2) {
        sloge("Permissions issue finding Club Sport Elite V3 Pedals");
        return res;
    }

    usbhapticdevice->handle = fopen(usbhapticdevice->dev, "w");

    if (!usbhapticdevice->handle)
    {
        sloge("Could not open pedal device...");
        return res;
    }

    slogd("CSL Elite V3 Pedals Successfully initialized...");

    return res;
}
