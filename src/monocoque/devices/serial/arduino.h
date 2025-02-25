#ifndef _ARDUINO_H
#define _ARDUINO_H

#include "../simdevice.h"
#include "../serialdevice.h"


int arduino_simled_update(SimDevice* this, SimData* simdata);
int arduino_update(SerialDevice* serialdevice, void* data, size_t size);
int arduino_customled_init(SerialDevice* serialdevice, const char* portdev, const char* luafile);
int arduino_customled_update(SimDevice* this, SimData* simdata);
int arduino_init(SerialDevice* serialdevice,  const char* portdev);
int arduino_free(SerialDevice* serialdevice);
int arduino_customled_free(SerialDevice* serialdevice, bool lua);

#endif
