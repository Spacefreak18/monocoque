#ifndef _CSLELITEV3_H
#define _CSLELITEV3_H

#include "../wheeldevice.h"
#include "../simdevice.h"

int cslelitev3_update(USBDevice* usbdevice, int effecttype, int play);
int cslelitev3_init(USBDevice* usbdevice);
int cslelitev3_free(USBDevice* usbdevice);

#endif
