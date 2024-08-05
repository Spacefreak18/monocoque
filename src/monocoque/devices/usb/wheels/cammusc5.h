#ifndef _CAMMUSC5_H
#define _CAMMUSC5_H

#include "../../wheeldevice.h"

int cammusc5_update(WheelDevice* wheeldevice, int maxrpm, int rpm, int gear, int velocity);
int cammusc5_init(WheelDevice* wheeldevice);
int cammusc5_free(WheelDevice* wheeldevice);

#endif
