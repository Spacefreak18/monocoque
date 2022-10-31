#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <poll.h>
#include <termios.h>

#include "gameloop.h"
#include "../helper/parameters.h"
#include "../helper/confighelper.h"
#include "../devices/simdevice.h"
#include "../simulatorapi/simdata.h"
#include "../simulatorapi/simmapper.h"
#include "../slog/slog.h"

#define DEFAULT_UPDATE_RATE      120.0

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
            fputc(' ', stdout);
        }
    }
    fflush(stdout);
}

int looper(SimDevice* devices[], int numdevices, Simulator simulator)
{

    slogi("preparing game loop with %i devices...", numdevices);
    SimData* simdata = malloc(sizeof(SimData));
    SimMap* simmap = malloc(sizeof(SimMap));

    int error = siminit(simdata, simmap, simulator);

    if (error != MONOCOQUE_ERROR_NONE)
    {
        return error;
    }

    struct termios newsettings, canonicalmode;
    tcgetattr(0, &canonicalmode);
    newsettings = canonicalmode;
    newsettings.c_lflag &= (~ICANON & ~ECHO);
    newsettings.c_cc[VMIN] = 1;
    newsettings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newsettings);
    char ch;
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };

    double update_rate = DEFAULT_UPDATE_RATE;
    int t=0;
    int go = true;
    while (go == true)
    {
        simdatamap(simdata, simmap, simulator);
        showstats(simdata);
        t++;
        if(simdata->rpms<250)
        {
            simdata->rpms=250;
        }
        for (int x = 0; x < numdevices; x++)
        {
            if (devices[x]->type == SIMDEV_SERIAL)
            {
                if(t>=update_rate)
                {
                    devupdate(devices[x], simdata);
                }
            }
            else
            {
                devupdate(devices[x], simdata);
            }

        }
        if(t>=update_rate)
        {
            t=0;
        }
        if( poll(&mypoll, 1, 1000.0/update_rate) )
        {
            scanf("%c", &ch);
            if(ch == 'q')
            {
                go = false;
            }
        }
    }

    tcsetattr(0, TCSANOW, &canonicalmode);

    free(simdata);
    free(simmap);

    return 0;
}
