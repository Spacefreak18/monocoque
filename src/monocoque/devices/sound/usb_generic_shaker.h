#ifndef _USB_GENERIC_SHAKER_H
#define _USB_GENERIC_SHAKER_H

#include "../sounddevice.h"

int usb_generic_shaker_init(SoundDevice* sounddevice);
int usb_generic_shaker_free(SoundDevice* sounddevice);

#endif
