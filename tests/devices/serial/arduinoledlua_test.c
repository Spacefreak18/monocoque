#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdint.h>
#include "../../../src/monocoque/devices/serial/arduinoledlua.h"
extern uint8_t get_color_rgb_value(int color, int rgb);
static void primary_colors_map_to_rgb(void **state)
{
    (void)state;
    assert_int_equal(get_color_rgb_value(LUALEDCOLOR_RED, 0), 255);
    assert_int_equal(get_color_rgb_value(LUALEDCOLOR_RED, 1), 0);
    assert_int_equal(get_color_rgb_value(LUALEDCOLOR_GREEN, 1), 255);
    assert_int_equal(get_color_rgb_value(LUALEDCOLOR_BLUE, 2), 255);
}
static void telemetry_is_exported_to_lua(void **state)
{
    (void)state;
    lua_State *L = luaL_newstate();
    assert_non_null(L);
    SimData data = {0};
    data.rpms = 3500;
    data.maxrpm = 7000;
    assert_int_equal(simdata_to_lua(L, &data), 0);
    lua_getfield(L, -1, "rpm");
    assert_int_equal(lua_tointeger(L, -1), 3500);
    lua_pop(L, 1);
    lua_getfield(L, -1, "maxrpm");
    assert_int_equal(lua_tointeger(L, -1), 7000);
    lua_close(L);
}
int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(primary_colors_map_to_rgb),
        cmocka_unit_test(telemetry_is_exported_to_lua),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
