#ifndef _CAMMUSC12_H
#define _CAMMUSC12_H

#include "../../wheeldevice.h"

int cammusc12_update(WheelDevice* wheeldevice, int maxrpm, int rpm, int gear, int velocity);
int cammusc12_init(WheelDevice* wheeldevice);
int cammusc12_free(WheelDevice* wheeldevice);

#endif
