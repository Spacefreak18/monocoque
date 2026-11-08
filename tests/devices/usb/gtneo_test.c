#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../../../src/monocoque/devices/usb/wheels/gtneo.h"

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
extern void prepare_device(hid_device *device);
static void prepare_report_has_protocol_header(void **state)
{
    (void)state;
    hid_calls = 0;
    hid_result = SIMAGIC_MAX_PAYLOAD_SIZE;
    prepare_device((hid_device *)(uintptr_t)1);
    assert_int_equal(hid_calls, 1);
    assert_int_equal(report_length, SIMAGIC_MAX_PAYLOAD_SIZE);
    assert_int_equal(report[0], SIMAGIC_LED_REPORT_ID);
    assert_int_equal(report[6], SIMAGIC_LED_DESCRIPTOR_LED_CONTROL);
    assert_int_equal(report[7], SIMAGIC_LED_DESCRIPTOR_PREPARE_LEDS);
}
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(prepare_report_has_protocol_header),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
