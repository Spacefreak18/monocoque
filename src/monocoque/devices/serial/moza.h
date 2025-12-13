#ifndef _MOZA_H
#define _MOZA_H

#include "../serialdevice.h"
#include "../simdevice.h"

int moza_update(SerialDevice* serialdevice, unsigned short rpm, unsigned short maxrpm);
int moza_init(SerialDevice* serialdevice, const char* portdev);
unsigned char moza_checksum(unsigned char *data, unsigned short size);

#endif
