#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "serialadapter.h"
#include "../slog/slog.h"

int msastrcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

int check(enum sp_return result)
{
    char* error_message;

    switch (result)
    {
        case SP_ERR_ARG:
            return 1;
        case SP_ERR_FAIL:
            error_message = sp_last_error_message();
            sloge("error: serial write failed: %s", error_message);
            sp_free_error_message(error_message);
        case SP_ERR_SUPP:
            printf("Error: Not supported.\n");
        case SP_ERR_MEM:
            printf("Error: Couldn't allocate memory.\n");
        case SP_OK:
        default:
            return result;
    }
}

int monocoque_serial_free(SerialDevice* serialdevice)
{
    monocoque_serial_device monocoque_serial_dev = monocoque_serial_devices[serialdevice->id];

    if(monocoque_serial_dev.open == true)
    {
        while(monocoque_serial_dev.busy == true)
        {
            slogt("hopefully this doesn't happen long");
            continue;
        }

        monocoque_serial_devices[serialdevice->id].busy = true;
        monocoque_serial_devices[serialdevice->id].refs--;

        if(monocoque_serial_devices[serialdevice->id].refs == 0)
        {
            slogd("freeing physical device %s", monocoque_serial_devices[serialdevice->id].portname);
            sp_close(monocoque_serial_devices[serialdevice->id].port);
            sp_free_port(monocoque_serial_devices[serialdevice->id].port);
            monocoque_serial_devices[serialdevice->id].busy = false;
            monocoque_serial_devices[serialdevice->id].open = false;
            monocoque_serial_devices[serialdevice->id].openfail = false;
            free(monocoque_serial_devices[serialdevice->id].portname);
        }
        else
        {
            monocoque_serial_devices[serialdevice->id].busy = false;
        }

    }
}

int monocoque_serial_write(uint8_t serialdevicenum, void* data, size_t size, int timeout)
{
    slogt("serial device id %i", serialdevicenum);
    monocoque_serial_device monocoque_serial_dev = monocoque_serial_devices[serialdevicenum];

    slogt("port name: %s, busy %i, open %i, openfail %i", monocoque_serial_dev.portname, monocoque_serial_dev.busy, monocoque_serial_dev.open, monocoque_serial_dev.busy);

    if(monocoque_serial_dev.port == NULL)
    {
        sloge("port is null");
    }

    int result = -1;
    if(monocoque_serial_dev.busy == false && monocoque_serial_dev.open == true)
    {
        monocoque_serial_dev.busy = true;
        result = sp_blocking_write(monocoque_serial_dev.port, data, size, timeout);
    }
    else
    {
        slogw("serial device data update ignored due to busy or lost device");
        result = -1;
    }
    slogt("write result is %i", result);
    monocoque_serial_dev.busy = false;
    return result;
}

int monocoque_serial_write_block(uint8_t serialdevicenum, void* data, size_t size, int timeout)
{
    slogt("serial device id %i", serialdevicenum);
    monocoque_serial_device monocoque_serial_dev = monocoque_serial_devices[serialdevicenum];

    slogt("port name: %s, busy %i, open %i, openfail %i", monocoque_serial_dev.portname, monocoque_serial_dev.busy, monocoque_serial_dev.open, monocoque_serial_dev.busy);

    if(monocoque_serial_dev.port == NULL)
    {
        sloge("port is null");
    }

    int result = -1;
    if(monocoque_serial_dev.open == true)
    {
        while(monocoque_serial_dev.busy == true)
        {
            slogt("hopefully this doesn't happen long");
            continue;
        }

        monocoque_serial_dev.busy = true;
        result = sp_blocking_write(monocoque_serial_dev.port, data, size, timeout);
        slogi("actually performed write");
    }

    monocoque_serial_dev.busy = false;
    return result;
}

int monocoque_serial_open(SerialDevice* serialdevice, const char* portdev)
{
    int serial_device_num = -1;
    bool notfound = true;
    slogi("looking to open physical serialdevice %s", portdev);
    for(int i = 0; i < 10; i++)
    {
        if(monocoque_serial_devices[i].open == true || monocoque_serial_devices[i].openfail == true)
        {
            if(msastrcicmp(monocoque_serial_devices[i].portname, portdev) == 0)
            {
                notfound = false;
                serial_device_num = i;
                monocoque_serial_devices[i].refs++;
                slogd("found exisiting handle to serial device %s", portdev);
            }
        }
    }

    if(notfound == true)
    {
        slogd("no existing connections found, looking to create new");
        int i = -1;
        bool avail = false;
        while(avail == false)
        {
            i++;
            if(monocoque_serial_devices[i].open == false && monocoque_serial_devices[i].openfail == false)
            {
                avail = true;
                break;
            }
        }


        slogi("opening physical serial device...");
        int error = 0;
        char* port_name = strdup(portdev);
        monocoque_serial_devices[i].portname = strdup(port_name);

        struct sp_port* sp;
        slogd("Looking for port %s", port_name);
        error = check(sp_get_port_by_name(port_name, &sp));
        if (error != 0)
        {
            monocoque_serial_devices[i].open = false;
            monocoque_serial_devices[i].openfail = true;
            return -1;
        }

        slogd("Opening port");
        check(sp_open(sp, SP_MODE_READ_WRITE));

        slogd("Setting port to %i 8N1, no flow control", serialdevice->baudrate);
        check(sp_set_baudrate(sp, serialdevice->baudrate));
        check(sp_set_bits(sp, 8));
        check(sp_set_parity(sp, SP_PARITY_NONE));
        check(sp_set_stopbits(sp, 1));
        check(sp_set_flowcontrol(sp, SP_FLOWCONTROL_NONE));

        monocoque_serial_devices[i].port = sp;

        monocoque_serial_devices[i].open = true;
        monocoque_serial_devices[i].openfail = false;
        monocoque_serial_devices[i].busy = false;
        monocoque_serial_devices[i].refs++;

        serial_device_num = i;

        free(port_name);
        slogd("Successfully setup arduino serial device...");
    }

    return serial_device_num;
}
