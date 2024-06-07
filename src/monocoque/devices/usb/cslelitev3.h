#ifndef _CSLELITEV3_H
#define _CSLELITEV3_H

int cslelitev3_update(USBGenericHapticDevice* usbhapticdevice, int effecttype, int play);
int cslelitev3_init(USBGenericHapticDevice* usbhapticdevice);
int cslelitev3_free(USBGenericHapticDevice* usbhapticdevice);

#endif
