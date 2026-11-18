#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <poll.h>
#include <termios.h>
#include <signal.h>
#include <uv.h>

#include "gameloop.h"
#include "loopdata.h"
#include "testlooplua.h"
#include "../helper/confighelper.h"
#include "../devices/simdevice.h"
#include "../devices/hapticeffect.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../simulatorapi/simapi/simapi/simmap.h"
#include "../slog/slog.h"

#define DEFAULT_UPDATE_RATE      240.0
#define SIM_CHECK_RATE           1.0

struct sigaction act;
static pthread_t loop_thread;
static uv_loop_t *loop = NULL;

SimData* simdata;
SimMap* simmap;

loop_data* baton;
device_loop_data* test_baton;

SimDevice* test_simdevice;
SimInfo* test_siminfo;

uv_idle_t idler;
uv_timer_t datachecktimer;
uv_timer_t showstatstimer;
uv_timer_t datamaptimer;
uv_timer_t tyrediametertimer;
uv_timer_t testdevicetimer;
uv_udp_t recv_socket;

bool doui = false;

void shmdatamapcallback(uv_timer_t* handle);
void showstatscallback(uv_timer_t* handle);
void datacheckcallback(uv_timer_t* handle);
void tyrediametercheckcallback(uv_timer_t* handle);
void startdatalogger(MonocoqueSettings* ms, loop_data* l);

static void close_walk_cb(uv_handle_t* handle, void* arg)
{
    if (!uv_is_closing(handle))
    {
        uv_close(handle, NULL);
    }
}
#define ASSERT(expr) expr

int showstats(SimData* simdata)
{
    printf("\r");
    for (int i=0; i<4; i++)
    {
        if (i==0)
        {
            fputc('s', stdout);
            fputc('p', stdout);
            fputc('e', stdout);
            fputc('e', stdout);
            fputc('d', stdout);
            fputc(':', stdout);
            fputc(' ', stdout);

            int speed = simdata->velocity;
            int digits = 0;
            if (speed > 0)
            {
                while (speed > 0)
                {
                    int mod = speed % 10;
                    speed = speed / 10;
                    digits++;
                }
                speed = simdata->velocity;
                int s[digits];
                int digit = 0;
                while (speed > 0)
                {
                    int mod = speed % 10;
                    s[digit] = mod;
                    speed = speed / 10;
                    digit++;
                }
                speed = simdata->velocity;
                digit = digits;
                while (digit > 0)
                {
                    fputc(s[digit-1]+'0', stdout);
                    digit--;
                }
            }
            else
            {
                fputc('0', stdout);
            }
            fputc(' ', stdout);
        }
        if (i==1)
        {
            fputc('r', stdout);
            fputc('p', stdout);
            fputc('m', stdout);
            fputc('s', stdout);
            fputc(':', stdout);
            fputc(' ', stdout);

            int rpms = simdata->rpms;
            int digits = 0;
            if (rpms > 0)
            {
                while (rpms > 0)
                {
                    int mod = rpms % 10;
                    rpms = rpms / 10;
                    digits++;
                }
                rpms = simdata->rpms;
                int s[digits];
                int digit = 0;
                while (rpms > 0)
                {
                    int mod = rpms % 10;
                    s[digit] = mod;
                    rpms = rpms / 10;
                    digit++;
                }
                rpms = simdata->rpms;
                digit = digits;
                while (digit > 0)
                {
                    fputc(s[digit-1]+'0', stdout);
                    digit--;
                }
            }
            else
            {
                fputc('0', stdout);
            }
            fputc(' ', stdout);
        }
        if (i==2)
        {
            fputc('g', stdout);
            fputc('e', stdout);
            fputc('a', stdout);
            fputc('r', stdout);
            fputc(':', stdout);
            fputc(' ', stdout);
            fputc(simdata->gear+'0', stdout);
            fputc(' ', stdout);
        }
        if (i==3)
        {
            fputc('a', stdout);
            fputc('l', stdout);
            fputc('t', stdout);
            fputc(':', stdout);
            fputc(' ', stdout);

            int alt = simdata->altitude;
            int digits = 0;
            if (alt > 0)
            {
                while (alt > 0)
                {
                    int mod = alt % 10;
                    alt = alt / 10;
                    digits++;
                }
                alt = simdata->altitude;
                int s[digits];
                int digit = 0;
                while (alt > 0)
                {
                    int mod = alt % 10;
                    s[digit] = mod;
                    alt = alt / 10;
                    digit++;
                }
                alt = simdata->altitude;
                digit = digits;
                while (digit > 0)
                {
                    fputc(s[digit-1]+'0', stdout);
                    digit--;
                }
            }
            else
            {
                fputc('0', stdout);
            }
            fputc(' ', stdout);
        }
    }
    fflush(stdout);
}

void simapilib_loginfo(char* message)
{
    slogi(message);
}

void simapilib_logdebug(char* message)
{
    slogd(message);
}

void simapilib_logtrace(char* message)
{
    slog_display(SLOG_TRACE, 1, message);
}

void on_timer_close_complete(uv_handle_t* handle)
{
    free(handle);
}


void devicetimercallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    device_loop_data* f = (device_loop_data*) b;
    SimData* simdata = f->simdata;
    SimDevice* device = f->simdevice;
    device->update(device, simdata);
}


void tyrediametercheckcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    device_loop_data* f = (device_loop_data*) b;
    SimData* simdata = f->simdata;

    if(simdata->car != NULL || simdata->car[0] != 0)
    {
        slogi("car is %s", simdata->car);
        // check for saved tyre diameter in config file
        // if not saved version exists get tyre diameter and save it
        // use config check variable to track if the config check has been performed
        // avoid many opens of the same file
        int error = 0;
        if(hasTyreDiameter(simdata)==false && f->tyre_data_checked == false)
        {
            slogi("attempting load of tyre diameter config");
            error = loadtyreconfig(simdata, f->ms->tyre_diameter_config, true);
            f->tyre_data_checked = true;
            if(hasTyreDiameter(simdata)==false)
            {
                slogt("could not find tyre diameter in config file, attempting to calculate new");
            }
        }
    }
    if(hasTyreDiameter(simdata)==true)
    {
        int a = loadtyreconfig(simdata, f->ms->tyre_diameter_config, false);
        if(a < 0)
        {
            slogi("saving new tyre diameter config for car %s");
            savetyreconfig(simdata, f->ms->tyre_diameter_config);
        }
        uv_timer_stop(handle);
    }
    getTyreDiameter(simdata);

}

void looprun(MonocoqueSettings* ms, loop_data* f, SimData* simdata)
{
    if (doui == true)
    {
        int confignum = -1;
        int forced_config = mono_config_force_config(ms->cfg, ms);
        if(forced_config >= 0)
        {
            confignum = ms->force_config;
            slogi("Configuration force configuration %i");
        }
        else 
        {
            slogi("Attempting to resolve best config for sim %i car %s and simapi %i", f->siminfo.simulatorexe, simdata->car, f->siminfo.simulatorapi);
            confignum = getconfigtouse(ms->config_str, simdata->car, f->siminfo.simulatorexe, f->siminfo.simulatorapi);
            slogd("Selected config num %i", confignum);
        }

        int configureddevices = 0;
        DeviceSettings *ds = NULL;
        if (device_settings_load(ms->config_str, confignum, ms,
                                 &ds, &configureddevices) != 0)
        {
            sloge("Could not load device settings");
            doui = false;
            return;
        }
        slogd("loading confignum %i, with %i devices.", confignum, configureddevices);
        f->numdevices = configureddevices;
        f->simdevices = calloc(configureddevices ? (size_t)configureddevices : 1, sizeof(*f->simdevices));
        if (f->simdevices == NULL)
        {
            device_settings_free_all(ds, configureddevices);
            f->numdevices = 0;
            sloge("Could not allocate runtime devices");
            doui = false;
            return;
        }
        int initdevices = devinit(f->simdevices, &f->siminfo, configureddevices, ds, ms);
        slogi("initialized %i devices", initdevices);
        device_settings_free_all(ds, configureddevices);

        int numdevices = f->numdevices;
        SimDevice* devices = f->simdevices;
        f->device_timers = (uv_timer_t*) (malloc(uv_handle_size(UV_TIMER) * numdevices));
        f->device_batons = (device_loop_data*) (malloc(sizeof(device_loop_data) * numdevices));
        f->started_tyre_calc_thread = false;

        for (int x = 0; x < numdevices; x++)
        {
            if (devices[x].initialized == true)
            {
                device_loop_data* dld = &f->device_batons[x];
                dld->simdevice = &devices[x];
                dld->simdata = simdata;
                uv_timer_t* dt = &f->device_timers[x];
                uv_timer_init(uv_handle_get_loop((uv_handle_t *)&datachecktimer), dt);
                uv_handle_set_data((uv_handle_t*) dt, (void*) dld);
                int interval = 1000/devices[x].fps;
                uv_timer_start(dt, devicetimercallback, 0, interval);
                slogi("starting device type %i at id at %i fps: %i (%i ms ticks)", devices[x].type, x, devices[x].fps, interval);

                SimInfo siminfo = f->siminfo;
                if(f->started_tyre_calc_thread == false)
                {
                    if(devices[x].hapticeffect.effecttype == EFFECT_TYRELOCK || devices[x].hapticeffect.effecttype == EFFECT_TYRESLIP || devices[x].hapticeffect.effecttype == EFFECT_ABSBRAKES)
                    {
                        dld->tyre_data_checked = false;
                        if(ms->usetyreconfig == true)
                        {
                            dld->tyre_data_checked = false;
                        }
                        if(siminfo.SimCalculatesTyreDiameter == false && siminfo.SimSupportsHapticEffects == true && siminfo.SimCalculatesSlipRatio == false)
                        {
                            slogi("Starting thread to calculate tyre diameters and save to config file");
                            f->started_tyre_calc_thread = true;

                            f->tyrebaton = (device_loop_data*) malloc(sizeof(device_loop_data));
                            f->tyrebaton->simdata = simdata;
                            f->tyrebaton->ms = f->ms;

                            uv_handle_set_data((uv_handle_t*) &tyrediametertimer, (void*) f->tyrebaton);
                            uv_timer_start(&tyrediametertimer, tyrediametercheckcallback, 0, 1000);
                        }
                    }
                }
            }
        }

        if(f->cli == true)
        {
            uv_timer_start(&showstatstimer, showstatscallback, 0, 100);
        }
        doui = false;
    }
}

void showstatscallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    if(appstate == 2)
    {
        showstats(simdata);
    }
}

/* Device timer arrays stay allocated until their uv_close callbacks run. */
typedef struct device_timer_cleanup {
    uv_timer_t *timers;
    device_loop_data *batons;
    int pending;
} device_timer_cleanup;

static void device_timer_closed(uv_handle_t *handle)
{
    device_timer_cleanup *owner = handle->data;
    if (--owner->pending == 0)
    {
        free(owner->timers);
        free(owner->batons);
        free(owner);
    }
}

static void close_device_timers(loop_data *f)
{
    int count = 0;
    for (int x = 0; x < f->numdevices; ++x)
        if (f->simdevices[x].initialized)
            ++count;

    if (count == 0)
    {
        free(f->device_timers);
        free(f->device_batons);
    }
    else
    {
        device_timer_cleanup *owner = malloc(sizeof(*owner));
        if (owner != NULL)
            *owner = (device_timer_cleanup){f->device_timers, f->device_batons, count};

        /* If allocation fails, leak arrays instead of freeing live handles. */
        for (int x = 0; x < f->numdevices; ++x)
        {
            if (!f->simdevices[x].initialized)
                continue;
            uv_timer_t *dt = &f->device_timers[x];
            uv_timer_stop(dt);
            if (owner != NULL)
                uv_handle_set_data((uv_handle_t *)dt, owner);
            uv_close((uv_handle_t *)dt, owner != NULL ? device_timer_closed : NULL);
        }
    }
    f->device_timers = NULL;
    f->device_batons = NULL;
}


void releaseloop(loop_data* f, SimData* simdata, SimMap* simmap)
{
        fprintf(stdout, "\n");
        slogi("release loop");
        if(f->releasing == false)
        {
            f->releasing = true;
            uv_timer_stop(&datamaptimer);
            uv_timer_stop(&showstatstimer);
            if (uv_is_active((uv_handle_t*)&recv_socket))
            {
                uv_udp_recv_stop(&recv_socket);
            }
            // Close the socket handle so it can be reinitialized with a different port
            //if (!uv_is_closing((uv_handle_t*)&recv_socket))
            //{
            //    uv_close((uv_handle_t*)&recv_socket, NULL);
            //}
            slogi("releasing devices, please wait");

            //attempt tyre diameter saving
            uv_timer_stop(&tyrediametertimer);
            if(f->started_tyre_calc_thread == true)
            {
                free(f->tyrebaton);
            }

            f->uion = false;
            SimDevice* devices = f->simdevices;
            int numdevices = f->numdevices;

            // help things spin down
            simdata->simstatus = 0;
            simdata->rpms = 0;
            simdata->velocity = 0;

            close_device_timers(f);
            slogt("stopped device timers");
            for (int x = 0; x < numdevices; x++)
            {
                if (devices[x].initialized == true)
                {
                    devices[x].update(&devices[x], simdata);
                }
            }
            sleep(1);
            for (int x = 0; x < numdevices; x++)
            {
                if (devices[x].initialized == true)
                {
                    devices[x].free(&devices[x]);
                }
            }
            free(devices);

            int r = simapi_sim_clear(simdata, simmap, false);
            slogd("simfree returned %i", r);
            f->numdevices = 0;
            slogi("stopped mapping data, press q again to quit");
            //stopui(ms->ui_type, f);
            // free loop data

            if(appstate > 0)
            {
                slogi("restarting checking for data...");
                uv_timer_start(&datachecktimer, datacheckcallback, 0, 1000);
            }
            f->releasing = false;
            if(appstate > 1)
            {
                appstate = 1;
            }
        }
}

void shmdatamapcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    MonocoqueSettings* ms = f->ms;
    //appstate = 2;
    if (appstate == 2)
    {
        simapi_datamap(simdata, simmap, f->siminfo.mapapi, false, NULL);
        looprun(ms, f, simdata);
    }

    if (f->siminfo.isSimOn == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        releaseloop(f, simdata, simmap);
    }
}

void on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
    bzero(buf->base, suggested_size);
    slogt("udp malloc:%lu %p\n",buf->len,buf->base);
}

static void on_udp_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* rcvbuf, const struct sockaddr* addr, unsigned flags) {
    if (nread > 0) {
        slogt("udp data received");
    }
    if (nread <= 0) {
        free(rcvbuf->base);
        return;
    }

    char* a;
    a = rcvbuf->base;

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    MonocoqueSettings* ms = f->ms;

    if (appstate == 2)
    {
        simapi_datamap(simdata, simmap, f->siminfo.mapapi, true, a);
        looprun(ms, f, simdata);
    }

    if (f->siminfo.isSimOn == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        releaseloop(f, simdata, simmap);
    }

    slogt("udp free  :%lu %p\n",rcvbuf->len,rcvbuf->base);
    free(rcvbuf->base);
}

int startudp(int port)
{

    struct sockaddr_in recv_addr;
    uv_ip4_addr("0.0.0.0", port, &recv_addr);
    int err = uv_udp_bind(&recv_socket, (const struct sockaddr *) &recv_addr, UV_UDP_REUSEADDR);

    slogt("initial udp error is %i", err);
    return err;
}

void udpstart(MonocoqueSettings* sms, loop_data* f, SimData* simdata, SimMap* simmap)
{
    if (appstate == 2)
    {
        simapi_datamap(simdata, simmap, f->siminfo.simulatorapi, true, NULL);
        if (doui == true)
        {
            looprun(sms, f, simdata);
        }
    }
}

void datacheckcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;

    if ( appstate == 1 )
    {
        f->siminfo = simapi_get_sim(simdata, simmap, f->ms->force_udp_mode, startudp, false);

        if(f->ms->force_udp_mode == true)
        {
            f->use_udp = true;
        }
    }
    if (f->siminfo.isSimOn == true && simdata->simstatus >= 2)
    {
        if ( appstate == 1 )
        {
            appstate++;
            doui = true;
            simdata->tyrediameter[0] = -1;
            simdata->tyrediameter[1] = -1;
            simdata->tyrediameter[2] = -1;
            simdata->tyrediameter[3] = -1;

            if(f->use_udp == true || f->siminfo.SimUsesUDP == true)
            {
                slogt("starting udp receive loop");
                udpstart(f->ms, f, simdata, simmap);
                uv_udp_recv_start(&recv_socket, on_alloc, on_udp_recv);
                slogt("udp receive loop started");
            }
            else
            {
                int interval = 1000 / f->ms->fps;
                slogd("starting telemetry mapping at %i fps (%i ms ticks)", f->ms->fps, interval);
                uv_timer_start(&datamaptimer, shmdatamapcallback, 2000, interval);
            }
        }
        if(appstate == 2)
        {
            f->siminfo = simapi_get_sim(simdata, simmap, f->ms->force_udp_mode, NULL, false);
            if(f->siminfo.isSimOn == false)
            {
                appstate = 1;
                releaseloop(f, simdata, simmap);
            }
        }
    }

    if (appstate == 0)
    {
        slogi("stopped checking for data");
        uv_timer_stop(handle);
    }
}

void cb(uv_poll_t* handle, int status, int events)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    char ch;
    scanf("%c", &ch);
    if (ch == 'q')
    {
        if(f->releasing == false && doui == false)
        {
            appstate--;
            fprintf(stdout, "\nUser requested stop appstate is now %i\n", appstate);
            fflush(stdout);
            slogi("User requested stop appstate is now %i", appstate);
        }
    }

    if (appstate == 0)
    {
        slogi("Monocoque is exiting...");
        uv_udp_recv_stop(&recv_socket);
        uv_timer_stop(&datamaptimer);
        uv_timer_stop(&showstatstimer);
        // at this point these below should be the only active threads
        uv_timer_stop(&datachecktimer);
        uv_poll_stop(handle);
    }
}



static uv_poll_t mainloop_stdin_poll;
static uv_async_t mainloop_stop_async;

static void mainloop_stop_requested(uv_async_t *handle)
{
    /* uv_async_send() is thread-safe; stopping libuv handles here ensures
     * uv_stop() executes on the event-loop thread, not the GUI thread. */
    appstate = 0;
    if (baton != NULL && baton->numdevices > 0 && !baton->releasing)
        releaseloop(baton, simdata, simmap);
    uv_udp_recv_stop(&recv_socket);
    uv_timer_stop(&datamaptimer);
    uv_timer_stop(&showstatstimer);
    uv_timer_stop(&datachecktimer);
    uv_timer_stop(&tyrediametertimer);
    uv_stop(handle->loop);
}

static int mainloop_run(MonocoqueSettings *ms, uv_loop_t *run_loop, bool cli)
{
    int err;
    simdata = calloc(1, sizeof(*simdata));
    simmap = simapi_simmap_create();
    baton = calloc(1, sizeof(*baton));
    if (simdata == NULL || simmap == NULL || baton == NULL)
    {
        free(baton);
        free(simdata);
        free(simmap);
        baton = NULL;
        simdata = NULL;
        simmap = NULL;
        return -1;
    }

    baton->siminfo.mapapi = -1;
    baton->simmap = simmap;
    baton->simdata = simdata;
    baton->ms = ms;
    baton->req.data = baton;
    baton->cli = cli;
    doui = false;

    simapi_set_log_info(simapilib_loginfo);
    simapi_set_log_debug(simapilib_logdebug);
    simapi_set_log_trace(simapilib_logtrace);

    /* Use the supplied loop for both front ends, including all device timers
     * created later by looprun(). */
    if ((err = uv_udp_init(run_loop, &recv_socket)) != 0 ||
        (err = uv_timer_init(run_loop, &datachecktimer)) != 0 ||
        (err = uv_timer_init(run_loop, &showstatstimer)) != 0 ||
        (err = uv_timer_init(run_loop, &datamaptimer)) != 0 ||
        (err = uv_timer_init(run_loop, &tyrediametertimer)) != 0)
    {
        fprintf(stderr, "Failed to initialize main loop handle: %s\n", uv_strerror(err));
        return -1;
    }

    uv_handle_set_data((uv_handle_t *)&recv_socket, baton);
    uv_handle_set_data((uv_handle_t *)&datachecktimer, baton);
    uv_handle_set_data((uv_handle_t *)&showstatstimer, baton);
    uv_handle_set_data((uv_handle_t *)&datamaptimer, baton);

    if (cli)
    {
        err = uv_poll_init(run_loop, &mainloop_stdin_poll, STDIN_FILENO);
        if (err == 0)
        {
            uv_handle_set_data((uv_handle_t *)&mainloop_stdin_poll, baton);
            err = uv_poll_start(&mainloop_stdin_poll, UV_READABLE, cb);
        }
        if (err != 0)
        {
            fprintf(stderr, "Failed to initialize CLI input: %s\n", uv_strerror(err));
            return -1;
        }
        fprintf(stdout, "Searching for sim data... Press q to quit...\n");
    }

    slogd("setting initial app state");
    appstate = 1;
    err = uv_timer_start(&datachecktimer, datacheckcallback, 1000, 1000);
    if (err != 0)
    {
        fprintf(stderr, "Failed to start simulator timer: %s\n", uv_strerror(err));
        return -1;
    }
    uv_run(run_loop, UV_RUN_DEFAULT);
    return 0;
}

void *monocoque_mainloop_start(void *arg)
{
    (void)mainloop_run((MonocoqueSettings *)arg, loop, false);
    return NULL;
}

SimData* get_test_simdata()
{
    if(test_baton == NULL)
    {
        return NULL;
    }
    return test_baton->simdata;
}

void* monocoque_testloop_start(void* arg)
{
    test_loop_args* args = arg;
    simmap = simapi_simmap_create();


    simapi_set_log_info(simapilib_loginfo);
    simapi_set_log_debug(simapilib_logdebug);
    simapi_set_log_trace(simapilib_logtrace);


    DeviceSettings ds;
    //int numdevices = getsingledevice(args->ms->config_str, args->confignum, args->devicenum, args->ms, &ds);
    test_simdevice = calloc(1, sizeof(SimDevice));
    test_siminfo = malloc(sizeof(SimInfo));
    simapi_set_faux_siminfo(test_siminfo);
    int initdevices = devinit(test_simdevice, test_siminfo, 1, args->ds, args->ms);

    test_baton->siminfo.mapapi = -1;
    test_baton->simdevice = test_simdevice;

    test_baton->ms = args->ms;
    test_baton->req.data = (void*) test_baton;


    uv_timer_init(loop, &testdevicetimer);
    uv_handle_set_data((uv_handle_t*) &testdevicetimer, (void*) test_baton);
    if (initdevices > 0 && test_simdevice->initialized && test_simdevice->update != NULL)
    {
        uv_timer_start(&testdevicetimer, devicetimercallback, 1000, 16);
    }
    else
    {
        slogw("Device test skipped: device initialization failed");
    }

    uv_run(loop, UV_RUN_DEFAULT);


    return NULL;
}

int monocoque_testloop_stop()
{
    uv_timer_stop(&testdevicetimer);

    int err = uv_async_send(&mainloop_stop_async);
    pthread_join(loop_thread, NULL);

    uv_walk(loop, close_walk_cb, NULL);
    uv_run(loop, UV_RUN_NOWAIT);
    uv_loop_close(loop);
    uv_library_shutdown();

    slogi("All threads stopped...");

    if (test_simdevice != NULL && test_simdevice->initialized && test_simdevice->free != NULL)
    {
        test_simdevice->free(test_simdevice);
    }
    free(test_simdevice);
    test_simdevice = NULL;
    free(loop);
    loop = NULL;
    free(test_baton);
    //free(simdata);
    free(simmap);

    return 0;
}

int monocoque_mainloop_stop(MonocoqueSettings *ms)
{
    (void)ms;
    if (loop == NULL)
        return -1;

    int err = uv_async_send(&mainloop_stop_async);
    if (err != 0)
        return err;
    pthread_join(loop_thread, NULL);

    uv_walk(loop, close_walk_cb, NULL);
    uv_run(loop, UV_RUN_DEFAULT);  /* Drain uv_close callbacks. */
    err = uv_loop_close(loop);
    if (err != 0)
        return err;  /* Do not free an active uv_loop_t. */

    free(loop);
    loop = NULL;
    free(baton);
    free(simdata);
    free(simmap);
    baton = NULL;
    simdata = NULL;
    simmap = NULL;
    appstate = 0;
    slogi("All threads stopped...");
    return 0;
}


int start_loop(MonocoqueSettings *ms)
{
    if (loop != NULL)
        return -1;
    loop = malloc(sizeof(*loop));
    if (loop == NULL)
        return -1;

    int err = uv_loop_init(loop);
    if (err != 0)
    {
        free(loop);
        loop = NULL;
        return err;
    }
    err = uv_async_init(loop, &mainloop_stop_async, mainloop_stop_requested);
    if (err != 0)
    {
        uv_loop_close(loop);
        free(loop);
        loop = NULL;
        return err;
    }
    err = pthread_create(&loop_thread, NULL, monocoque_mainloop_start, ms);
    if (err != 0)
    {
        uv_close((uv_handle_t *)&mainloop_stop_async, NULL);
        uv_run(loop, UV_RUN_DEFAULT);
        uv_loop_close(loop);
        free(loop);
        loop = NULL;
    }
    return err;
}

int start_test(test_loop_args* test_data)
{

    test_baton = (device_loop_data*) malloc(sizeof(device_loop_data));
    test_baton->simdata = test_data->simdata;

    loop = malloc(sizeof(uv_loop_t));

    if (loop == NULL)
    {
        return -1;
    }

    if (uv_loop_init(loop) != 0)
    {
        free(loop);
        loop = NULL;
        return -1;
    }
    return pthread_create(&loop_thread, NULL, monocoque_testloop_start, test_data);
}

const char* get_simexe_name(void)
{
    if(appstate <= 0)
    {
        return "None Detected";
    }
    if(baton == NULL)
    {
        return "None Detected";
    }
    if(baton->siminfo.simulatorexe <= 0)
    {
        return "None Detected";
    }
    return simapi_gametofullstr(baton->siminfo.simulatorexe);
}

const char* get_simd_onoff(void)
{
    if(appstate <= 0)
    {
        return "Not Detected";
    }
    if(baton == NULL)
    {
        return "Not Detected";
    }
    if(baton->siminfo.mapapi == 0)
    {
        return "Running";
    }
    return "Not Detected";
}

int monocoque_mainloop(MonocoqueSettings *ms)
{
    struct termios original, raw;
    bool restore_terminal = (tcgetattr(STDIN_FILENO, &original) == 0);
    if (restore_terminal)
    {
        raw = original;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }

    uv_loop_t *run_loop = uv_default_loop();
    int result = mainloop_run(ms, run_loop, true);
    uv_walk(run_loop, close_walk_cb, NULL);
    uv_run(run_loop, UV_RUN_DEFAULT);
    int close_error = uv_loop_close(run_loop);
    if (result == 0 && close_error != 0)
        result = close_error;

    if (restore_terminal)
        tcsetattr(STDIN_FILENO, TCSANOW, &original);
    fprintf(stdout, "\n");
    fflush(stdout);

    free(baton);
    free(simdata);
    free(simmap);
    baton = NULL;
    simdata = NULL;
    simmap = NULL;
    slogi("All threads stopped...");
    return result;
}

void set_basic_simdata(SimData* simdata)
{
    simdata->car[0] = 'D';
    simdata->car[1] = 'E';
    simdata->car[2] = 'F';
    simdata->car[3] = 'A';
    simdata->car[4] = 'U';
    simdata->car[5] = 'L';
    simdata->car[6] = 'T';
    simdata->car[7] = '\0';
    simdata->gear = SIMAPI_GEAR_NEUTRAL;
    simdata->gearc[0] = 0x4e;
    simdata->gearc[1] = 0;
    simdata->velocity = 160;
    simdata->rpms = 7000;
    simdata->maxrpm = 8000;
    simdata->abs = 0;
    simdata->tyrediameter[0] = -1;
    simdata->tyrediameter[1] = -1;
    simdata->tyrediameter[2] = -1;
    simdata->tyrediameter[3] = -1;
    simdata->tyreslipratio[0] = 0;
    simdata->tyreslipratio[1] = 0;
    simdata->tyreslipratio[2] = 0;
    simdata->tyreslipratio[3] = 0;
    simdata->Xvelocity = 0;
    simdata->Yvelocity = 100;
    simdata->Zvelocity = 0;
}

void set_wheel_spin_simdata(SimData* simdata)
{
    simdata->velocity = 15;
    simdata->tyreRPS[0] = 50;
    simdata->tyreRPS[1] = 50;
    simdata->tyreRPS[2] = 50;
    simdata->tyreRPS[3] = 50;
    simdata->tyrediameter[0] = 0.638636385206394;
    simdata->tyrediameter[1] = 0.633384434597093;
    simdata->tyrediameter[2] = 0.710475735564615;
    simdata->tyrediameter[3] = 0.710475735564615;
}

void set_wheel_lock_simdata(SimData* simdata)
{
    simdata->velocity = 150;
    simdata->tyreRPS[0] = 25;
    simdata->tyreRPS[1] = 25;
    simdata->tyreRPS[2] = 25;
    simdata->tyreRPS[3] = 25;
    simdata->tyrediameter[0] = 0.638636385206394;
    simdata->tyrediameter[1] = 0.633384434597093;
    simdata->tyrediameter[2] = 0.710475735564615;
    simdata->tyrediameter[3] = 0.710475735564615;
}

// Drives every initialized device with the current test simdata, then
// mirrors that same simdata into the SIMAPI.DAT shared memory segment (if
// available) so external tools - dashboards, telemetry viewers, anything
// that reads SIMAPI.DAT the same way a real running sim would populate it -
// can follow monocoque's own test sequence instead of seeing nothing.
static void update_devices(SimDevice* devices, int numdevices, SimData* simdata, SimMap* testsimmap)
{
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    if (testsimmap != NULL && testsimmap->addr != NULL)
    {
        simdata->mtick++;
        memcpy(testsimmap->addr, simdata, sizeof(SimData));
    }
}

static int static_test(SimDevice* devices, int numdevices, SimData* simdata, SimMap* testsimmap)
{

    fflush(stdout);
    //// TODO: look into this, my serial leds make this hang
    //for (int r = 0; r < 8000; r += 3)
    //{
    //    simdata->rpms = 1000 + r;
    //    update_devices(devices, numdevices, simdata, testsimmap);
    //    usleep(1000);
    //}

    //for (int r = 0; r < 8000; r += 3)
    //{
    //    simdata->rpms = 9000 - r;
    //    update_devices(devices, numdevices, simdata, testsimmap);
    //    usleep(1000);
    //}

    fprintf(stdout, "Setting rpms to 1000\n");
    simdata->rpms = 1000;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Green Flag!\n");
    simdata->playerflag = SIMAPI_FLAG_GREEN;

    fprintf(stdout, "Shifting into first gear\n");
    simdata->gear = SIMAPI_GEAR_FIRST;
    simdata->gearc[0] = 0x31;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Setting speed to 100\n");
    simdata->velocity = 100;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "testing wheel spin\n");
    simdata->velocity = 15;
    simdata->tyreRPS[0] = 50;
    simdata->tyreRPS[1] = 50;
    simdata->tyreRPS[2] = 50;
    simdata->tyreRPS[3] = 50;
    simdata->tyrediameter[0] = 0.638636385206394;
    simdata->tyrediameter[1] = 0.633384434597093;
    simdata->tyrediameter[2] = 0.710475735564615;
    simdata->tyrediameter[3] = 0.710475735564615;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Testing wheel Lock\n");
    simdata->tyreRPS[0] = 25;
    simdata->tyreRPS[1] = 25;
    simdata->tyreRPS[2] = 25;
    simdata->tyreRPS[3] = 25;
    simdata->velocity = 150;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Shifting into second gear\n");
    simdata->tyreRPS[0] = 0;
    simdata->tyreRPS[1] = 0;
    simdata->tyreRPS[2] = 0;
    simdata->tyreRPS[3] = 0;
    simdata->tyrediameter[0] = -1;
    simdata->tyrediameter[1] = -1;
    simdata->tyrediameter[2] = -1;
    simdata->tyrediameter[3] = -1;
    simdata->abs = 0;
    simdata->gear = SIMAPI_GEAR_SECOND;
    simdata->gearc[0] = 0x32;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Testing abs brake lock Lock\n");
    simdata->tyreRPS[0] = 50;
    simdata->tyreRPS[1] = 50;
    simdata->tyreRPS[2] = 50;
    simdata->tyreRPS[3] = 50;
    simdata->tyrediameter[0] = 0.638636385206394;
    simdata->tyrediameter[1] = 0.633384434597093;
    simdata->tyrediameter[2] = 0.710475735564615;
    simdata->tyrediameter[3] = 0.710475735564615;
    simdata->abs = .11;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Setting speed to 200\n");
    simdata->tyreRPS[0] = 0;
    simdata->tyreRPS[1] = 0;
    simdata->tyreRPS[2] = 0;
    simdata->tyreRPS[3] = 0;
    simdata->tyrediameter[0] = -1;
    simdata->tyrediameter[1] = -1;
    simdata->tyrediameter[2] = -1;
    simdata->tyrediameter[3] = -1;
    simdata->abs = 0;
    simdata->velocity = 200;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Yellow Flag!\n");
    simdata->playerflag = SIMAPI_FLAG_YELLOW;

    fprintf(stdout, "Shifting into third gear\n");
    simdata->gear = SIMAPI_GEAR_THIRD;
    simdata->gearc[0] = 0x33;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Setting rpms to 2000\n");
    simdata->rpms = 2000;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Blue Flag!\n");
    simdata->playerflag = SIMAPI_FLAG_BLUE;

    fprintf(stdout, "Setting rpms to 4000\n");
    simdata->rpms = 4000;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Shifting into fourth gear\n");
    simdata->gear = SIMAPI_GEAR_FOURTH;
    simdata->gearc[0] = 0x34;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Setting speed to 300\n");
    simdata->velocity = 300;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Setting rpms to 7000\n");
    simdata->rpms = 7000;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(3);

    fprintf(stdout, "Red Flag!\n");
    simdata->playerflag = SIMAPI_FLAG_RED;

    fprintf(stdout, "Setting rpms to 8000\n");
    simdata->rpms = 8000;
    update_devices(devices, numdevices, simdata, testsimmap);
    for(int x = 0; x < 100; x++)
    {
        update_devices(devices, numdevices, simdata, testsimmap);
    }
    sleep(3);

    simdata->velocity = 0;
    simdata->rpms = 100;
    simdata->gear = SIMAPI_GEAR_NEUTRAL;
    simdata->gearc[0] = 0x4e;
    update_devices(devices, numdevices, simdata, testsimmap);
    sleep(1);

    fflush(stdout);

}

int tester(MonocoqueSettings* ms, SimDevice* devices, int numdevices)
{

    slogi("preparing test with %i devices...", numdevices);
    SimData* simdata = malloc(sizeof(SimData));
    memset(simdata, 0, sizeof(SimData));
    simdata->simon = true;
    simdata->simstatus = SIMAPI_STATUS_ACTIVEPLAY;

    SimMap* testsimmap = malloc(sizeof(SimMap));
    memset(testsimmap, 0, sizeof(SimMap));
    int shmerr = simapi_universalmap_open(testsimmap, simdata);
    if (shmerr != SIMAPI_ERROR_NONE)
    {
        slog_warn("Could not open shared telemetry memory for test mode (error %i) - test sequence will still drive local devices, but external tools won't see it", shmerr);
        free(testsimmap);
        testsimmap = NULL;
    }

    struct termios newsettings, canonicalmode;
    tcgetattr(0, &canonicalmode);
    newsettings = canonicalmode;
    newsettings.c_lflag &= (~ICANON & ~ECHO);
    newsettings.c_cc[VMIN] = 1;
    newsettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newsettings);


    fprintf(stdout, "\n");
    simdata->car[0] = 'D';
    simdata->car[1] = 'E';
    simdata->car[2] = 'F';
    simdata->car[3] = 'A';
    simdata->car[4] = 'U';
    simdata->car[5] = 'L';
    simdata->car[6] = 'T';
    simdata->car[7] = '\0';

    simdata->gear = SIMAPI_GEAR_NEUTRAL;
    simdata->gearc[0] = 0x4e;
    simdata->gearc[1] = 0;
    simdata->velocity = 16;
    simdata->rpms = 100;
    simdata->maxrpm = 8000;
    simdata->abs = 0;
    simdata->tyrediameter[0] = -1;
    simdata->tyrediameter[1] = -1;
    simdata->tyrediameter[2] = -1;
    simdata->tyrediameter[3] = -1;
    simdata->tyreslipratio[0] = 0;
    simdata->tyreslipratio[1] = 0;
    simdata->tyreslipratio[2] = 0;
    simdata->tyreslipratio[3] = 0;
    simdata->Xvelocity = 0;
    simdata->Yvelocity = 100;
    simdata->Zvelocity = 0;

    if(ms->lua_test == true)
    {
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);
        
        slogi("Using file %s", ms->test_lua_file_str);
        
        int top = lua_gettop(L);
        int status = luaL_loadfile(L, ms->test_lua_file_str);

        if (status)
        {
            /* If something went wrong, error message is at the top of the stack*/
            fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        }
        else
        {
            lua_setglobal(L,"myFunc");

            lua_pushstring(L, "simdata");
            lua_pushlightuserdata(L, simdata);
            lua_settable(L, LUA_REGISTRYINDEX);
            lua_pushstring(L, "simdevices");
            lua_pushlightuserdata(L, devices);
            lua_settable(L, LUA_REGISTRYINDEX);
            lua_pushstring(L, "testsimmap");
            lua_pushlightuserdata(L, testsimmap);
            lua_settable(L, LUA_REGISTRYINDEX);
            lua_pushstring(L, "numdevices");
            lua_pushinteger(L, numdevices);
            lua_settable(L, LUA_REGISTRYINDEX);

            lua_register(L, "update_devices", monocoque_test_update_devices);
            lua_register(L, "sleep", monocoque_test_sleep);
            lua_register(L, "set_rpms", monocoque_test_set_rpms);
            lua_register(L, "set_max_rpms", monocoque_test_set_max_rpms);


            lua_getglobal(L,"myFunc");
            if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            {
                fprintf(stderr, "Error calling Lua script: %s\n", lua_tostring(L, -1));
            }
        }
        lua_close(L);
    }
    else 
    {
        static_test(devices, numdevices, simdata, testsimmap);
    }

    tcsetattr(0, TCSANOW, &canonicalmode);

    // Not shm_unlink()'d - the segment itself stays behind, the same way a
    // real sim's shared memory file stays around after that sim exits - but
    // its *contents* are reset to an inactive frame first. Leaving the last
    // active-play frame in place made every external reader (e.g.
    // typiql-tauri) see this test run as a sim that's still actively
    // driving forever, since nothing else ever writes to SIMAPI.DAT again
    // until the next `monocoque test`/`monocoque play`.
    if (testsimmap != NULL)
    {
        simdata->simon = false;
        simdata->simstatus = SIMAPI_STATUS_OFF;
        update_devices(devices, numdevices, simdata, testsimmap);

        munmap(testsimmap->addr, sizeof(SimData));
        close(testsimmap->fd);
        free(testsimmap);
    }

    free(simdata);

    return 0;
}
