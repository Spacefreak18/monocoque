#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "arduino.h"
#include "arduinoledlua.h"
#include "../serialadapter.h"
#include "../../slog/slog.h"

#define arduino_timeout 9000

int arduino_check(enum sp_return result)
{
    char* error_message;

    switch (result)
    {
        case SP_ERR_ARG:
            return 1;
        case SP_ERR_FAIL:
            error_message = sp_last_error_message();
            sloge("error: serial write failed: %s", error_message);
            sp_free_error_message(error_message);
        case SP_ERR_SUPP:
            printf("Error: Not supported.\n");
        case SP_ERR_MEM:
            printf("Error: Couldn't allocate memory.\n");
        case SP_OK:
        default:
            return result;
    }
}

int arduino_update(SerialDevice* serialdevice, void* data, size_t size)
{
    int result = 1;
    slogt("copying %i bytes to arduino device", size);
    result = monocoque_serial_write(serialdevice->id, data, size, arduino_timeout);

    return result;
}

int arduino_init(SerialDevice* serialdevice, const char* portdev)
{
    serialdevice->id = monocoque_serial_open(serialdevice, portdev);
    return serialdevice->id;
}

int arduino_customled_init(SerialDevice* serialdevice, const char* portdev, const char* luafile)
{
    serialdevice->id = monocoque_serial_open(serialdevice, portdev);

    size_t bufsize1 = 11;
    size_t recv_bufsize1 = 5;
    char recv_buf1[recv_bufsize1];
    char bytes1[bufsize1];

    for(int j = 0; j < bufsize1; j++)
    {
        bytes1[j] = 0x00;
    }
    bytes1[0] = 0xff;
    bytes1[1] = 0xff;
    bytes1[2] = 0xff;
    bytes1[3] = 0xff;
    bytes1[4] = 0xff;
    bytes1[5] = 0xff;
    bytes1[6] = 0x6c;
    bytes1[7] = 0x65;
    bytes1[8] = 0x64;
    bytes1[9] = 0x73;
    bytes1[10] = 0x63;

    int result = 0;
    unsigned int timeout = 3000;
    result = monocoque_serial_write_block(serialdevice->id, &bytes1, bufsize1, timeout);
    result = monocoque_serial_read_block(serialdevice->id, &recv_buf1, recv_bufsize1, timeout);
    //slogi("wrote %i bytes", result);
    //sleep(2);
    //monocoque_serial_device monocoque_serial_dev = monocoque_serial_devices[serialdevice->id];
    //result = arduino_check(sp_blocking_read(monocoque_serial_dev.port, &recv_buf1, recv_bufsize1, 5000));
    //slogi("read %i bytes", result);
    //result = sp_blocking_read(serialdevice->port, &recv_buf1, recv_bufsize1, timeout);

    char numstr[recv_bufsize1];
    for(int j = 0; j < recv_bufsize1; j++)
    {
        numstr[j] = '\0';
    }
    for(int j = 0; j < recv_bufsize1; j++)
    {
        if(recv_buf1[j] != 0 && recv_buf1[j] != 0x0d && recv_buf1[j] != 0x0a)
        {
            numstr[j] = recv_buf1[j];
        }
    }
    int numlights = atoi(numstr);
    serialdevice->numleds = numlights;
    slogi("numlights is %i\n", numlights);

    if(luafile == NULL)
    {
        return serialdevice->id;
    }

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    int top=lua_gettop(L);
    int status = luaL_loadfile(L, luafile);

    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        lua_close(serialdevice->m.L);
        return -1;
        exit(1);
    }
    lua_setglobal(L,"myFunc");

    serialdevice->m.L = L;

    return serialdevice->id;
}


int arduino_simled_update(SerialDevice* serialdevice, SimData* simdata)
{
    int result = 1;

    int total_leds = serialdevice->numleds;
    size_t bufsize = (total_leds * 3) + 14;
    char bytes[bufsize];
    int endled = serialdevice->endled;
    int startled = serialdevice->startled;
    if(endled == 0)
    {
        endled = total_leds;
    }
    int num_avail_leds = endled - startled + 1;
    int rpm = simdata->rpms;
    int maxrpm = simdata->maxrpm;
    int litleds = 0;
    if(rpm > 0 && maxrpm > 0)
    {
        int rpmmargin = ceil(.05*maxrpm);
        int rpminterval = (maxrpm-rpmmargin) / (num_avail_leds);


        for (int l = 1; l <= (num_avail_leds); l++)
        {
            if(rpm >= (rpminterval * l))
            {
                litleds = l;
            }
        }



        for(int j = 0; j < bufsize; j++)
        {
            bytes[j] = 0x00;
        }
        bytes[0] = 0xff;
        bytes[1] = 0xff;
        bytes[2] = 0xff;
        bytes[3] = 0xff;
        bytes[4] = 0xff;
        bytes[5] = 0xff;
        bytes[6] = 0x73;
        bytes[7] = 0x6c;
        bytes[8] = 0x65;
        bytes[9] = 0x64;
        bytes[10] = 0x73;
        bytes[bufsize-1] = 0xfd;
        bytes[bufsize-2] = 0xfe;
        bytes[bufsize-3] = 0xff;

        for(int i = 0; i < litleds; i++)
        {
            if(i < ((num_avail_leds) / 2))
            {
                //green
                bytes[11 + ((i + startled - 1) * 3) + 1] = 0xff;
            }
            else
            {
                if(i < num_avail_leds - 1)
                {
                    //yellow
                    bytes[11 + ((i + startled - 1) * 3) + 0] = 0xff;
                    bytes[11 + ((i + startled - 1) * 3) + 1] = 0xff;
                }
            }
            if(i == num_avail_leds - 1)
            {
                //red
                bytes[11 + ((i + startled - 1) * 3) + 0] = 0xff;
            }
        }
    }

    slogt("Updating arduino device lights to %i", litleds);
    // we can add configs to set all the colors
    size_t size = sizeof(bytes);

    result = monocoque_serial_write(serialdevice->id, &bytes, size, arduino_timeout);

    return result;
}

int arduino_customled_update(SerialDevice* serialdevice, SimData* simdata)
{
    int result = 1;

    int total_leds = serialdevice->numleds;
    size_t bufsize = (total_leds * 3) + 14;
    char ledbytes[total_leds * 3];
    char bytes[bufsize];

    for(int j = 0; j < bufsize; j++)
    {
        bytes[j] = 0x00;
    }
    for(int j = 0; j < total_leds * 3; j++)
    {
        ledbytes[j] = 0x00;
    }
    bytes[0] = 0xff;
    bytes[1] = 0xff;
    bytes[2] = 0xff;
    bytes[3] = 0xff;
    bytes[4] = 0xff;
    bytes[5] = 0xff;
    bytes[6] = 0x73;
    bytes[7] = 0x6c;
    bytes[8] = 0x65;
    bytes[9] = 0x64;
    bytes[10] = 0x73;
    bytes[bufsize-1] = 0xfd;
    bytes[bufsize-2] = 0xfe;
    bytes[bufsize-3] = 0xff;


    lua_State* L = serialdevice->m.L;

    lua_pushstring(L, "buff");
    lua_pushlightuserdata(L, &ledbytes);
    lua_settable(L, LUA_REGISTRYINDEX);

    simdata_to_lua(L, simdata);
    lua_setglobal(L, "simdata");

    lua_pushinteger(L, total_leds);
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

    for(int i = 0; i < total_leds; i++)
    {
        bytes[11 + (i * 3) + 0] = ledbytes[(i * 3) + 0];
        bytes[11 + (i * 3) + 1] = ledbytes[(i * 3) + 1];
        bytes[11 + (i * 3) + 2] = ledbytes[(i * 3) + 2];
    }

    size_t size = sizeof(bytes);
    result = arduino_update(serialdevice, &bytes, size);

    slogt("custom led wrote %i bytes", result);

    return result;
}

int arduino_customled_free(SerialDevice* serialdevice, bool lua)
{
    size_t bufsize = (serialdevice->numleds * 3) + 14;
    char bytes[bufsize];
    int endled = serialdevice->endled;
    int startled = serialdevice->startled;
    if(endled == 0)

    for(int j = 0; j < bufsize; j++)
    {
        bytes[j] = 0x00;
    }
    bytes[0] = 0xff;
    bytes[1] = 0xff;
    bytes[2] = 0xff;
    bytes[3] = 0xff;
    bytes[4] = 0xff;
    bytes[5] = 0xff;
    bytes[6] = 0x73;
    bytes[7] = 0x6c;
    bytes[8] = 0x65;
    bytes[9] = 0x64;
    bytes[10] = 0x73;
    bytes[bufsize-1] = 0xfd;
    bytes[bufsize-2] = 0xfe;
    bytes[bufsize-3] = 0xff;

    for(int i = 0; i < serialdevice->numleds; i++)
    {
        bytes[11 + (i * 3) + 0] = 0x00;
        bytes[11 + (i * 3) + 1] = 0x00;
        bytes[11 + (i * 3) + 2] = 0x00;
    }
    size_t size = sizeof(bytes);

    int result = monocoque_serial_write_block(serialdevice->id, &bytes, size, arduino_timeout);

    lua_close(serialdevice->m.L);

    return result;
}
