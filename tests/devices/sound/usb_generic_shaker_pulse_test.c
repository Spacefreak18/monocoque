#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <pulse/pulseaudio.h>
#include "../../../src/monocoque/devices/simdevice.h"
extern void gear_sound_stream(pa_stream *s, size_t length, void *userdata);
static int writes;
static int16_t captured[64];
int pa_stream_write(pa_stream *s, const void *data, size_t length,
                    pa_free_cb_t free_cb, int64_t offset, pa_seek_mode_t seek)
{
    (void)s; (void)free_cb; (void)offset;
    assert_int_equal(seek, PA_SEEK_RELATIVE);
    assert_true(length <= sizeof(captured));
    memcpy(captured, data, length);
    writes++;
    return 0;
}
static void muted_stream_writes_silence(void **state)
{
    (void)state;
    SoundData data = {0};
    writes = 0;
    memset(captured, 0xff, sizeof(captured));
    gear_sound_stream((pa_stream *)(uintptr_t)1, sizeof(captured), &data);
    assert_int_equal(writes, 1);
    for (size_t i = 0; i < sizeof(captured)/sizeof(captured[0]); i++)
        assert_int_equal(captured[i], 0);
}
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(muted_stream_writes_silence),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
