#include <stdio.h>
#include <math.h>

#include <hidapi/hidapi.h>

#include "cammusc12.h"
#include "../../serial/arduinoledlua.h"
#include "../../../slog/slog.h"

const int cammusc12_hidupdate_buf_size = 16;
const int cammusc12_hidledupdate_buf_size = 7;

const int cammusc12_total_leds = 15;

int cammusc12_update(USBDevice* usbdevice, int maxrpm, int rpm, int gear, int velocity)
{

    int res = 0;

    unsigned char bytes[cammusc12_hidupdate_buf_size];

    for (int x = 0; x < cammusc12_hidupdate_buf_size; x++)
    {
        bytes[x] = 0x00;
    }
    // byte 1 must be fc it seems
    bytes[0] = 0xFA;
    bytes[1] = 0xFB;
    bytes[2] = 0xD4;

    int perct = 0;
    if(rpm > 0 && maxrpm > 0)
    {
        double rpmperct = (double) rpm / (double) maxrpm;
        perct = trunc(nearbyint( rpmperct * 100 ));
    }
    bytes[3] = perct;

    // bytes 2 and 3 are a 16 bit velocity
    if ( velocity > 0 )
    {
        bytes[5] = (velocity >> 8) & 0xFF;
        bytes[4] = velocity & 0xFF;
    }

    // byte 4 is gear
    bytes[6] = gear-1;

    slogt("writing bytes x%02xx%02xx%02xx%02xx%02x%02x%02x from rpm %i velocity %i gear %i", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], rpm, velocity, gear);
    if (usbdevice->handle)
    {
        res = hid_write(usbdevice->handle, bytes, cammusc12_hidupdate_buf_size);
    }
    else
    {
        slogd("no handle");
    }

    return res;
}

int cammusc12_free(USBDevice* wheeldevice)
{
    int res = 0;

    hid_close(wheeldevice->handle);
    res = hid_exit();

    return res;
}

int cammusc12_init_(USBDevice* wheeldevice)
{
    slogi("initializing cammus c12 wheel...");

    int res = 0;

    res = hid_init();

    wheeldevice->handle = hid_open(0x3416, 0x1023, NULL);

    if (!wheeldevice->handle)
    {
        sloge("Could not find attached Cammus C12 Wheel");
        res = hid_exit();
        return 1;
    }
    slogd("Found Cammus C12 Wheel...");
    return res;
}

int cammusc12_init(USBDevice* wheeldevice, const char* luafile)
{

    int res = cammusc12_init_(wheeldevice);
    if(luafile == NULL)
    {
        return res;
    }

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    int top=lua_gettop(L);
    int status = luaL_loadfile(L, luafile);

    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        sloge("There is an issue with your lua script");
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        //exit(1);
    }

    lua_setglobal(L,"myFunc");

    wheeldevice->m.L = L;

    return res;
}


int cammusc12_customled_update(USBDevice* usbdevice, SimData* simdata)
{
    int result = 1;


    size_t bufsize = (cammusc12_total_leds * 3);
    char ledbytes[bufsize];


    lua_State* L = usbdevice->m.L;

    lua_pushstring(L, "buff");
    lua_pushlightuserdata(L, &ledbytes);
    lua_settable(L, LUA_REGISTRYINDEX);

    simdata_to_lua(L, simdata);
    lua_setglobal(L, "simdata");

    lua_pushinteger(L, cammusc12_total_leds);
    lua_setglobal(L, "TotalLeds");

    lua_register(L, "set_led_to_color", set_led_to_color);
    lua_register(L, "set_led_range_to_color", set_led_range_to_color);
    lua_register(L, "set_led_to_rgb_color", set_led_to_rgb_color);
    lua_register(L, "set_led_range_to_rgb_color", set_led_range_to_rgb_color);
    lua_register(L, "led_clear_all", led_clear_all);

    lua_pushinteger(L, LUALEDCOLOR_RED);
    lua_setglobal(L, "RED");
    lua_pushinteger(L, LUALEDCOLOR_GREEN);
    lua_setglobal(L, "GREEN");
    lua_pushinteger(L, LUALEDCOLOR_BLUE);
    lua_setglobal(L, "BLUE");
    lua_pushinteger(L, LUALEDCOLOR_YELLOW);
    lua_setglobal(L, "YELLOW");
    lua_pushinteger(L, LUALEDCOLOR_ORANGE);
    lua_setglobal(L, "ORANGE");

    lua_getglobal(L,"myFunc");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        fprintf(stderr, "Error calling Lua script: %s\n", lua_tostring(L, -1));
    }

    int res = 0;

    unsigned char bytes[cammusc12_hidledupdate_buf_size];
    for(int x = 0; x < cammusc12_hidledupdate_buf_size; x++)
    {
        bytes[x] = 0x00;
    }
    bytes[0] = 0xFA;
    bytes[1] = 0xFB;
    bytes[2] = 0x02;

    for(int i = 0; i < cammusc12_total_leds; i++)
    {
        for (int x = 3; x < cammusc12_hidledupdate_buf_size; x++)
        {
            bytes[x] = 0x00;
        }
        uint8_t led = i;
        uint8_t red = ledbytes[(i * 3) + 0];
        uint8_t green = ledbytes[(i * 3) + 1];
        uint8_t blue = ledbytes[(i * 3) + 2];

        bytes[3] = led;
        bytes[4] = red;
        bytes[5] = green;
        bytes[6] = blue;

        slogt("writing bytes x%02xx%02xx%02xx%02xx%02x%02x%02x from red %i green %i blue %i", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], red, green, blue);
        if (usbdevice->handle)
        {
            res = hid_write(usbdevice->handle, bytes, cammusc12_hidupdate_buf_size);
        }
        else
        {
            slogd("no handle");
        }
    }

    return res;
}
