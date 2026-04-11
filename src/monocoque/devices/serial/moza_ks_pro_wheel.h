#ifndef _MOZA_KS_PRO_WHEEL_H
#define _MOZA_KS_PRO_WHEEL_H

#include "../serialdevice.h"
#include "../simdevice.h"

int moza_ks_pro_wheel_update(SerialDevice* serialdevice, SimData* simData);
int moza_ks_pro_wheel_init(SerialDevice* serialdevice, const char* portdev);

#endif
