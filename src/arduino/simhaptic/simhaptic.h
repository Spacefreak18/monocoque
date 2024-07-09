#ifndef _SIMHAPTICDATA_H
#define _SIMHAPTICDATA_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    int motor;
    float effect;
    float power;
}
SimHapticData;


#endif
