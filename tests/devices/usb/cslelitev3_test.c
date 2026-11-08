#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../../src/monocoque/devices/usb/cslelitev3.h"
static void slip_effect_writes_value(void **state)
{
    (void)state;
    USBDevice dev = {0};
    char output[32] = {0};
    dev.filehandle = tmpfile();
    assert_non_null(dev.filehandle);
    assert_int_equal(cslelitev3_update(&dev, EFFECT_TYRESLIP, 1), 0);
    rewind(dev.filehandle);
    assert_non_null(fgets(output, sizeof(output), dev.filehandle));
    assert_string_equal(output, "16711680\n");
    fclose(dev.filehandle);
}
static void stopped_effect_writes_zero(void **state)
{
    (void)state;
    USBDevice dev = {0};
    char output[32] = {0};
    dev.filehandle = tmpfile();
    assert_non_null(dev.filehandle);
    assert_int_equal(cslelitev3_update(&dev, EFFECT_TYRESLIP, 0), 0);
    rewind(dev.filehandle);
    assert_non_null(fgets(output, sizeof(output), dev.filehandle));
    assert_string_equal(output, "0\n");
    fclose(dev.filehandle);
}
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(slip_effect_writes_value),
        cmocka_unit_test(stopped_effect_writes_zero),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
