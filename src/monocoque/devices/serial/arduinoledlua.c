#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include "arduinoledlua.h"
#include "arduino.h"

#include "../../slog/slog.h"

long long ledTimeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}


int simdata_to_lua(lua_State *L, SimData* simdata) {

    // make the time up now if we are running in test mode
    if(simdata->mtick == 0)
    {
        simdata->mtick = ledTimeInMilliseconds();
    }

    lua_newtable(L);

    lua_pushinteger(L, simdata->playerflag);
    lua_setfield(L, -2, "playerflag");

    lua_pushinteger(L, simdata->rpms);
    lua_setfield(L, -2, "rpm");

    lua_pushinteger(L, simdata->mtick);
    lua_setfield(L, -2, "mtick");

    lua_pushinteger(L, simdata->maxrpm);
    lua_setfield(L, -2, "maxrpm");

    lua_pushinteger(L, PROXCARS);
    lua_setfield(L, -2, "proxcars");

    lua_newtable(L);
    for(int i = 0; i < PROXCARS; i++)
    {
        lua_newtable(L);
        lua_pushinteger(L, simdata->pd[i].radius);
        lua_setfield(L, -2, "radius");
        lua_pushinteger(L, simdata->pd[i].theta);
        lua_setfield(L, -2, "theta");
        lua_rawseti(L, -2, i+1);
    }
    lua_setfield(L, -2, "pd");

    return 0; // Return the table to Lua
}

uint8_t get_color_rgb_value(int color, int rgb)
{
    switch (color)
    {
        case LUALEDCOLOR_RED:
            switch (rgb)
            {
                case 0:
                    return 255;
                case 1:
                    return 0;
                case 2:
                    return 0;
            }
        case LUALEDCOLOR_GREEN:
            switch (rgb)
            {
                case 0:
                    return 0;
                case 1:
                    return 255;
                case 2:
                    return 0;
            }
        case LUALEDCOLOR_BLUE:
            switch (rgb)
            {
                case 0:
                    return 0;
                case 1:
                    return 0;
                case 2:
                    return 255;
            }
        case LUALEDCOLOR_YELLOW:
            switch (rgb)
            {
                case 0:
                    return 255;
                case 1:
                    return 255;
                case 2:
                    return 0;
            }
        case LUALEDCOLOR_ORANGE:
            switch (rgb)
            {
                case 0:
                    return 255;
                case 1:
                    return 165;
                case 2:
                    return 0;
            }
        default:
            return 0;
    }
}

int set_led_range_to_color(lua_State *L)
{

    slogt("lua called c function set_led_range_to_color");

    int range_start = lua_tonumber(L, 1);
    int range_end = lua_tonumber(L, 2);
    int color = lua_tonumber(L, 3);

    slogd("lua range start is %i", range_start);
    slogd("lua range end is %i", range_end);
    slogd("lua color is %i", color);

    range_start = range_start - 1;

    lua_getglobal(L, "TotalLeds");
    int numlights = 0;
    if (lua_isnumber(L, -1))
    {
        numlights = lua_tonumber(L, -1);
    }
    slogd("num leds is %i", numlights);

    if(range_end > numlights)
    {
        range_end = numlights;
    }

    if(range_end == 0)
    {
        slogt("Invalid range, doing nothing");
        return 1;
    }

    lua_pushstring(L, "buff");
    lua_gettable(L, LUA_REGISTRYINDEX);
    char* bytes = lua_touserdata(L, -1);

    slogt("first byte of buff is x%02x", bytes[0]);

    uint8_t color0 = get_color_rgb_value(color, 0);
    uint8_t color1 = get_color_rgb_value(color, 1);
    uint8_t color2 = get_color_rgb_value(color, 2);

    for( int i = range_start; i < range_end; i++)
    {
        bytes[(i * 3) + 0] = color0;
        bytes[(i * 3) + 1] = color1;
        bytes[(i * 3) + 2] = color2;
    }

}

int set_led_range_to_rgb_color(lua_State *L)
{

    slogt("lua called c function set_led_range_to_rgb_color");

    int range_start = lua_tonumber(L, 1);
    int range_end = lua_tonumber(L, 2);
    int color = lua_tonumber(L, 3);

    uint8_t color0 = (color >> 16) & 0xff;
    uint8_t color1 = (color >> 8) & 0xff;
    uint8_t color2 = (color >> 0) & 0xff;
    //int color0 = lua_tonumber(L, 3);
    //int color1 = lua_tonumber(L, 4);
    //int color2 = lua_tonumber(L, 5);

    slogd("lua range start is %i", range_start);
    slogd("lua range end is %i", range_end);
    slogd("lua color0 is %i", color0);
    slogd("lua color1 is %i", color1);
    slogd("lua color2 is %i", color2);

    range_start = range_start - 1;

    lua_getglobal(L, "TotalLeds");
    int numlights = 0;
    if (lua_isnumber(L, -1))
    {
        numlights = lua_tonumber(L, -1);
    }
    slogd("num leds is %i", numlights);

    if(range_end > numlights)
    {
        range_end = numlights;
    }

    if(range_end == 0)
    {
        return 1;
    }

    lua_pushstring(L, "buff");
    lua_gettable(L, LUA_REGISTRYINDEX);
    char* bytes = lua_touserdata(L, -1);

    slogt("first byte of buff is x%02x", bytes[0]);

    for( int i = range_start; i < range_end; i++)
    {
        bytes[(i * 3) + 0] = color0;
        bytes[(i * 3) + 1] = color1;
        bytes[(i * 3) + 2] = color2;
    }

}

int set_led_to_color(lua_State *L)
{
    slogt("lua called c function set_led_to_rgb_color");

    int led = lua_tonumber(L, 1);
    int color = lua_tonumber(L, 2);

    slogd("lua led is %i", led);
    slogd("lua color is %i", color);

    led = led - 1;

    lua_getglobal(L, "TotalLeds");
    int numlights = 0;
    if (lua_isnumber(L, -1))
    {
        numlights = lua_tonumber(L, -1);
    }
    slogd("num leds is %i", numlights);

    if(led > numlights)
    {
        return 1;
    }

    lua_pushstring(L, "buff");
    lua_gettable(L, LUA_REGISTRYINDEX);
    char* bytes = lua_touserdata(L, -1);

    slogt("first byte of buff is x%02x", bytes[0]);

    uint8_t color0 = get_color_rgb_value(color, 0);
    uint8_t color1 = get_color_rgb_value(color, 1);
    uint8_t color2 = get_color_rgb_value(color, 2);

    bytes[(led * 3) + 0] = color0;
    bytes[(led * 3) + 1] = color1;
    bytes[(led * 3) + 2] = color2;

}


int set_led_to_rgb_color(lua_State *L)
{
    slogt("lua called c function set_led_to_rgb_color");

    int led = lua_tonumber(L, 1);
    int color = lua_tonumber(L, 2);

    uint8_t color0 = (color >> 16) & 0xff;
    uint8_t color1 = (color >> 8) & 0xff;
    uint8_t color2 = (color >> 0) & 0xff;

    slogd("lua led is %i", led);
    slogd("lua color0 is %i", color0);
    slogd("lua color1 is %i", color1);
    slogd("lua color2 is %i", color2);

    led = led - 1;

    lua_getglobal(L, "TotalLeds");
    int numlights = 0;
    if (lua_isnumber(L, -1))
    {
        numlights = lua_tonumber(L, -1);
    }
    slogd("num leds is %i", numlights);

    if(led > numlights)
    {
        return 1;
    }

    lua_pushstring(L, "buff");
    lua_gettable(L, LUA_REGISTRYINDEX);
    char* bytes = lua_touserdata(L, -1);

    slogt("first byte of buff is x%02x", bytes[0]);

    bytes[(led * 3) + 0] = color0;
    bytes[(led * 3) + 1] = color1;
    bytes[(led * 3) + 2] = color2;

}


int led_clear_all(lua_State *L)
{

    slogt("lua called c function led_clear_all");

    lua_getglobal(L, "TotalLeds");
    int numlights = 0;
    if (lua_isnumber(L, -1))
    {
        numlights = lua_tonumber(L, -1);
    }
    slogd("num leds is %i", numlights);

    lua_pushstring(L, "buff");
    lua_gettable(L, LUA_REGISTRYINDEX);
    char* bytes = lua_touserdata(L, -1);

    slogt("first byte of buff is x%02x", bytes[0]);

    for( int i = 0; i < numlights; i++)
    {
        bytes[(i * 3) + 0] = 0x00;
        bytes[(i * 3) + 1] = 0x00;
        bytes[(i * 3) + 2] = 0x00;
    }

}
