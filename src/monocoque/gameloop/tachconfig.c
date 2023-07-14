#include <stdio.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <termios.h>
#include <poll.h>

#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>

#include "../devices/simdevice.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../slog/slog.h"

#define DEFAULT_UPDATE_RATE      30.0

int WriteXmlFromArrays(int nodes, int rpm_array[], int values_array[], int maxrevs, const char* save_file)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr rootnode = NULL, onenode = NULL, settingsvaluenode = NULL, maxdisplayvaluenode = NULL;
    char buff[256];
    int i, j;

    doc = xmlNewDoc(BAD_CAST "1.0");

    rootnode = xmlNewNode(NULL, BAD_CAST "TachometerSettings");
    xmlDocSetRootElement(doc, rootnode);
    settingsvaluenode = xmlNewNode(NULL, BAD_CAST "SettingsValues");

    for(int i = 0; i< nodes; ++i)
    {
        onenode = xmlNewNode(NULL, BAD_CAST "SettingsItem");

        char value[10];
        sprintf(value, "%d", values_array[i]);

        char rpm[10];
        sprintf(rpm, "%d", rpm_array[i]);

        xmlNewChild(onenode, NULL, BAD_CAST "Value", BAD_CAST value);
        xmlNewChild(onenode, NULL, BAD_CAST "TimeValue", BAD_CAST rpm);
        xmlAddChild(settingsvaluenode, onenode);
    }


    xmlAddChild(rootnode, settingsvaluenode);
    char revs[10];
    sprintf(revs, "%d", maxrevs);
    maxdisplayvaluenode = xmlNewChild(rootnode, NULL, BAD_CAST "MaxDisplayValue", BAD_CAST revs);

    xmlSaveFormatFileEnc(save_file,  doc, "UTF-8", 1);

    xmlFreeDoc(doc);

    xmlCleanupParser();

    return 0;
}

int config_tachometer(int max_revs, int granularity, const char* save_file, SimDevice* simdevice, SimData* simdata)
{
    int pulses = 0;
    int nodes = 0;

    if (max_revs<2000)
    {
        fprintf(stderr, "revs must be at least 2000\n");
        return 0;
    }

    int increment = 1000;
    if (granularity == 2)
    {
        increment = 500;
    }
    if (granularity == 4)
    {
        increment = 250;
    }

    nodes = ((max_revs/1000)*granularity)+1;
    if (granularity >= 4)
    {
        nodes--;
    }
    int rpm_array[nodes];
    int values_array[nodes];

    values_array[0]=250;
    values_array[1]=increment;
    if (granularity >= 4)
    {
        values_array[0] = increment;
        values_array[1]= increment * 2;
    }

    for(int i=2; i<nodes; i++)
    {
        values_array[i]=values_array[i-1]+increment;
    }


    for(int i=0; i<nodes; i++)
    {

        struct termios newsettings, canonicalmode;
        tcgetattr(0, &canonicalmode);
        newsettings = canonicalmode;
        newsettings.c_lflag &= (~ICANON & ~ECHO);
        newsettings.c_cc[VMIN] = 1;
        newsettings.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &newsettings);
        char ch = ' ';

        if (i==0)
        {
            fprintf(stdout, "Press Return to continue...\n");
            scanf("%c",&ch);
        }
        sleep(2);
        fprintf(stdout, "Set tachometer revs to %i: Press > to increase, < to decrease, and Return to accept (m increases by 1000, n decreases by 1000, c increases by 100, z decreases by 100...\n", values_array[i]);

        struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
        double update_rate = DEFAULT_UPDATE_RATE;

        int go=1;
        while (go>0)
        {

            simdata->pulses = pulses;
            devupdate(simdevice, simdata);
            if( poll(&mypoll, 1, 1000.0/update_rate) )
            {
                ch = ' ';
                scanf("%c", &ch);

                if (ch == 'n')
                {
                    pulses=pulses-1000;
                }
                if (ch == 'm')
                {
                    pulses=pulses+1000;
                }
                if (ch == 'z')
                {
                    pulses=pulses-100;
                }
                if (ch == 'c')
                {
                    pulses=pulses+100;
                }
                if (ch == '<')
                {
                    pulses--;
                }
                if (ch == '>')
                {
                    pulses++;
                }

                if (ch == '\n')
                {
                    go=0;

                    fprintf(stdout, "set pulses to %i\n", pulses);
                    rpm_array[i]=pulses;

                }


            }
        }
        tcsetattr(0, TCSANOW, &canonicalmode);
    }

    WriteXmlFromArrays(nodes, rpm_array, values_array, max_revs, save_file);
    sleep(2);
    simdata->pulses = 0;
    simdevice->update(simdevice, simdata);
    fflush(stdout);

    return 0;
}
