#ifndef _MOZA_NEW_H
#define _MOZA_NEW_H

#include "../serialdevice.h"
#include "../simdevice.h"

int moza_new_update(SerialDevice* serialdevice, SimData* simData);
int moza_new_init(SerialDevice* serialdevice, const char* portdev);

#endif
