#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libserialport.h>
#include "../src/arduino/shiftlights/shiftlights.h"

/* Helper function for error handling. */
int check(enum sp_return result);

int main()
{

    char* port_name = "/dev/ttyACM0";

    /* The ports we will use. */
    struct sp_port* port;

    /* Open and configure each port. */
    printf("Looking for port %s.\n", port_name);
    check(sp_get_port_by_name(port_name, &port));

    printf("Opening port.\n");
    check(sp_open(port, SP_MODE_READ_WRITE));

    printf("Setting port to 115200 8N1, no flow control.\n");
    check(sp_set_baudrate(port, 115200));
    check(sp_set_bits(port, 8));
    check(sp_set_parity(port, SP_PARITY_NONE));
    check(sp_set_stopbits(port, 1));
    check(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE));

    ShiftLightsData sd;

    int numlights = 7;
    size_t bufsize = (numlights * 3) + 9;
    char bytes[bufsize];

    bytes[0] = 0xff;
    bytes[1] = 0xff;
    bytes[2] = 0xff;
    bytes[3] = 0xff;
    bytes[4] = 0xff;
    bytes[5] = 0xff;
    bytes[bufsize-1] = 0xfd;
    bytes[bufsize-2] = 0xfe;
    bytes[bufsize-3] = 0xff;

    int result;
    unsigned int timeout = 2000;
    for( int i = 0; i < 7; i++)
    {
        bytes[(i * 3) + 6 + 1] = 0xff;

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
    sd.litleds = 0;
    result = check(sp_blocking_write(port, &bytes, bufsize, timeout));

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
