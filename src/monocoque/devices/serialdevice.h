#ifndef _SERIALDEVICE_H
#define _SERIALDEVICE_H

#include <libserialport.h>
#include "../helper/parameters.h"
#include "../simulatorapi/simdata.h"

typedef enum
{
    SERIALDEV_UNKNOWN       = 0,
    SERIALDEV_ARDUINO       = 1
}
SerialType;

typedef struct
{
    int id;
    SerialType type;
    struct sp_port* port;
}
SerialDevice;

int serialdev_update(SerialDevice* serialdevice, SimData* simdata);
int serialdev_init(SerialDevice* serialdevice);
int serialdev_free(SerialDevice* serialdevice);

#endif
