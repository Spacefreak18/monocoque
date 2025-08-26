#ifndef _ARDUINOLEDLUA_H
#define _ARDUINOLEDLUA_H

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "../../simulatorapi/simapi/simapi/simdata.h"

typedef enum {
    LUALEDCOLOR_RED = 1,
    LUALEDCOLOR_GREEN,
    LUALEDCOLOR_BLUE,
    LUALEDCOLOR_YELLOW,
    LUALEDCOLOR_ORANGE,
} LUALEDColor;

int simdata_to_lua(lua_State *L, SimData* simdata);
int set_led_range_to_color(lua_State *L);
int set_led_to_color(lua_State *L);
int set_led_range_to_rgb_color(lua_State *L);
int set_led_to_rgb_color(lua_State *L);
int led_clear_all(lua_State *L);
int led_clear_range(lua_State *L);

#endif
