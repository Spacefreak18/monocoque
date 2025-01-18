#ifndef _SIMAGICP1000_H
#define _SIMAGICP1000_H

int simagicp1000_update(USBGenericHapticDevice* usbhapticdevice, int effecttype, int play);
int simagicp1000_init(USBGenericHapticDevice* usbhapticdevice);
int simagicp1000_free(USBGenericHapticDevice* usbhapticdevice);

#endif
