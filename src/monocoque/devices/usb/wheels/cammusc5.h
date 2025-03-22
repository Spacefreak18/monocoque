#ifndef _CAMMUSC5_H
#define _CAMMUSC5_H

#include "../../simdevice.h"

int cammusc5_update(USBDevice* wheeldevice, int maxrpm, int rpm, int gear, int velocity);
int cammusc5_init(USBDevice* wheeldevice);
int cammusc5_free(USBDevice* wheeldevice);

#endif
