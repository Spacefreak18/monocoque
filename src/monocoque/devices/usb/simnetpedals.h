#ifndef _SIMNETPEDALS_H
#define _SIMNETPEDALS_H

#include "../wheeldevice.h"
#include "../simdevice.h"

int simnetpedals_update(USBDevice* usbdevice, int effecttype, int play, int motorposition, int frequency, int amplitude);
int simnetpedals_init(USBDevice* usbdevice);
int simnetpedals_free(USBDevice* usbdevice);

#endif
