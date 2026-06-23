#ifndef _SIMAGICP1000_H
#define _SIMAGICP1000_H

#include "../wheeldevice.h"
#include "../simdevice.h"

int simagicp1000_update(USBDevice* usbdevice, int effecttype, int play);
int simagicp1000_init(USBDevice* usbdevice);
int simagicp1000_free(USBDevice* usbdevice);

#endif
