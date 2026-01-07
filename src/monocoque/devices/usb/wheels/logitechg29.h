#ifndef _LOGITECHG29_H
#define _LOGITECHG29_H

#include "../../simdevice.h"

int logitechg29_update(USBDevice* wheeldevice, int maxrpm, int rpm, int gear, int velocity);
int logitechg29_init(USBDevice* wheeldevice);
int logitechg29_free(USBDevice* wheeldevice);

#endif
