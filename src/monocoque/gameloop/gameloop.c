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

#include "../helper/parameters.h"
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
        simdatamap(simdata, simmap, f->sim);

        if (doui == true)
        {
            slogi("looking for ui config %s", ms->config_str);
            int confignum = getconfigtouse(ms->config_str, simdata->car, f->sim);
            int configureddevices;
            configcheck(ms->config_str, confignum, &configureddevices);

            DeviceSettings* ds = malloc(configureddevices * sizeof(DeviceSettings));
            slogd("loading confignum %i, with %i devices.", confignum, configureddevices);

            f->numdevices = uiloadconfig(ms->config_str, confignum, configureddevices, ms, ds);

            f->simdevices = malloc(f->numdevices * sizeof(SimDevice));
            int initdevices = devinit(f->simdevices, configureddevices, ds, ms);
            int i = 0;
            for( i = 0; i < configureddevices; i++)
            {
                settingsfree(ds[i]);
            }
            free(ds);
            doui = false;
        }
        else
        {
            showstats(simdata);
            SimDevice* devices = f->simdevices;
            int numdevices = f->numdevices;
            for (int x = 0; x < numdevices; x++)
            {
                if (devices[x].initialized == true)
                {
                    devices[x].update(&devices[x], simdata);
                }
            }
        }
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
            int r = simfree(simdata, simmap, f->sim);
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

void datacheckcallback(uv_timer_t* handle)
{
    void* b = uv_handle_get_data((uv_handle_t*) handle);
    loop_data* f = (loop_data*) b;
    SimData* simdata = f->simdata;
    SimMap* simmap = f->simmap;

    if ( appstate == 1 )
    {
        getSim(simdata, simmap, &f->simstate, &f->sim);
    }
    if (f->simstate == true && simdata->simstatus >= 2)
    {
        if ( appstate == 1 )
        {
            appstate++;
            doui = true;
            uv_timer_start(&datamaptimer, shmdatamapcallback, 2000, 16);
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
            slogi("User requested stop appstate is now %i", appstate);
            fprintf(stdout, "User requested stop appstate is now %i\n", appstate);
            fflush(stdout);
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
    SimMap* simmap = malloc(sizeof(SimMap));

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
    uv_handle_set_data((uv_handle_t*) poll, (void*) baton);
    appstate = 1;
    slogd("setting initial app state");
    uv_timer_init(uv_default_loop(), &datachecktimer);
    fprintf(stdout, "Searching for sim data... Press q to quit...\n");
    uv_timer_start(&datachecktimer, datacheckcallback, 1000, 1000);


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
    free(simmap);

    return 0;
}

//int clilooper(SimDevice* devices, int numdevices, Parameters* p, SimData* simdata, SimMap* simmap)
//{
//    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
//    double update_rate = DEFAULT_UPDATE_RATE;
//    char ch;
//    int t=0;
//    int s=0;
//    go2 = true;
//    slogd("game data found, starting game loop");
//    while (go2 == true && simdata->simstatus > 1)
//    {
//        simdatamap(simdata, simmap, p->sim);
//        showstats(simdata);
//        t++;
//        s++;
//        if(simdata->rpms<100)
//        {
//            simdata->rpms=100;
//        }
//
//        for (int x = 0; x < numdevices; x++)
//        {
//            if (devices[x].initialized == true)
//            {
//                devices[x].update(&devices[x], simdata);
//            }
//        }
//
//    	if( poll(&mypoll, 1, 1000.0/update_rate) )
//    	{
//    	    scanf("%c", &ch);
//    	    if(ch == 'q')
//    	    {
//                slogd("User gameloop exit requested.");
//    	        go2 = false;
//    	    }
//    	}
//    }
//
//    simdata->velocity = 0;
//    simdata->rpms = 100;
//    simdata->gear = MONOCOQUE_GEAR_NEUTRAL;
//    for (int x = 0; x < numdevices; x++)
//    {
//        if (devices[x].initialized == true)
//        {
//            devices[x].update(&devices[x], simdata);
//        }
//    }
//
//    fprintf(stdout, "\n");
//    int r = simfree(simdata, simmap, p->sim);
//    slogd("simfree returned %i", r);
//
//    return 0;
//}

//int looper(SimDevice* devices, int numdevices, Parameters* p)
//{
//    memset(&act, 0, sizeof(act));
//    act.sa_sigaction = sighandler;
//    act.sa_flags = SA_SIGINFO;
//    sigaction(SIGTERM, &act, NULL);
//    sigaction(SIGINT, &act, NULL);
//    sigaction(SIGTSTP, &act, NULL);
//
//    SimData* simdata = malloc(sizeof(SimData));
//    SimMap* simmap = malloc(sizeof(SimMap));
//    simdata->tyrediameter[0] = -1;
//    simdata->tyrediameter[1] = -1;
//    simdata->tyrediameter[2] = -1;
//    simdata->tyrediameter[3] = -1;
//
//    struct termios newsettings, canonicalmode;
//    tcgetattr(0, &canonicalmode);
//    newsettings = canonicalmode;
//    newsettings.c_lflag &= (~ICANON & ~ECHO);
//    newsettings.c_cc[VMIN] = 1;
//    newsettings.c_cc[VTIME] = 0;
//    tcsetattr(0, TCSANOW, &newsettings);
//    char ch;
//    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
//
//    fprintf(stdout, "Searching for sim data... Press q to quit...\n");
//    p->simon = false;
//    double update_rate = SIM_CHECK_RATE;
//    go = true;
//    while (go == true)
//    {
//        p->simon = false;
//        getSim(simdata, simmap, &p->simon, &p->sim);
//
//        if (p->simon == true && simdata->simstatus > 1)
//        {
//            slogi("preparing game loop with %i devices...", numdevices);
//
//
//            slogi("sending initial data to devices");
//            simdata->velocity = 16;
//            simdata->rpms = 100;
//            for (int x = 0; x < numdevices; x++)
//            {
//                devices[x].update(&devices[x], simdata);
//            }
//            sleep(3);
//
//            clilooper(devices, numdevices, p, simdata, simmap);
//        }
//        if (p->simon == true)
//        {
//            p->simon = false;
//            fprintf(stdout, "Searching for sim data... Press q again to quit...\n");
//            sleep(2);
//        }
//
//        if (poll(&mypoll, 1, 1000.0/update_rate) )
//        {
//            if (go != false )
//            {
//                scanf("%c", &ch);
//                if(ch == 'q')
//                {
//                    slogd("User application exit requested.");
//                    go = false;
//                }
//            }
//        }
//    }
//
//    fprintf(stdout, "\n");
//    fflush(stdout);
//    tcsetattr(0, TCSANOW, &canonicalmode);
//
//    int r = simfree(simdata, simmap, p->sim);
//    slogd("simfree returned %i", r);
//    free(simdata);
//    free(simmap);
//
//    return 0;
//}

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
    simdata->gear = MONOCOQUE_GEAR_NEUTRAL;
    simdata->velocity = 16;
    simdata->rpms = 100;
    simdata->maxrpm = 8000;
    simdata->abs = 0;
    simdata->tyrediameter[0] = -1;
    simdata->tyrediameter[1] = -1;
    simdata->tyrediameter[2] = -1;
    simdata->tyrediameter[3] = -1;

    sleep(3);

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
    simdata->tyrediameter[0] = 0.638636385206394;
    simdata->tyrediameter[1] = 0.633384434597093;
    simdata->tyrediameter[2] = 0.710475735564615;
    simdata->tyrediameter[3] = 0.710475735564615;
    simdata->tyreRPS[0] = 103;
    simdata->tyreRPS[1] = 103;
    simdata->tyreRPS[2] = 103;
    simdata->tyreRPS[3] = 103;
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    sleep(3);

    fprintf(stdout, "Testing wheel Lock\n");
    simdata->tyreRPS[0] = 50;
    simdata->tyreRPS[1] = 50;
    simdata->tyreRPS[2] = 50;
    simdata->tyreRPS[3] = 50;
    simdata->velocity = 100;
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
    simdata->tyrediameter[0] = 0.638636385206394;
    simdata->tyrediameter[1] = 0.633384434597093;
    simdata->tyrediameter[2] = 0.710475735564615;
    simdata->tyrediameter[3] = 0.710475735564615;
    simdata->tyreRPS[3] = 50;
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

    simdata->velocity = 0;
    simdata->rpms = 100;
    simdata->gear = MONOCOQUE_GEAR_NEUTRAL;
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
