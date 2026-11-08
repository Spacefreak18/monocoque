#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../../../src/monocoque/devices/serial/moza.h"

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
static void init_fails_when_port_open_fails(void **state)
{
    (void)state;
    SerialDevice dev = {0};
    open_result = -3;
    writes = 0;
    assert_int_equal(moza_init(&dev, "test-port"), -3);
    assert_int_equal(writes, 0);
}
static void init_success(void **state)
{
    (void)state;
    SerialDevice dev = {0};
    open_result = 4;
    write_result = 0;
    writes = 0;
    assert_int_equal(moza_init(&dev, "test-port"), 0);
    assert_int_equal(dev.id, 4);
}
static void checksum_is_modulo_256(void **state)
{
    (void)state;
    unsigned char bytes[] = {0xff, 0x01};
    assert_int_equal(moza_checksum(bytes, sizeof(bytes)), 13);
}
static void update_encodes_rpm_leds(void **state)
{
    (void)state;
    SerialDevice dev = {0};
    dev.port = (void *)1;
    write_result = 0;
    writes = 0;
    assert_int_equal(moza_update(&dev, 5000, 10000), 0);
    assert_int_equal(writes, 1);
    assert_int_equal(packet_size, 11);
    assert_int_equal(packet[9], 0x1f);
}
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(init_fails_when_port_open_fails),
        cmocka_unit_test(init_success),
        cmocka_unit_test(checksum_is_modulo_256),
        cmocka_unit_test(update_encodes_rpm_leds),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
