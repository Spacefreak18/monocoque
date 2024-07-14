#ifndef _SIMHAPTICDATA_H
#define _SIMHAPTICDATA_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t motor1;
    uint32_t effect1;
    uint32_t motor2;
    uint32_t effect2;
    uint32_t motor3;
    uint32_t effect3;
    uint32_t motor4;
    uint32_t effect4;
}
SimHapticData;


#endif
