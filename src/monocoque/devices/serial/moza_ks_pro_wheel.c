#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <hidapi/hidapi.h>

#include "moza.h"
#include "moza_ks_pro_wheel.h"
#include "../serialadapter.h"
#include "../../slog/slog.h"

#define MOZA_TIMEOUT 1000
#define MOZA_MAGIC_VALUE 0x0d
#define MOZA_RPM_MASK_TEMPLATE {0x7e, 0x06, 0x3f, 0x17, 0x1a, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}
#define MOZA_RPM_MASK_PAYLOAD_SIZE 11
#define MOZA_RPM_COLOR_PAYLOAD_1 {0x7e, 0x16, 0x3f, 0x17, 0x19, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0xff, 0, 0, 4, 0xff, 0, 0, 0}
#define MOZA_RPM_COLOR_PAYLOAD_2 {0x7e, 0x16, 0x3f, 0x17, 0x19, 0, 5, 0xff, 0, 0, 6, 0xff, 0, 0, 7, 0xff, 0, 0, 8, 0xff, 0, 0, 9, 0xff, 0, 0, 0}
#define MOZA_RPM_COLOR_PAYLOAD_3 {0x7e, 0x16, 0x3f, 0x17, 0x19, 0, 10, 0xff, 0x7f, 0, 11, 0xff, 0x7f, 0, 12, 0xff, 0x7f, 0, 13, 0, 0, 0xff, 14, 0, 0, 0xff, 0}
#define MOZA_BTN_COLOR_PAYLOAD_1 {0x7e, 0x16, 0x3f, 0x17, 0x19, 1, 0, 0xff, 0, 0, 1, 0xff, 0, 0, 2, 0xff, 0, 0, 3, 0xff, 0, 0, 4, 0xff, 0, 0, 0}
#define MOZA_BTN_COLOR_PAYLOAD_2 {0x7e, 0x16, 0x3f, 0x17, 0x19, 1, 5, 0xff, 0, 0, 6, 0xff, 0, 0, 7, 0xff, 0, 0, 8, 0xff, 0, 0, 9, 0xff, 0, 0, 0}
#define MOZA_COLOR_PAYLOAD_SIZE 27
#define MOZA_FLAG_COLOR_YELLOW_LEFT  {0x7e, 0x16, 0x3f, 0x17, 0x19, 0, 0, 0xff, 0xaa, 0, 1, 0xff, 0xaa, 0, 2, 0xff, 0xaa, 0, 0, 0xff, 0xaa, 0, 1, 0xff, 0xaa, 0, 0}
#define MOZA_FLAG_COLOR_YELLOW_RIGHT {0x7e, 0x16, 0x3f, 0x17, 0x19, 0, 15, 0xff, 0xaa, 0, 16, 0xff, 0xaa, 0, 17, 0xff, 0xaa, 0, 15, 0xff, 0xaa, 0, 16, 0xff, 0xaa, 0, 0}
#define MOZA_FLAG_COLOR_OFF_LEFT     {0x7e, 0x16, 0x3f, 0x17, 0x19, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}
#define MOZA_FLAG_COLOR_OFF_RIGHT    {0x7e, 0x16, 0x3f, 0x17, 0x19, 0, 15, 0, 0, 0, 16, 0, 0, 0, 17, 0, 0, 0, 15, 0, 0, 0, 16, 0, 0, 0, 0}
#define BIT(nr) (1UL << (nr))

int moza_ks_pro_wheel_update(SerialDevice* serialdevice, SimData* simData)
{
    static uint8_t last_flag = 0xff;
    unsigned char bytes[] = MOZA_RPM_MASK_TEMPLATE;
    int size = MOZA_RPM_MASK_PAYLOAD_SIZE;
    float perctflt = ((float)simData->rpms/(float)simData->maxrpm)*100;
    int perct = round(perctflt);
    if (perct >= 98 && (simData->mtick >> 7) & 1 == 1) perct = 0;


    if (perct >= 75)
        bytes[6] |= BIT(3);

    if (perct >= 77)
        bytes[6] |= BIT(4);

    if (perct >= 79)
        bytes[6] |= BIT(5);

    if (perct >= 81)
        bytes[6] |= BIT(6);

    if (perct >= 83)
        bytes[6] |= BIT(7);

    if (perct >= 85)
        bytes[7] |= BIT(0);

    if (perct >= 87)
        bytes[7] |= BIT(1);

    if (perct >= 89)
        bytes[7] |= BIT(2);

    if (perct >= 91)
        bytes[7] |= BIT(3);

    if (perct >= 93)
        bytes[7] |= BIT(4);

    if (perct >= 95)
        bytes[7] |= BIT(5);

    if (perct >= 97)
        bytes[7] |= BIT(6);


    if (simData->playerflag == SIMAPI_FLAG_YELLOW) {
        bytes[6] |= BIT(0) | BIT(1) | BIT(2);  // left 3 flag LEDs (indices 0, 1, 2)
        bytes[7] |= BIT(7);                      // right flag LED (index 15)
        bytes[8] |= BIT(0) | BIT(1);            // right flag LEDs (indices 16, 17)
    }

    bytes[10] = moza_checksum(bytes, size);

    int result = 1;
    if (serialdevice->port)
    {
        if (simData->playerflag != last_flag) {
            last_flag = simData->playerflag;
            if (simData->playerflag == SIMAPI_FLAG_YELLOW) {
                unsigned char fl[] = MOZA_FLAG_COLOR_YELLOW_LEFT;
                unsigned char fr[] = MOZA_FLAG_COLOR_YELLOW_RIGHT;
                fl[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(fl, MOZA_COLOR_PAYLOAD_SIZE);
                fr[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(fr, MOZA_COLOR_PAYLOAD_SIZE);
                monocoque_serial_write(serialdevice->id, fl, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);
                monocoque_serial_write(serialdevice->id, fr, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);
            } else {
                unsigned char fl[] = MOZA_FLAG_COLOR_OFF_LEFT;
                unsigned char fr[] = MOZA_FLAG_COLOR_OFF_RIGHT;
                fl[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(fl, MOZA_COLOR_PAYLOAD_SIZE);
                fr[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(fr, MOZA_COLOR_PAYLOAD_SIZE);
                monocoque_serial_write(serialdevice->id, fl, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);
                monocoque_serial_write(serialdevice->id, fr, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);
            }
        }

        slogd("copying %i bytes to moza device", MOZA_RPM_MASK_PAYLOAD_SIZE);
        slogt("writing bytes %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x from rpm %i maxrpm %i", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], bytes[8], bytes[9], bytes[10], simData->rpms, simData->maxrpm);
        result = monocoque_serial_write(serialdevice->id, bytes, size, MOZA_TIMEOUT);
    }

    return result;
}

int moza_ks_pro_wheel_init(SerialDevice* serialdevice, const char* portdev)
{
    serialdevice->id = monocoque_serial_open(serialdevice, portdev);
    if (serialdevice->id == -1) return serialdevice->id;

    unsigned char p1[] = MOZA_RPM_COLOR_PAYLOAD_1;
    unsigned char p2[] = MOZA_RPM_COLOR_PAYLOAD_2;
    unsigned char p3[] = MOZA_RPM_COLOR_PAYLOAD_3;
    unsigned char p4[] = MOZA_BTN_COLOR_PAYLOAD_1;
    unsigned char p5[] = MOZA_BTN_COLOR_PAYLOAD_2;

    p1[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(p1, MOZA_COLOR_PAYLOAD_SIZE);
    p2[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(p2, MOZA_COLOR_PAYLOAD_SIZE);
    p3[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(p3, MOZA_COLOR_PAYLOAD_SIZE);
    p4[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(p4, MOZA_COLOR_PAYLOAD_SIZE);
    p5[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(p5, MOZA_COLOR_PAYLOAD_SIZE);

    monocoque_serial_write(serialdevice->id, p1, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);
    monocoque_serial_write(serialdevice->id, p2, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);
    monocoque_serial_write(serialdevice->id, p3, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);
    monocoque_serial_write(serialdevice->id, p4, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);
    monocoque_serial_write(serialdevice->id, p5, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);

    unsigned char p6[] = MOZA_FLAG_COLOR_OFF_RIGHT;
    p6[MOZA_COLOR_PAYLOAD_SIZE-1] = moza_checksum(p6, MOZA_COLOR_PAYLOAD_SIZE);
    monocoque_serial_write(serialdevice->id, p6, MOZA_COLOR_PAYLOAD_SIZE, MOZA_TIMEOUT);

    return serialdevice->id;
}
