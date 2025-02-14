#ifndef _SERIALDEVICE_H
#define _SERIALDEVICE_H

#include <libserialport.h>
#include "wheeldevice.h"

typedef enum
{
    ARDUINODEV__SHIFTLIGHTS   = 0,
    ARDUINODEV__SIMWIND       = 1,
    ARDUINODEV__HAPTIC        = 2,
    SERIALDEV__MOZAR5         = 3,
    ARDUINODEV__SIMLED        = 4
}
SerialDeviceType;

#endif
