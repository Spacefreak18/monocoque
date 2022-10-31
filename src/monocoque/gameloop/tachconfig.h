#ifndef _TACHCONFIG_H
#define _TACHCONFIG_H

#include "../devices/simdevice.h"
#include "../simulatorapi/simdata.h"

int config_tachometer(int max_revs, int granularity, const char* save_file, SimDevice* simdevice, SimData* simdata);

#endif
