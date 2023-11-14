#include <stdbool.h>
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
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../slog/slog.h"

#define DEFAULT_UPDATE_RATE      240.0
#define SIM_CHECK_RATE           1.0

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


int clilooper(SimDevice* devices, int numdevices, Parameters* p, SimData* simdata, SimMap* simmap)
{

    slogi("preparing game loop with %i devices...", numdevices);


    slogi("sending initial data to devices");
    simdata->velocity = 16;
    simdata->rpms = 100;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);


    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
    double update_rate = DEFAULT_UPDATE_RATE;
    char ch;
    int t=0;
    int s=0;
    bool go = true;
    while (go == true && simdata->simstatus > 1)
    {
        simdatamap(simdata, simmap, p->sim);
        showstats(simdata);
        t++;
        s++;
        if(simdata->rpms<100)
        {
            simdata->rpms=100;
        }
        for (int x = 0; x < numdevices; x++)
        {
            devices[x].update(&devices[x], simdata);
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

    simdata->velocity = 0;
    simdata->rpms = 100;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }

    fprintf(stdout, "\n");

    return 0;
}

int looper(SimDevice* devices, int numdevices, Parameters* p)
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

    fprintf(stdout, "Searching for sim data... Press q to quit...\n");

    p->simon = false;
    double update_rate = SIM_CHECK_RATE;
    int go = true;
    while (go == true)
    {




        if (p->simon == false)
        {
            getSim(simdata, simmap, &p->simon, &p->sim);
        }

        if (p->simon == true)
        {
            clilooper(devices, numdevices, p, simdata, simmap);
        }
        if (p->simon == true)
        {
            p->simon = false;
            fprintf(stdout, "Searching for sim data... Press q again to quit...\n");
            sleep(2);
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

    fprintf(stdout, "\n");
    fflush(stdout);
    tcsetattr(0, TCSANOW, &canonicalmode);

    free(simdata);
    free(simmap);

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
    simdata->gear = 0;
    simdata->velocity = 16;
    simdata->rpms = 100;
    simdata->maxrpm = 8000;
    sleep(3);

    fprintf(stdout, "Setting rpms to 1000\n");
    simdata->rpms = 1000;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Shifting into first gear\n");
    simdata->gear = 2;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Setting speed to 100\n");
    simdata->velocity = 100;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Shifting into second gear\n");
    simdata->gear = 3;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Setting speed to 200\n");
    simdata->velocity = 200;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Shifting into third gear\n");
    simdata->gear = 4;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Setting rpms to 2000\n");
    simdata->rpms = 2000;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Setting rpms to 4000\n");
    simdata->rpms = 4000;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Shifting into fourth gear\n");
    simdata->gear = 5;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Setting speed to 300\n");
    simdata->velocity = 300;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    fprintf(stdout, "Setting rpms to 8000\n");
    simdata->rpms = 8000;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(3);

    simdata->velocity = 0;
    simdata->rpms = 100;
    for (int x = 0; x < numdevices; x++)
    {
        devices[x].update(&devices[x], simdata);
    }
    sleep(1);

    fflush(stdout);
    tcsetattr(0, TCSANOW, &canonicalmode);

    free(simdata);

    return 0;
}
