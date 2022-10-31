#ifndef _SIMDATA_H
#define _SIMDATA_H

#include <stdint.h>

typedef struct
{
    uint32_t velocity;
    uint32_t rpms;
    uint32_t gear;
    uint32_t pulses;
    uint32_t maxrpm;
    uint32_t altitude;
}
SimData;

#endif
