#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libserialport.h>
#include "../src/monocoque/simulatorapi/simapi/simapi/simdata.h"

/* Helper function for error handling. */
int check(enum sp_return result);

int main()
{
    SimData sd;
    sd.maxrpm = 6500;
    sd.rpms = 0;
    sd.altitude = 10;
    sd.gear = 1;
    sd.velocity = 74;

    char* port_name = "/dev/ttyACM0";

    /* The ports we will use. */
    struct sp_port* port;

    /* Open and configure each port. */
    printf("Looking for port %s.\n", port_name);
    check(sp_get_port_by_name(port_name, &port));

    printf("Opening port.\n");
    check(sp_open(port, SP_MODE_READ_WRITE));

    printf("Setting port to 9600 8N1, no flow control.\n");
    check(sp_set_baudrate(port, 9600));
    check(sp_set_bits(port, 8));
    check(sp_set_parity(port, SP_PARITY_NONE));
    check(sp_set_stopbits(port, 1));
    check(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE));

    while ( sd.rpms <= sd.maxrpm )
    {


        unsigned int timeout = 2000;

        /* On success, sp_blocking_write() and sp_blocking_read()
         * return the number of bytes sent/received before the
        * timeout expired. We'll store that result here. */
        int result;
        int size = sizeof(SimData);

        /* Send data. */
        result = check(sp_blocking_write(port, &sd, size, timeout));

        /* Check whether we sent all of the data. */
        if (result == size)
        {
            printf("Sent %d bytes successfully, %i rpm.\n", size, sd.rpms);
        }
        else
        {
            printf("Timed out, %d/%d bytes sent.\n", result, size);
        }

        sd.rpms += 1000;
        if ( sd.rpms == sd.maxrpm + 1000 )
        {
            sd.rpms = 0;
        }
        if ( sd.rpms > sd.maxrpm )
        {
            sd.rpms = sd.maxrpm;
        }

        sleep(1);
    }

    check(sp_close(port));
    sp_free_port(port);
}

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
