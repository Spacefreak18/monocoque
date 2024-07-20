#ifndef _USB_GENERIC_SHAKER_H
#define _USB_GENERIC_SHAKER_H

#include "../simdevice.h"

#ifdef USE_PULSEAUDIO
int usb_generic_shaker_init(SoundDevice* sounddevice, pa_threaded_mainloop* mainloop, pa_context* context, const char* devname, int volume, int pan, int channels, const char* streamname);
#else
int usb_generic_shaker_init(SoundDevice* sounddevice);
#endif
int usb_generic_shaker_free(SoundDevice* sounddevice);

#endif
