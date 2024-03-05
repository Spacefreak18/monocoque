#ifndef _ARDUINO_H
#define _ARDUINO_H

#include "../simdevice.h"
#include "../serialdevice.h"


int arduino_update(SerialDevice* serialdevice, void* data, size_t size);
int arduino_init(SerialDevice* serialdevice,  const char* portdev);
int arduino_free(SerialDevice* serialdevice);
int check(enum sp_return result);

#endif
