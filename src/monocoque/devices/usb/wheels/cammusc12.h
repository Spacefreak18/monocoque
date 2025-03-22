#ifndef _CAMMUSC12_H
#define _CAMMUSC12_H

#include "../../wheeldevice.h"

#include "../../simdevice.h"

int cammusc12_update(USBDevice* wheeldevice, int maxrpm, int rpm, int gear, int velocity);
int cammusc12_init(USBDevice* wheeldevice, const char* luafile);
int cammusc12_free(USBDevice* wheeldevice);

#endif
