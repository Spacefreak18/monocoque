#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <hidapi/hidapi.h>

#include "moza.h"
#include "../serialadapter.h"
#include "../../slog/slog.h"

#define MOZA_TIMEOUT 1000
#define MOZA_SERIAL_TEMPLATE {0x7e, 0x06, 0x41, 0x17, 0xfd, 0xde, 0x0, 0x0, 0x0, 0x0, 0x0}
#define MOZA_NUM_AVAILABLE_LEDS 10
#define MOZA_BLINKING_BIT 7
#define MOZA_MAGIC_VALUE 0x0d
#define BIT(nr) (1UL << (nr))

unsigned char moza_checksum(unsigned char *data)
{
    unsigned char ret = MOZA_MAGIC_VALUE;
    for (short i = 0; i < sizeof(data)/sizeof(data[0]); i++)
    {
        ret += data[i];
    }

    return ret;
}

int moza_update(SerialDevice* serialdevice, unsigned short maxrpm, unsigned short rpm)
{
    unsigned char bytes[] = MOZA_SERIAL_TEMPLATE;
    int size = sizeof(bytes)/sizeof(bytes[0]);

    float perct = (float)rpm/(float)maxrpm;

    if (perct >= 0.8)
        bytes[9] |= BIT(0);

    if (perct >= 0.83)
        bytes[9] |= BIT(1);

    if (perct >= 0.86)
        bytes[9] |= BIT(2);

    if (perct >= 0.89)
        bytes[9] |= BIT(3);

    if (perct >= 0.91)
        bytes[8] |= BIT(4);

    if (perct >= 0.92)
        bytes[8] |= BIT(5);

    if (perct >= 0.93)
        bytes[8] |= BIT(6);

    if (perct >= 0.94)
        bytes[8] |= BIT(7);

    if (perct >= 0.96)
        bytes[8] |= BIT(0);

    if (perct >= 0.96)
        bytes[8] |= BIT(1);

    // blinking
    if (perct >= 0.97)
        bytes[9] |= BIT(MOZA_BLINKING_BIT);

    bytes[10] = moza_checksum(bytes);

    int result = 1;
    if (serialdevice->port)
    {
        slogd("copying %i bytes to moza device", size);
        slogt("writing bytes %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x from rpm %i maxrpm %i", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], bytes[8], bytes[9], bytes[10], rpm, maxrpm);
        result = monocoque_serial_write(serialdevice->id, bytes, size, MOZA_TIMEOUT);
    }

    return result;
}

int moza_init(SerialDevice* serialdevice, const char* portdev)
{
    serialdevice->id = monocoque_serial_open(serialdevice, portdev);
    return serialdevice->id;
}
