#ifndef _TACHDEVICE_H
#define _TACHDEVICE_H


#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"

//typedef int (*tachdev_update)(int revs);

typedef enum
{
    TACHDEV_UNKNOWN       = 0,
    TACHDEV_REVBURNER     = 1
}
TachType;

typedef struct
{
    int id;
    TachType type;
    bool use_pulses;
    TachometerSettings tachsettings;
}
TachDevice;



#endif
