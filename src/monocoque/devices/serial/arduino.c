#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "arduino.h"
#include "../../slog/slog.h"

#define arduino_timeout 5000

int arduino_update(SerialDevice* serialdevice, void* data, size_t size)
{
    int result = 1;
    if (serialdevice->port)
    {
        slogt("copying %i bytes to arduino device", size);
        result = check(sp_blocking_write(serialdevice->port, data, size, arduino_timeout));
    }

    return result;
}

//int arduino_shiftlights_update(SerialDevice* serialdevice, SimData* simdata)
//{
//    int result = 1;
//    if (serialdevice->port)
//    {
//        result = check(sp_blocking_write(serialdevice->port, simdata, sizeof(SimData), arduino_timeout));
//    }
//
//    return result;
//}
//
//int arduino_simwind_update(SerialDevice* serialdevice, SimData* simdata)
//{
//    int result = 1;
//    if (serialdevice->port)
//    {
//        result = check(sp_blocking_write(serialdevice->port, simdata, sizeof(SimData), arduino_timeout));
//    }
//
//    return result;
//}

int arduino_init(SerialDevice* serialdevice, const char* portdev)
{
    slogi("initializing arduino serial device...");
    int error = 0;
    char* port_name = strdup(portdev);

    slogd("Looking for port %s.\n", port_name);
    error = check(sp_get_port_by_name(port_name, &serialdevice->port));
    if (error != 0)
    {
        return error;
    }

    slogd("Opening port.\n");
    check(sp_open(serialdevice->port, SP_MODE_READ_WRITE));

    slogd("Setting port to %i 8N1, no flow control", serialdevice->baud);
    check(sp_set_baudrate(serialdevice->port, serialdevice->baud));
    check(sp_set_bits(serialdevice->port, 8));
    check(sp_set_parity(serialdevice->port, SP_PARITY_NONE));
    check(sp_set_stopbits(serialdevice->port, 1));
    check(sp_set_flowcontrol(serialdevice->port, SP_FLOWCONTROL_NONE));

    free(port_name);
    slogd("Successfully setup arduino serial device...");
    return 0;
}

int arduino_free(SerialDevice* serialdevice)
{
    check(sp_close(serialdevice->port));
    sp_free_port(serialdevice->port);
}

int check(enum sp_return result)
{
    /* For this example we'll just exit on any error by calling abort(). */
    char* error_message;

    switch (result)
    {
        case SP_ERR_ARG:
            //printf("Error: Invalid argument.\n");
            return 1;
        //abort();
        case SP_ERR_FAIL:
            error_message = sp_last_error_message();
            //printf("error: failed: %s\n", error_message);
            sloge("error: serial write failed: %s", error_message);
            sp_free_error_message(error_message);
            //abort();
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

