#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <poll.h>
#include <termios.h>
#include <signal.h>
#include <uv.h>

#include "gameloop.h"
#include "loopdata.h"
#include "../helper/confighelper.h"
#include "../devices/simdevice.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/slog.h"

#define DEFAULT_UPDATE_RATE      240.0
#define SIM_CHECK_RATE           1.0
bool go = false;
bool go2 = false;
struct sigaction act;


uv_idle_t idler;
uv_timer_t datachecktimer;
uv_timer_t datamaptimer;
uv_udp_t recv_socket;

bool doui = false;
int appstate = 0;

void shmdatamapcallback(uv_timer_t* handle);
void datacheckcallback(uv_timer_t* handle);
void startdatalogger(MonocoqueSettings* ms, loop_data* l);

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

void looprun(MonocoqueSettings* ms, loop_data* f, SimData* simdata)
{

    if (doui == true)
    {
        slogi("looking for ui config %s pass 1", ms->config_str);
        int confignum = getconfigtouse2(ms->config_str, simdata->car, f->sim);
        slogi("first pass finished");
        if(confignum == -1)
        {
            slogi("looking for ui config %s pass 2", ms->config_str);
            confignum = getconfigtouse1(ms->config_str, simdata->car, f->sim);
        }
        if(confignum == -1)
        {
            slogi("looking for ui config %s pass 3", ms->config_str);
            confignum = getconfigtouse(ms->config_str, simdata->car, f->sim);
        }

        int configureddevices;
        configcheck(ms->config_str, confignum, &configureddevices);
        DeviceSettings* ds = malloc(configureddevices * sizeof(DeviceSettings));
        slogd("loading confignum %i, with %i devices.", confignum, configureddevices);
        f->numdevices = uiloadconfig(ms->config_str, confignum, configureddevices, ms, ds);

        if(ms->useconfig == 1)
        {
            ms->configcheck = 0;
        }

        f->simdevices = malloc(f->numdevices * sizeof(SimDevice));
        int initdevices = devinit(f->simdevices, configureddevices, ds, ms);

        for( int i = 0; i < configureddevices; i++)
        {
            settingsfree(ds[i]);
        }
        free(ds);

        int numdevices = f->numdevices;
        SimDevice* devices = f->simdevices;
        f->device_timers = (uv_timer_t*) (malloc(uv_handle_size(UV_TIMER) * numdevices));
        f->device_batons = (device_loop_data*) (malloc(sizeof(device_loop_data) * numdevices));

        for (int x = 0; x < numdevices; x++)
        {
            if (devices[x].initialized == true)
            {
                device_loop_data* dld = &f->device_batons[x];
                dld->simdevice = &devices[x];
                dld->simdata = simdata;
                uv_timer_t* dt = &f->device_timers[x];
                uv_timer_init(uv_default_loop(), dt);
                uv_handle_set_data((uv_handle_t*) dt, (void*) dld);
                int interval = 1000/devices[x].fps;
                uv_timer_start(dt, devicetimercallback, 0, interval);
                slogi("starting device type %i at id at %i fps: %i (%i ms ticks)", devices[x].type, x, devices[x].fps, interval);
            }
            else
            {
                slogw("skipped id %i", x);
            }
        }


        doui = false;
    }
    else
    {
        showstats(simdata);
        //SimDevice* devices = f->simdevices;
        //int numdevices = f->numdevices;
        //for (int x = 0; x < numdevices; x++)
        //{
        //    if (devices[x].initialized == true && devices[x].type == 2)
        //    {
        //        devices[x].update(&devices[x], simdata);
        //    }
        //}
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
        simdatamap(simdata, simmap, NULL, f->map, false, NULL);
        looprun(ms, f, simdata);
    }

    if (f->simstate == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        if(f->releasing == false)
        {
            f->releasing = true;
            uv_timer_stop(handle);
            slogi("releasing devices, please wait");
            f->uion = false;
            SimDevice* devices = f->simdevices;
            int numdevices = f->numdevices;

            // help things spin down
            simdata->rpms = 0;
            simdata->velocity = 0;

            for (int x = 0; x < numdevices; x++)
            {
                if (devices[x].initialized == true)
                {
                    uv_timer_t* dt = &f->device_timers[x];
                    slogt("attempting device timer stop and release");
                    slogt("timer active status %i", uv_is_active((uv_handle_t*) dt));
                    uv_timer_stop(dt);
                    //uv_close((uv_handle_t*) dt, on_timer_close_complete);
                }
            }
            free(f->device_batons);
            free(f->device_timers);
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


            int r = simfree(simdata, simmap, f->map);
            slogd("simfree returned %i", r);
            f->numdevices = 0;
            slogi("stopped mapping data, press q again to quit");
            //stopui(ms->ui_type, f);
            // free loop data

            if(appstate > 0)
            {
                uv_timer_start(&datachecktimer, datacheckcallback, 3000, 1000);
            }
            f->releasing = false;
            if(appstate > 1)
            {
                appstate = 1;
            }
        }
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

    char* a;
    a = rcvbuf->base;

    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;
    MonocoqueSettings* ms = f->ms;

    if (appstate == 2)
    {
        simdatamap(simdata, simmap, NULL, f->map, true, a);
        looprun(ms, f, simdata);
    }

    if (f->simstate == false || simdata->simstatus <= 1 || appstate <= 1)
    {
        if(f->releasing == false)
        {
            f->releasing = true;
            uv_udp_recv_stop(handle);
            slogi("releasing devices, please wait");
            f->uion = false;
            SimDevice* devices = f->simdevices;
            int numdevices = f->numdevices;

            // help things spin down
            simdata->rpms = 0;
            simdata->velocity = 0;
            for (int x = 0; x < numdevices; x++)
            {
                if (devices[x].initialized == true)
                {
                    devices[x].update(&devices[x], simdata);
                }
            }
            for (int x = 0; x < numdevices; x++)
            {
                if (devices[x].initialized == true)
                {
                    devices[x].free(&devices[x]);
                }
            }
            free(devices);
            int r = simfree(simdata, simmap, f->map);
            slogd("simfree returned %i", r);
            f->numdevices = 0;
            slogi("stopped mapping data, press q again to quit");
            //stopui(ms->ui_type, f);
            // free loop data

            if(appstate > 0)
            {
                uv_timer_start(&datachecktimer, datacheckcallback, 3000, 1000);
            }
            f->releasing = false;
            if(appstate > 1)
            {
                appstate = 1;
            }
        }
    }

    slogt("udp free  :%lu %p\n",rcvbuf->len,rcvbuf->base);
    free(rcvbuf->base);
}

int startudp(int port)
{
    uv_udp_init(uv_default_loop(), &recv_socket);
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
        simdatamap(simdata, simmap, NULL, f->sim, true, NULL);
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
        SimInfo si = getSim(simdata, simmap, f->ms->force_udp_mode, startudp, false);
        //TODO: move all this to a siminfo struct in loop_data
        f->simstate = si.isSimOn;
        f->sim = si.simulatorapi;
        f->map = si.mapapi;
        f->use_udp = si.SimUsesUDP;

        if(f->ms->force_udp_mode == true)
        {
            f->use_udp = true;
        }
    }
    if (f->simstate == true && simdata->simstatus >= 2)
    {
        if ( appstate == 1 )
        {
            appstate++;
            doui = true;
            simdata->tyrediameter[0] = -1;
            simdata->tyrediameter[1] = -1;
            simdata->tyrediameter[2] = -1;
            simdata->tyrediameter[3] = -1;

            if(f->use_udp == true)
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
            uv_timer_stop(handle);
        }
    }

    if (appstate == 0)
    {
        slogi("stopping checking for data");
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
        uv_timer_stop(&datachecktimer);
        uv_poll_stop(handle);
    }
}



int monocoque_mainloop(MonocoqueSettings* ms)
{

    SimData* simdata = malloc(sizeof(SimData));
    SimMap* simmap = createSimMap();

    struct termios newsettings, canonicalmode;
    tcgetattr(0, &canonicalmode);
    newsettings = canonicalmode;
    newsettings.c_lflag &= (~ICANON & ~ECHO);
    newsettings.c_cc[VMIN] = 1;
    newsettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newsettings);
    char ch;
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };

    uv_poll_t* poll = (uv_poll_t*) malloc(uv_handle_size(UV_POLL));

    loop_data* baton = (loop_data*) malloc(sizeof(loop_data));
    baton->simmap = simmap;
    baton->simdata = simdata;
    baton->ms = ms;
    baton->simstate = false;
    baton->uion = false;
    baton->releasing = false;
    baton->sim = 0;
    baton->req.data = (void*) baton;
    uv_handle_set_data((uv_handle_t*) &datachecktimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &datamaptimer, (void*) baton);
    uv_handle_set_data((uv_handle_t*) &recv_socket, (void*) baton);
    uv_handle_set_data((uv_handle_t*) poll, (void*) baton);
    appstate = 1;
    slogd("setting initial app state");
    uv_timer_init(uv_default_loop(), &datachecktimer);
    fprintf(stdout, "Searching for sim data... Press q to quit...\n");
    uv_timer_start(&datachecktimer, datacheckcallback, 1000, 1000);

    set_simapi_log_info(simapilib_loginfo);
    set_simapi_log_debug(simapilib_logdebug);
    set_simapi_log_trace(simapilib_logtrace);


    if (0 != uv_poll_init(uv_default_loop(), poll, 0))
    {
        return 1;
    };
    if (0 != uv_poll_start(poll, UV_READABLE, cb))
    {
        return 2;
    };


    uv_timer_init(uv_default_loop(), &datamaptimer);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    fprintf(stdout, "\n");
    fflush(stdout);
    tcsetattr(0, TCSANOW, &canonicalmode);

    free(baton);
    free(simdata);
    freesimmap(simmap, false);

    return 0;
}

int tester(SimDevice* devices, int numdevices)
{

    slogi("preparing test with %i devices...", numdevices);
    SimData* simdata = malloc(sizeof(SimData));

    struct termios newsettings, canonicalmode;
    tcgetattr(0, &canonicalmode);
    newsettings = canonicalmode;
    newsettings.c_lflag &= (~ICANON & ~ECHO);
    newsettings.c_cc[VMIN] = 1;
    newsettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newsettings);

    fprintf(stdout, "\n");
    simdata->car[0] = 'C';
    simdata->car[1] = 'A';
    simdata->car[2] = 'R';
    simdata->car[3] = '\0';

    simdata->gear = MONOCOQUE_GEAR_NEUTRAL;
    simdata->velocity = 16;
    simdata->rpms = 100;
    simdata->maxrpm = 10000;
    simdata->abs = 0;
    simdata->tyrediameter[0] = -1;
    simdata->tyrediameter[1] = -1;
    simdata->tyrediameter[2] = -1;
    simdata->tyrediameter[3] = -1;
    simdata->Xvelocity = 0;
    simdata->Yvelocity = 100;
    simdata->Zvelocity = 0;

    sleep(3);
/**
    fprintf(stdout, "Setting rpms to 1000\n");
    simdata->rpms = 1000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Green Flag!\n");
    simdata->playerflag = 0;

    fprintf(stdout, "Shifting into first gear\n");
    simdata->gear = 2;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Setting speed to 100\n");
    simdata->velocity = 100;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
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
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Testing wheel Lock\n");
    simdata->tyreRPS[0] = 25;
    simdata->tyreRPS[1] = 25;
    simdata->tyreRPS[2] = 25;
    simdata->tyreRPS[3] = 25;
    simdata->velocity = 150;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
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
    simdata->gear = 3;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
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
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
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
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Yellow Flag!\n");
    simdata->playerflag = 1;

    fprintf(stdout, "Shifting into third gear\n");
    simdata->gear = 4;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Setting rpms to 2000\n");
    simdata->rpms = 2000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Blue Flag!\n");
    simdata->playerflag = 4;

    fprintf(stdout, "Setting rpms to 4000\n");
    simdata->rpms = 4000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Shifting into fourth gear\n");
    simdata->gear = 5;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Setting speed to 300\n");
    simdata->velocity = 300;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Setting rpms to 7000\n");
    simdata->rpms = 7000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Red Flag!\n");
    simdata->playerflag = 2;

    fprintf(stdout, "Setting rpms to 8000\n");
    simdata->rpms = 8000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);
**/
    fprintf(stdout, "Setting rpms to 1000\n");
    simdata->rpms = 1000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 2000\n");
    simdata->rpms = 2000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 3000\n");
    simdata->rpms = 3000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 4000\n");
    simdata->rpms = 4000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 5000\n");
    simdata->rpms = 5000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 6000\n");
    simdata->rpms = 6000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 7000\n");
    simdata->rpms = 7000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 8000\n");
    simdata->rpms = 8000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 9000\n");
    simdata->rpms = 9000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fprintf(stdout, "Setting rpms to 10000\n");
    simdata->rpms = 10000;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(1);

    fflush(stdout);
    tcsetattr(0, TCSANOW, &canonicalmode);

    free(simdata);

    return 0;
}
