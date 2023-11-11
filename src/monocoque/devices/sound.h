#ifndef _SOUND_H
#define _SOUND_H

#include <pulse/pulseaudio.h>

extern pa_threaded_mainloop* mainloop;
extern pa_context* context;


int setupsound();
int freesound();

#endif
