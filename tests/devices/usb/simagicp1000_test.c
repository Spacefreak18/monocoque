#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../../../src/monocoque/devices/usb/simagicp1000.h"

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

static void slip_effect_sends_feature_report(void **state)
{
    (void)state;
    USBDevice dev = {0};
    dev.handle = (hid_device *)(uintptr_t)1;
    hid_result = 49;
    hid_calls = 0;
    (void)simagicp1000_update(&dev, EFFECT_TYRESLIP, 1);
    assert_int_equal(hid_calls, 1);
    assert_int_equal(report_length, 49);
    assert_int_equal(report[0], 241);
    assert_int_equal(report[1], 0xEC);
    assert_int_equal(report[2], 2);
}
static void abs_effect_sends_two_reports(void **state)
{
    (void)state;
    USBDevice dev = {0};
    dev.handle = (hid_device *)(uintptr_t)1;
    hid_result = 49;
    hid_calls = 0;
    (void)simagicp1000_update(&dev, EFFECT_ABSBRAKES, 1);
    assert_int_equal(hid_calls, 2);
    assert_int_equal(report[2], 1);
}
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(slip_effect_sends_feature_report),
        cmocka_unit_test(abs_effect_sends_two_reports),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
