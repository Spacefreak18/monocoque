#ifdef USE_PULSEAUDIO

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>



#include "usb_generic_shaker.h"
#include "../sounddevice.h"

#define FORMAT PA_SAMPLE_S16LE
#define SAMPLE_RATE   (44100)
#define AMPLITUDE .5
#define DURATION 1.0

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

void gear_sound_stream(pa_stream *s, size_t length, void *userdata) {

    SoundData* data = (SoundData*)userdata;
    size_t num_samples = length / sizeof(int16_t);
    int16_t buffer[num_samples];


    for (size_t i = 0; i < num_samples; i++) {
        static double t = 0.0;
        double sample = 0;
        if (data->frequency>0)
        {
            sample = AMPLITUDE * 32767.0 * sin(2.0 * M_PI * data->curr_frequency * data->curr_duration);
        }


        buffer[i] = (int16_t)sample;

        data->curr_duration += 1.0 / SAMPLE_RATE;
        if (data->curr_duration >= data->duration) {
            data->curr_duration = 0.0;
            data->curr_frequency = 0;
        }
    }
    pa_stream_write(s, buffer, length, NULL, 0LL, PA_SEEK_RELATIVE);
}

double get_closest_frequency(size_t num_samples, double input_frequency) {

    double samples_per_cycle = SAMPLE_RATE / input_frequency;

    double adjusted_frequency = SAMPLE_RATE / (num_samples / floor(num_samples/samples_per_cycle) );

    return adjusted_frequency;
}

void engine_sound_stream(pa_stream *s, size_t length, void *userdata) {

    SoundData* data = (SoundData*)userdata;
    double freq = data->curr_frequency;

    size_t num_samples = length / sizeof(int16_t);
    freq = get_closest_frequency( num_samples, freq );
    //size_t num_samples = (size_t) (DURATION * SAMPLE_RATE);
    int16_t buffer[num_samples];

    //double t = 0;
    for (size_t i = 0; i < num_samples; i++) {
        double t = data->phase;

        double sample = AMPLITUDE * 32767.0 * sin( 2.0 * M_PI * freq * t );

        buffer[i] = (int16_t)sample;

        t += 1.0 / SAMPLE_RATE;
        if (t >= SAMPLE_RATE) {
            t = 0;
        }
        data->phase = t;
    }
    data->phase = round(data->phase);

    pa_stream_write(s, buffer, length, NULL, 0LL, PA_SEEK_RELATIVE);
}



void stream_success_cb(pa_stream *stream, int success, void *userdata) {
    return;
}

void stream_state_cb(pa_stream *s, void *mainloop) {
    pa_threaded_mainloop_signal(mainloop, 0);
}

int usb_generic_shaker_free(SoundDevice* sounddevice, pa_threaded_mainloop* mainloop)
{
    pa_threaded_mainloop_lock(mainloop);
    int err = 0;
    if (sounddevice->stream)
    {
        pa_stream_disconnect(sounddevice->stream);
        pa_stream_unref(sounddevice->stream);
        // why is this wrong
        //pa_xfree(sounddevice->stream);
    }
    pa_threaded_mainloop_unlock(mainloop);
    return err;
}

int usb_generic_shaker_init(SoundDevice* sounddevice, pa_threaded_mainloop* mainloop, pa_context* context, const char* devname, int volume, int pan, int channels, const char* streamname)
{
    pa_threaded_mainloop_lock(mainloop);
    pa_stream *stream;

    // Create a playback stream
    pa_sample_spec sample_specifications;
    sample_specifications.format = FORMAT;
    sample_specifications.rate = SAMPLE_RATE;
    sample_specifications.channels = channels;


    pa_channel_map channel_map;
    pa_channel_map_init_auto(&channel_map, channels, PA_CHANNEL_MAP_DEFAULT);
    //pa_channel_map_init_stereo(&channel_map);

    // not sure about what to do here
    if(channels == 2)
    {
        pa_channel_map_parse(&channel_map, "front-left,front-right");
    }
    if(channels == 4)
    {
        pa_channel_map_parse(&channel_map, "front-left,front-right,rear-left,rear-right");
    }
    if(channels == 6)
    {
        pa_channel_map_parse(&channel_map, "front-left,front-right,front-center,lfe,rear-left,rear-right");
    }
    if(channels == 8)
    {
        pa_channel_map_parse(&channel_map, "front-left,front-right,front-center,lfe,rear-left,rear-right,side-left,side-right");
    }

    stream = pa_stream_new(context, streamname, &sample_specifications, &channel_map);
    pa_stream_set_state_callback(stream, stream_state_cb, mainloop);

    if (sounddevice->m.hapticeffect.effecttype == EFFECT_GEARSHIFT)
    {
        pa_stream_set_write_callback(stream, gear_sound_stream, &sounddevice->sounddata);
    }
    else
    {
        pa_stream_set_write_callback(stream, engine_sound_stream, &sounddevice->sounddata);
    }

    // recommended settings, i.e. server uses sensible values
    pa_buffer_attr buffer_attr;
    buffer_attr.maxlength = (uint32_t) 32767;
    buffer_attr.tlength = (uint32_t) -1;
    buffer_attr.prebuf = (uint32_t) -1;
    buffer_attr.minreq = (uint32_t) -1;

    pa_cvolume cv;
    pa_cvolume_mute(&cv, channels);

    pa_volume_t channel_volume = PA_CLAMP_VOLUME((pa_volume_t)((volume/100.d)*PA_VOLUME_NORM));

    pa_stream_flags_t stream_flags;
    stream_flags = PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_ADJUST_LATENCY | PA_STREAM_START_CORKED;


    // for now i'm only supporting playing on one specified channel which is the concept you should build your setups around
    cv.values[pan] = channel_volume;

    assert(pa_stream_connect_playback(stream, devname, &buffer_attr, stream_flags, &cv, NULL) == 0);

    // Wait for the stream to be ready
    for(;;) {
        pa_stream_state_t stream_state = pa_stream_get_state(stream);
        assert(PA_STREAM_IS_GOOD(stream_state));
    if (stream_state == PA_STREAM_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }



    // Uncork the stream so it will start playing
    pa_stream_cork(stream, 0, stream_success_cb, mainloop);

    sounddevice->stream = stream;
    pa_threaded_mainloop_unlock(mainloop);
    return 0;
}

#endif
