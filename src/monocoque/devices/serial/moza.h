#ifndef _MOZA_H
#define _MOZA_H

#include "../serialdevice.h"
#include "../simdevice.h"

int moza_update(SerialDevice* serialdevice, unsigned short maxrpm, unsigned short rpm);
int moza_init(SerialDevice* serialdevice, const char* portdev);
int moza_free(SerialDevice* serialdevice);
int moza_serial_check(enum sp_return result);


#endif
