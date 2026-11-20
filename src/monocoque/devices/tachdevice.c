#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "tachdevice.h"
#include "arduinoledlua.h"
#include "simdevice.h"
#include "usb/revburner.h"
#include "../helper/confighelper.h"
#include "../helper/dirhelper.h"
#include "../simulatorapi/simapi/simapi/simdata.h"
#include "../slog/slog.h"

int tachdev_update(USBDevice* usbdevice, SimData* simdata)
{
    TachDevice* tachdevice = &usbdevice->u.tachdevice;
    int pulses = 0;
    switch ( tachdevice->type )
    {
        case TACHDEV_UNKNOWN :
        case TACHDEV_REVBURNER :
            //this can go to revburner.c if it is too specific
            slogt("getting pulses from lua file");
            lua_State* L = usbdevice->m.L;

            simdata_to_lua(L, simdata);
            slogt("getting pulses from lua file");
            lua_setglobal(L, "simdata");

            lua_getglobal(L,"myFunc");
            if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            {
                sloge("Error calling Lua script: %s\n", lua_tostring(L, -1));
            }

            lua_getglobal(L, "Pulses");
            int result = 0;
            if (lua_isnumber(L, -1)) {
                pulses = lua_tonumber(L, -1);
            }
            lua_pop(L,1);

            slogt("Settings tachometer pulses to %i", pulses);
            result = revburner_update(usbdevice, pulses);
            return result < 0 ? result : 0;
            break;
    }

    return 0;
}

int tachdev_free(USBDevice* usbdevice)
{
    TachDevice* tachdevice = &usbdevice->u.tachdevice;
    switch ( tachdevice->type )
    {
        case TACHDEV_UNKNOWN :
        case TACHDEV_REVBURNER :
            revburner_update(usbdevice, 0);
            revburner_free(usbdevice);
            lua_close(usbdevice->m.L);
            free(usbdevice->m.device_specific_config_file);
            break;
    }

    return 0;
}

int tachdev_init(USBDevice* usbdevice, DeviceSettings* ds)
{
    slogi("initializing tachometer device...");
    int error = 0;

    // at this point all usb tach devices must have a device specific lua file
    if(does_file_exist(ds->specific_config_file) == true)
    {
        usbdevice->m.device_specific_config_file = strdup(ds->specific_config_file);
        slogi("LUA config specified for this device... initializing... %s", usbdevice->m.device_specific_config_file);

        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        int top=lua_gettop(L);
        int status = luaL_loadfile(L, usbdevice->m.device_specific_config_file);

        if (status) {
            sloge("Couldn't load lua file: %s\n", lua_tostring(L, -1));
            lua_close(usbdevice->m.L);
            error = -2;
        }

        if(error >= 0)
        {
            lua_setglobal(L,"myFunc");
            usbdevice->m.L = L;
            slogi("LUA config setup successful.");
        }

        if(error < 0)
        {
            free(usbdevice->m.device_specific_config_file);
        }
    }
    else
    {
        error = -1;
        return error;
    }

    TachDevice* tachdevice = &usbdevice->u.tachdevice;
    // detection of tach device model
    tachdevice->type = TACHDEV_UNKNOWN;
    tachdevice->type = TACHDEV_REVBURNER;

    tachdevice->tachsettings = ds->tachsettings;

    error = revburner_init(usbdevice);

    return error;
}
