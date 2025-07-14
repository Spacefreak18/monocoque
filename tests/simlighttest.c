#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libserialport.h>
#include "../src/arduino/shiftlights/shiftlights.h"

/* Helper function for error handling. */
int check(enum sp_return result)
{
    /* For this example we'll just exit on any error by calling abort(). */
    char* error_message;

    switch (result)
    {
        case SP_ERR_ARG:
            printf("Error: Invalid argument.\n");
            abort();
        case SP_ERR_FAIL:
            error_message = sp_last_error_message();
            printf("Error: Failed: %s\n", error_message);
            sp_free_error_message(error_message);
            abort();
        case SP_ERR_SUPP:
            printf("Error: Not supported.\n");
            abort();
        case SP_ERR_MEM:
            printf("Error: Couldn't allocate memory.\n");
            abort();
        case SP_OK:
        default:
            return result;
    }
}


int WaitForEventOnPort(struct sp_port* port, int event)
{
    int retval;
    struct sp_event_set* eventSet = NULL;

    retval = sp_new_event_set(&eventSet);
    if (retval == SP_OK)
    {
        retval = sp_add_port_events(eventSet, port, event);
        if (retval == SP_OK)
        {
            printf("set event on port\n");
            retval = sp_wait(eventSet, 5000);
        }
        else
        {
            puts("Unable to add events to port.");
            retval = -1;
        }
    }
    else
    {
        puts("Unable to create new event set.");
        retval = -1;
    }
    sp_free_event_set(eventSet);

    return retval;
}

// the input event waiting and flushing is the right way to do this
// most of the time i'm just "bit blasting" and the only receiving
// happens initially to get the number of lights
int ReadFromPort(struct sp_port* port, int* numlights)
{
    int count = 0;
    int bytesWaiting;
    char buf[256];
    int retval = 0;
    int i;

    sp_flush(port, SP_BUF_INPUT);

    while (count < 4)
    {
        printf("Attempting to retrieve num lights from port...\n");

        size_t bufsize1 = 11;
        size_t recv_bufsize1 = 5;
        char recv_buf1[recv_bufsize1];
        char bytes1[bufsize1];

        for(int j = 0; j < bufsize1; j++)
        {
            bytes1[j] = 0x00;
        }
        bytes1[0] = 0xff;
        bytes1[1] = 0xff;
        bytes1[2] = 0xff;
        bytes1[3] = 0xff;
        bytes1[4] = 0xff;
        bytes1[5] = 0xff;
        bytes1[6] = 0x6c;
        bytes1[7] = 0x65;
        bytes1[8] = 0x64;
        bytes1[9] = 0x73;
        bytes1[10] = 0x63;

        int result = 0;
        unsigned int timeout = 6000;
        printf("Sending message to get num lights\n");
        WaitForEventOnPort(port, 6);

        result = check(sp_blocking_write(port, &bytes1, bufsize1, timeout));

        printf("Attempting to receive response\n");


        WaitForEventOnPort(port, 5);
        bytesWaiting = sp_input_waiting(port);
        if (bytesWaiting > 0)
        {
            memset(buf, 0, sizeof(buf));
            //printf("bytes found on port\n");
            retval = sp_blocking_read(port, buf, sizeof(buf)-1, 10);
            if (retval < 0)
            {
                printf("Error reading from serial port: %d\r\n", retval);
                retval = -1;
                break;
            }
            else
            {
                for(i=0; i<retval; i++)
                {
                    printf("%c", buf[i]);
                    if (buf[i] == 13)
                    {
                        count++;
                    }
                }
                int ret = atoi(buf);
                *numlights = ret;
                return retval;
            }
        }
        else
            if (bytesWaiting < 0)
            {
                printf("Error getting bytes available from serial port: %d\r\n", bytesWaiting);
                retval = -1;
                break;
            }
        printf("didn't get nothing\n");
        //count++;
        retval = 0;
    }
    return retval;
}

int main()
{

    char* port_name = "/dev/ttyACM0";

    /* The ports we will use. */
    struct sp_port* port;

    /* Open and configure each port. */
    printf("Looking for port %s.\n", port_name);
    check(sp_get_port_by_name(port_name, &port));
    printf("Using %s\r\n", sp_get_port_name(port));
    printf("Opening port.\n");
    check(sp_open(port, SP_MODE_READ | SP_MODE_WRITE));

    printf("Setting port to 115200 8N1, no flow control.\n");
    check(sp_set_baudrate(port, 115200));
    check(sp_set_bits(port, 8));
    check(sp_set_parity(port, SP_PARITY_NONE));
    check(sp_set_stopbits(port, 1));
    check(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE));
    check(sp_set_rts(port, 1));
    check(sp_set_dtr(port, 1));

    ShiftLightsData sd;

    int result = 0;
    int numlights =  0;
    result = ReadFromPort(port, &numlights);

    int timeout = 2000;
    size_t bufsize = (numlights * 3) + 14;
    char bytes[bufsize];
    for(int j = 0; j < bufsize; j++)
    {
        bytes[j] = 0x00;
    }
    bytes[0] = 0xff;
    bytes[1] = 0xff;
    bytes[2] = 0xff;
    bytes[3] = 0xff;
    bytes[4] = 0xff;
    bytes[5] = 0xff;
    bytes[6] = 0x73;
    bytes[7] = 0x6c;
    bytes[8] = 0x65;
    bytes[9] = 0x64;
    bytes[10] = 0x73;
    bytes[bufsize-1] = 0xfd;
    bytes[bufsize-2] = 0xfe;
    bytes[bufsize-3] = 0xff;


    for( int i = 0; i < numlights; i++)
    {
        bytes[(i * 3) + 11 + 1] = 0xff;

        /* Send data. */
        result = check(sp_blocking_write(port, &bytes, bufsize, timeout));

        /* Check whether we sent all of the data. */
        if (result == bufsize)
        {
            printf("Sent %d bytes successfully, %i lit leds.\n", bufsize, numlights);
        }
        else
        {
            printf("Timed out, %d/%d bytes sent.\n", result, bufsize);
        }


        sleep(2);
    }
    for( int i = 0; i < numlights; i++)
    {
        bytes[(i * 3) + 11 + 1] = 0x00;
    }
    result = check(sp_blocking_write(port, &bytes, bufsize, timeout));

    check(sp_close(port));
    sp_free_port(port);
}


