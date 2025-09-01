#ifndef _ARDUINO_H
#define _ARDUINO_H

#include "../simdevice.h"
#include "../serialdevice.h"


int arduino_simled_update(SerialDevice* serialdevice, SimData* simdata);
int arduino_update(SerialDevice* serialdevice, void* data, size_t size);
int arduino_custom_init(SerialDevice* serialdevice, const char* portdev, const char* luafile, bool useleds);
int arduino_customled_update(SerialDevice* serialdevice, SimData* simdata);
int arduino_custom_update(SerialDevice* serialdevice, SimData* simdata);
int arduino_init(SerialDevice* serialdevice,  const char* portdev);
int arduino_free(SerialDevice* serialdevice);
int arduino_customled_free(SerialDevice* serialdevice, bool lua);

#endif
