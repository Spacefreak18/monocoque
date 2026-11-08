#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../../../src/monocoque/devices/serial/moza_ks_pro_wheel.h"

#include "../../../src/monocoque/devices/serialadapter.h"
static int open_result;
static int write_result;
static int writes;
static unsigned char packet[128];
static size_t packet_size;
int monocoque_serial_open(SerialDevice *dev, const char *port)
{
    (void)dev;
    assert_string_equal(port, "test-port");
    return open_result;
}
int monocoque_serial_write(uint8_t id, void *data, size_t size, int timeout)
{
    (void)id;
    assert_true(timeout > 0);
    assert_true(size <= sizeof(packet));
    memcpy(packet, data, size);
    packet_size = size;
    writes++;
    return write_result;
}
unsigned char moza_checksum(unsigned char *data, unsigned short size)
{
    unsigned int sum = 13;
    for (unsigned short i = 0; i < size; i++) sum += data[i];
    return (unsigned char)sum;
}
static void init_fails_when_port_open_fails(void **state)
{
    (void)state;
    SerialDevice dev = {0};
    open_result = -3;
    writes = 0;
    assert_int_equal(moza_ks_pro_wheel_init(&dev, "test-port"), -3);
    assert_int_equal(writes, 0);
}
static void init_success(void **state)
{
    (void)state;
    SerialDevice dev = {0};
    open_result = 4;
    write_result = 0;
    writes = 0;
    assert_int_equal(moza_ks_pro_wheel_init(&dev, "test-port"), 0);
    assert_int_equal(dev.id, 4);
}
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(init_fails_when_port_open_fails),
        cmocka_unit_test(init_success),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
