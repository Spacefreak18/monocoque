#ifndef _REVBURNER_H
#define _REVBURNER_H

#include "../simdevice.h"

int revburner_update(USBDevice* tachdevice, int pulses);
int revburner_init(USBDevice* tachdevice);
int revburner_free(USBDevice* tachdevice);

#endif
