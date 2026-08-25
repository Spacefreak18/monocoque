
/** @file patest_longsine.c
    @ingroup test_src
    @brief Play a sine wave until ENTER hit.
    @author Phil Burk  http://www.softsynth.com
*/
/*
 * $Id: patest_longsine.c 1097 2006-08-26 08:27:53Z rossb $
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include "portaudio.h"

#define SAMPLE_RATE   (44100)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif


#define MAX_TABLE_SIZE   (6000)
//#define TABLE_SIZE   (3200)
//#define HALF_TABLE   (1600)
typedef struct
{
    float sine[MAX_TABLE_SIZE];
    float pitch;
    int left_phase;
    int right_phase;
    int n;
    int table_size;
    int amp;
}

paTestData;

int x = 0;
int y = 0;
long n = 0;
//float note = 256;
float note = 60;
//float note = 161.626;
/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback(const void*                     inputBuffer,
                          void*                           outputBuffer,
                          unsigned long                   framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags           statusFlags,
                          void*                           userData)
{
    paTestData* data = (paTestData*)userData;
    float* out = (float*)outputBuffer;
    unsigned int i;
    unsigned int n;
    n = data->n;
    (void) inputBuffer; /* Prevent unused argument warning. */
    for( i=0; i<framesPerBuffer; i++,n++ )
    {


        //data->sine[i] = (float) sin( ((double)i/(double) MAX_TABLE_SIZE) * ( ( M_PI * 2. ) ) );
        //data->sine[n] = sin ( 2 * M_PI * ((float) n / (float) MAX_TABLE_SIZE) );
        //data->right_phase = data->left_phase;
        //data->left_phase = n;
        //data->right_phase = n;
        float v = data->amp * sin (2 * M_PI * ((float) n) / (float) SAMPLE_RATE);
        //*out++ = data->sine[data->left_phase];
        //*out++ = data->sine[data->right_phase];

        if (n>=data->table_size)
        {
            n=0;
        }

        *out++ = v;
        *out++ = v;

        data->left_phase += 1;
        data->right_phase += 1;
        /*
        float v = 800.81;
        if (i>=512)
        {
            v = -v;
        }
        *out++ = v;
        *out++ = v;
        */
        //if( data->left_phase >= MAX_TABLE_SIZE ) data->left_phase -= MAX_TABLE_SIZE;
        //if( data->right_phase >= MAX_TABLE_SIZE ) data->right_phase -= MAX_TABLE_SIZE;
        if( data->left_phase >= data->table_size )
        {
            data->left_phase -= data->table_size;
        }
        if( data->right_phase >= data->table_size )
        {
            data->right_phase -= data->table_size;
        }
    }
    data->n=n;
    return 0;
}

void generatesamples(int table_size, int half_table, int datum, paTestData* data)
{
    datum = 1;
    int j;
    for( j=data->n; j<MAX_TABLE_SIZE; j++)
    {
        for( int i=0; i<data->table_size; i++,j++ )
        {
            //data.sine[i] = (float) amp * sin( ((double)i/(double) TABLE_SIZE) * ( ( M_PI * 2. * freq) ) );
            data->sine[j] = (float) datum * sin( ((double)i/(double) data->table_size) * ( ( M_PI * 2. ) ) );
            /*
            data->sine[i] = datum;
            if (i>=half_table) {
                data->sine[i] = -datum;
            }
            */
            data->n=j;
        }
    }
}
/*******************************************************************/
int main(void);
int main(void)
{
    PaStreamParameters outputParameters;
    PaStream* stream;
    PaError err;
    paTestData data;
    int i;
    printf("PortAudio Test: output sine wave.\n");

    float amp = ((float)32);
    amp = ((float)32);
    //float freq = ((float) freq1/(SAMPLE_RATE/TABLE_SIZE));
    int freq = 32;
    // 500 rpms
    int table_size = 5292;
    int half_table = 2646;
    table_size = 200;
    half_table = 50;
    data.pitch = 1;
    data.pitch = 261.626;
    data.amp = 2;
    //data.pitch = 523.25;
    //table_size = 2646;
    //half_table = 1323;
    //table_size = 3200;
    //half_table = 1600;
    /* initialise sinusoidal wavetable */
    data.table_size=5292;
    //generatesamples(table_size,half_table,5,&data);
    data.left_phase = data.right_phase = 0;

    err = Pa_Initialize();
    if( err != paNoError )
    {
        goto error;
    }

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    outputParameters.channelCount = 2;                     /* stereo output */
    outputParameters.sampleFormat = paFloat32;             /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream( &stream,
                         NULL,              /* No input. */
                         &outputParameters, /* As above. */
                         SAMPLE_RATE,
                         440,               /* Frames per buffer. */
                         paClipOff,         /* No out of range samples expected. */
                         patestCallback,
                         &data );
    if( err != paNoError )
    {
        goto error;
    }

    fprintf(stdout,"Revving to 500 rpm...\n");

    err = Pa_StartStream( stream );
    if( err != paNoError )
    {
        goto error;
    }

    //printf("Hit ENTER to stop program.\n");
    //getchar();

    sleep(10);

    table_size = 2646;
    half_table = 1323;
    table_size = 100;
    data.table_size = 2646;
    half_table = 100;

    fprintf(stdout,"Revving to 1000 rpm...\n");
    //generatesamples(table_size,half_table,5,&data);
    data.pitch = 523.25;

    sleep(10);

    table_size = 1764;
    half_table = 882;
    table_size = 400;
    half_table = 200;
    data.table_size = 1764;

    fprintf(stdout,"Revving to 1500 rpm...\n");
    //generatesamples(table_size,half_table,5,&data);

    sleep(5);

    table_size = 1323;
    half_table = 661;
    data.table_size = 1323;

    fprintf(stdout,"Revving to 2000 rpm...\n");
    //generatesamples(table_size,half_table,5,&data);

    sleep(5);
    err = Pa_CloseStream( stream );
    if( err != paNoError )
    {
        goto error;
    }
    Pa_Terminate();

    printf("Test finished.\n");
    return err;

error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}
