#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../../../src/monocoque/devices/usb/wheels/cammusc12.h"

#include <hidapi/hidapi.h>
static int hid_result;
static int hid_calls;
static unsigned char report[128];
static size_t report_length;
int hid_write(hid_device *device, const unsigned char *data, size_t length)
{
    assert_non_null(device);
    assert_true(length <= sizeof(report));
    memcpy(report, data, length);
    report_length = length;
    hid_calls++;
    return hid_result;
}
int hid_send_feature_report(hid_device *device, const unsigned char *data, size_t length)
{
    return hid_write(device, data, length);
}

static int closed;
int hid_init(void) { return 0; }
hid_device *hid_open(unsigned short vendor_id, unsigned short product_id,
                     const wchar_t *serial_number)
{
    assert_int_equal(vendor_id, 0x3416);
    assert_int_equal(product_id, 0x1023);
    assert_null(serial_number);
    return (hid_device *)(uintptr_t)1;
}
void hid_close(hid_device *device)
{
    assert_non_null(device);
    closed++;
}
int hid_exit(void) { return 0; }
static void init_and_free_succeeds(void **state)
{
    (void)state;
    USBDevice dev = {0};
    closed = 0;
    assert_int_equal(cammusc12_init(&dev, NULL), 0);
    assert_non_null(dev.handle);
    assert_int_equal(cammusc12_free(&dev), 0);
    assert_int_equal(closed, 1);
}
static void update_encodes_expected_report(void **state)
{
    (void)state;
    USBDevice dev = {0};
    dev.handle = (hid_device *)(uintptr_t)1;
    hid_result = 16;
    hid_calls = 0;
    assert_true((cammusc12_update(&dev, 10000, 5000, 4, 258)) >= 0);
    assert_int_equal(hid_calls, 1);
    assert_int_equal(report_length, 16);
    assert_int_equal(report[0], 0xFA);
    assert_int_equal(report[3], 50);
}
static void update_propagates_hid_error(void **state)
{
    (void)state;
    USBDevice dev = {0};
    dev.handle = (hid_device *)(uintptr_t)1;
    hid_result = -7;
    hid_calls = 0;
    assert_true((cammusc12_update(&dev, 10000, 5000, 4, 258)) < 0);
    assert_int_equal(hid_calls, 1);
}
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(init_and_free_succeeds),
        cmocka_unit_test(update_encodes_expected_report),
        cmocka_unit_test(update_propagates_hid_error),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
