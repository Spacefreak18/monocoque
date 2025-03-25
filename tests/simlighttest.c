#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libserialport.h>
#include "../src/arduino/shiftlights/shiftlights.h"

/* Helper function for error handling. */
int check(enum sp_return result);

int main()
{

    char* port_name = "/dev/simdev0";

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
    unsigned int timeout = 2000;
    result = check(sp_blocking_write(port, &bytes1, bufsize1, timeout));
    sleep(2);
    result = check(sp_blocking_read(port, &recv_buf1, recv_bufsize1, timeout));
    sleep(2);
    char numstr[recv_bufsize1];
    for(int j = 0; j < recv_bufsize1; j++)
    {
        numstr[j] = '\0';
    }
    for(int j = 0; j < recv_bufsize1; j++)
    {
        printf("%02x\n", recv_buf1[j]);
        if(recv_buf1[j] != 0 && recv_buf1[j] != 0x0d && recv_buf1[j] != 0x0a)
        {
            numstr[j] = recv_buf1[j];
        }
    }
    int numlights = atoi(numstr);
    printf("numlights is %s %i\n", numstr, numlights);
    //printf("%x\n", recv_buf[0]);
    //printf("%x\n", recv_buf[1]);
    //printf("%x\n", recv_buf[2]);
    //printf("%x\n", recv_buf[3]);
    //printf("%x\n", recv_buf[4]);

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
