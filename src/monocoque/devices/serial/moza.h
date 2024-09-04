#ifndef _MOZA_H
#define _MOZA_H

#include "../serialdevice.h"

int moza_update(SerialDevice* serialdevice, unsigned short maxrpm, unsigned short rpm);
int moza_init(SerialDevice* serialdevice, const char* portdev);
int moza_free(SerialDevice* serialdevice);


#endif
