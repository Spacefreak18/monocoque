#ifndef _SIMNETPEDALS_H
#define _SIMNETPEDALS_H

int simnetpedals_update(USBGenericHapticDevice* usbhapticdevice, int effecttype, int play);
int simnetpedals_init(USBGenericHapticDevice* usbhapticdevice);
int simnetpedals_free(USBGenericHapticDevice* usbhapticdevice);

#endif
