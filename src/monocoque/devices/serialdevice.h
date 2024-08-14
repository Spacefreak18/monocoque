#ifndef _SERIALDEVICE_H
#define _SERIALDEVICE_H

#include <libserialport.h>

typedef enum
{
    ARDUINODEV__SHIFTLIGHTS   = 0,
    ARDUINODEV__SIMWIND       = 1,
    ARDUINODEV__HAPTIC        = 2,
    MOZADEV                   = 3,
}
SerialDeviceType;

#endif
