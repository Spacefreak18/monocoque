#ifndef _ARDUINO_H
#define _ARDUINO_H

#include "../serialdevice.h"

int arduino_update(SerialDevice* serialdevice, SimData* simdata);
int arduino_init(SerialDevice* serialdevice);
int arduino_free(SerialDevice* serialdevice);
int check(enum sp_return result);

#endif
