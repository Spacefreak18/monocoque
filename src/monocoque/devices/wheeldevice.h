#ifndef _WHEELDEVICE_H
#define _WHEELDEVICE_H

#include "../helper/confighelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"

//typedef int (*tachdev_update)(int revs);

typedef enum
{
    WHEELDEV_UNKNOWN       = 0,
    WHEELDEV_CAMMUSC5      = 1,
    WHEELDEV_CAMMUSC12     = 2,
    WHEELDEV_MOZAR5        = 3
}
WheelType;

typedef struct
{
    int id;
    WheelType type;
    bool useLua;
    char* port;
}
WheelDevice;


#endif
