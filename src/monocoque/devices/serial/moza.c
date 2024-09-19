#include <stdio.h>
#include <math.h>

#include <hidapi/hidapi.h>

#include "moza.h"
#include "../../slog/slog.h"

#define MOZA_TIMEOUT 1000
#define MOZA_SERIAL_TEMPLATE = {0x7e, 0x06, 0x41, 0x17, 0xfd, 0xde, 0x0, 0x0, 0x0, 0x0, 0x0}
#define MOZA_NUM_AVAILABLE_LEDS = 10
#define MOZA_BLINKING_BIT = 7
#define MOZA_MAGIC_VALUE = 0x0d
#define BIT(nr) (1UL << (nr))

unsigned char moza_checksum(unsigned char *data)
{
    unsigned char ret = MOZA_MAGIC_VALUE;
    for (short i = 0; i < sizeof(data)/sizeof(data[0]); i++)
    {
        ret += data[i];
    }

    return 0;
}

int moza_update(WheelDevice* wheeldevice, unsigned short maxrpm, unsigned short rpm)
{
    unsigned char bytes[] = MOZA_SERIAL_TEMPLATE;
    int size = sizeof(bytes)/sizeof(bytes[0])

    if (rpm/maxrpm >= 0.8)
        bytes[9] |= BIT(0);

    if (rpm/maxrpm >= 0.83)
        bytes[9] |= BIT(1);

    if (rpm/maxrpm >= 0.86)
        bytes[9] |= BIT(2);

    if (rpm/maxrpm >= 0.89)
        bytes[9] |= BIT(3);

    if (rpm/maxrpm >= 0.91)
        bytes[8] |= BIT(4);

    if (rpm/maxrpm >= 0.92)
        bytes[8] |= BIT(5);

    if (rpm/maxrpm >= 0.93)
        bytes[8] |= BIT(6);

    if (rpm/maxrpm >= 0.94)
        bytes[8] |= BIT(7);

    if (rpm/maxrpm >= 0.96)
        bytes[8] |= BIT(0);

    if (rpm/maxrpm >= 0.96)
        bytes[8] |= BIT(1);

    // blinking
    if (rpm/maxrpm >= 0.97)
        bytes[9] |= BIT(MOZA_BLINKING_BIT);

    bytes[10] = moza_checksum(bytes);

    int result = 1;
    if (serialdevice->port)
    {
        slogt("copying %i bytes to moza device", size);
        result = check(sp_blocking_write(serialdevice->port, bytes, size, MOZA_TIMEOUT));
    }

    return result;
}

int moza_init(SerialDevice* serialdevice, const char* portdev)
{
    slogi("initializing moza serial device...");
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

    slogd("Setting port to 115200 8N1, no flow control.\n");
    check(sp_set_baudrate(serialdevice->port, 115200));
    check(sp_set_bits(serialdevice->port, 8));
    check(sp_set_parity(serialdevice->port, SP_PARITY_NONE));
    check(sp_set_stopbits(serialdevice->port, 1));
    check(sp_set_flowcontrol(serialdevice->port, SP_FLOWCONTROL_NONE));

    free(port_name);
    slogd("Successfully setup moza serial device...");
    return 0;
}

int moza_free(WheelDevice* wheeldevice)
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
