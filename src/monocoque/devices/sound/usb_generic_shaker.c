#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "portaudio.h"

#include "usb_generic_shaker.h"
#include "../sounddevice.h"

#define SAMPLE_RATE   (48000)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif



int patestCallback(const void*                     inputBuffer,
                   void*                           outputBuffer,
                   unsigned long                   framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags           statusFlags,
                   void*                           userData)
{
    PATestData* data = (PATestData*)userData;
    float* out = (float*)outputBuffer;
    memset(out, 0, framesPerBuffer * 2 * sizeof(float));
    unsigned int i;
    unsigned int n;
    n = data->n;
    (void) inputBuffer; /* Prevent unused argument warning. */

    for( i=0; i<framesPerBuffer; i++,n++ )
    {
        float v = 0;
        v = data->amp * sin (2 * M_PI * ((float) n) / (float) SAMPLE_RATE);

        if ( data->gear_sound_data > 0 )
        {
            if (n>=1764)
            {
                n=0;
            }
        }
        else
        {
            if (n>=data->table_size)
            {
                n=0;
            }
        }


        if ( data->gear_sound_data > 0 )
        {
            // right channel only?
            // i have my butt hooked up to right channel... make this configurable?
            *out++ = v;
        }
        else
        {
            *out++ = v;
            *out++ = v;
        }
    }

    data->n=n;
    return 0;
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

    err = Pa_OpenStream( &sounddevice->stream,
                         NULL,              /* No input. */
                         &sounddevice->outputParameters, /* As above. */
                         SAMPLE_RATE,
                         440,               /* Frames per buffer. */
                         paClipOff,         /* No out of range samples expected. */
                         patestCallback,
                         &sounddevice->sounddata );
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
    //fprintf( stderr, "An error occured while using the portaudio stream\n" );
    //fprintf( stderr, "Error number: %d\n", err );
    //fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
