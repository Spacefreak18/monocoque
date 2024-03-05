#ifndef _SHIFTLIGHTSDATA_H
#define _SHIFTLIGHTSDATA_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    char color_1_red;
    char color_1_green;
    char color_1_blue;
    char color_2_red;
    char color_2_green;
    char color_2_blue;
    char color_3_red;
    char color_3_green;
    char color_3_blue;
    uint32_t maxrpm;
    uint32_t rpm;
    uint32_t pulses;
}
ShiftLightsData;


#endif
