#ifndef _SIMHAPTICDATA_H
#define _SIMHAPTICDATA_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t motor1;
    uint8_t effect1;
    uint8_t motor2;
    uint8_t effect2;
    uint8_t motor3;
    uint8_t effect3;
    uint8_t motor4;
    uint8_t effect4;
}
SimHapticData;


#endif
