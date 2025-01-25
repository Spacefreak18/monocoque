#include "sound.h"

#include "../slog/slog.h"

#ifdef USE_PULSEAUDIO

pa_threaded_mainloop* mainloop;
pa_context* context;

void context_state_cb(pa_context* context, void* mainloop) {
    pa_threaded_mainloop_signal(mainloop, 0);
}

int setupsound()
{



    pa_mainloop_api *mainloop_api;

    slogi("connecting pulseaudio...");
    // Get a mainloop and its context
    mainloop = pa_threaded_mainloop_new();
    assert(mainloop);
    mainloop_api = pa_threaded_mainloop_get_api(mainloop);
    context = pa_context_new(mainloop_api, "Monocoque");
    assert(context);

    pa_context_set_state_callback(context, &context_state_cb, mainloop);

    // Lock the mainloop so that it does not run and crash before the context is ready
    pa_threaded_mainloop_lock(mainloop);

    // Start the mainloop
    assert(pa_threaded_mainloop_start(mainloop) == 0);
    assert(pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) == 0);

    // Wait for the context to be ready
    for(;;) {
        pa_context_state_t context_state = pa_context_get_state(context);
        assert(PA_CONTEXT_IS_GOOD(context_state));
        if (context_state == PA_CONTEXT_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }

    slogi("successfully connected pulseaudio...");
    return 1;
}


int freesound()
{
    if (mainloop) {
        pa_threaded_mainloop_lock(mainloop);
    }
    if (context)
        pa_context_unref(context);

    if (mainloop) {
        pa_signal_done();
        pa_threaded_mainloop_unlock(mainloop);
        pa_threaded_mainloop_free(mainloop);
    }
}

#else

int setupsound()
{
    slogi("connecting portaudio...");
}


int freesound()
{

}

#endif
