#ifndef USE_PULSEAUDIO

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "portaudio.h"

#include "usb_generic_shaker.h"
#include "../sounddevice.h"

#define SAMPLE_RATE   (48000)

#define DURATION 4.0
#define AMPLITUDE .5

#ifndef M_PI
#define M_PI  (3.14159265)
#endif


int patestCallbackEngineRPM(const void*                     inputBuffer,
                   void*                           outputBuffer,
                   unsigned long                   framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags           statusFlags,
                   void*                           userData)
{
    SoundData* data = (SoundData*)userData;
    float* out = (float*)outputBuffer;
    memset(out, 0, framesPerBuffer * 2 * sizeof(float));
    unsigned int i;
    unsigned int n;
    //n = data->n;
    (void) inputBuffer; /* Prevent unused argument warning. */

    for( i=0; i<framesPerBuffer; i++,n++ )
    {
        static double t = 0.0;
        double sample = AMPLITUDE * 32767.0 * sin(2.0 * M_PI * data->curr_frequency * t);

        t += 1.0 / SAMPLE_RATE;
        if (t >= DURATION) {
            t = 0.0;
        }
        *out++ = sample;
        *out++ = sample;
    }

    return 0;
}


int patestCallbackGearShift(const void*                     inputBuffer,
                   void*                           outputBuffer,
                   unsigned long                   framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags           statusFlags,
                   void*                           userData)
{
    SoundData* data = (SoundData*)userData;
    float* out = (float*)outputBuffer;
    memset(out, 0, framesPerBuffer * 2 * sizeof(float));
    unsigned int i;
    unsigned int n;
    //n = data->n;
    (void) inputBuffer; /* Prevent unused argument warning. */

    for( i=0; i<framesPerBuffer; i++,n++ )
    {
        static double t = 0.0;
        double sample = 0;
        if (data->frequency>0)
        {
            sample = AMPLITUDE * 32767.0 * sin(2.0 * M_PI * data->curr_frequency * data->curr_duration);
        }


        data->curr_duration += 1.0 / SAMPLE_RATE;
        if (data->curr_duration >= data->duration) {
            data->curr_duration = 0.0;
            data->curr_frequency = 0.0;
        }

        *out++ = sample;
        *out++ = sample;
    }
}

int usb_generic_shaker_free(SoundDevice* sounddevice)
{
    int err = 0;
    err = Pa_CloseStream( sounddevice->stream );
    if( err != paNoError )
    {
        err = Pa_Terminate();
    }
    return err;
}

int usb_generic_shaker_init(SoundDevice* sounddevice)
{
    PaError err;
    err = paNoError;

    err = Pa_Initialize();
    if( err != paNoError )
    {
        goto error;
    }

    sounddevice->outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    sounddevice->outputParameters.channelCount = 2;                     /* stereo output */
    sounddevice->outputParameters.sampleFormat = paFloat32;             /* 32 bit floating point output */
    sounddevice->outputParameters.suggestedLatency = Pa_GetDeviceInfo( sounddevice->outputParameters.device )->defaultLowOutputLatency;
    sounddevice->outputParameters.hostApiSpecificStreamInfo = NULL;

    if (sounddevice->m.hapticeffect.effecttype == EFFECT_GEARSHIFT)
    {
        err = Pa_OpenStream( &sounddevice->stream,
                             NULL,              /* No input. */
                             &sounddevice->outputParameters, /* As above. */
                             SAMPLE_RATE,
                             440,               /* Frames per buffer. */
                             paClipOff,         /* No out of range samples expected. */
                             patestCallbackGearShift,
                             &sounddevice->sounddata );
    }
    else
    {
        err = Pa_OpenStream( &sounddevice->stream,
                             NULL,              /* No input. */
                             &sounddevice->outputParameters, /* As above. */
                             SAMPLE_RATE,
                             440,               /* Frames per buffer. */
                             paClipOff,         /* No out of range samples expected. */
                             patestCallbackEngineRPM,
                             &sounddevice->sounddata );
    }
    if( err != paNoError )
    {
        goto error;
    }


    err = Pa_StartStream( sounddevice->stream );
    if( err != paNoError )
    {
        goto error;
    }

    return err;

error:
    Pa_Terminate();
    return err;
}

#endif
