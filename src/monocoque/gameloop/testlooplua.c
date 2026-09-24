#include <unistd.h>
#include <string.h>
#include "testlooplua.h"

#include "../slog/slog.h"
#include "../devices/simdevice.h"
#include "../simulatorapi/simapi/simapi/simmapper.h"
#include "../simulatorapi/simapi/simapi/simmap.h"
#include "../simulatorapi/simapi/simapi/simdata.h"

static void update_devices(SimDevice* devices, int numdevices, SimData* simdata, SimMap* testsimmap)
{
    for (int x = 0; x < numdevices; x++)
    {
        if (devices[x].initialized == true)
        {
            devices[x].update(&devices[x], simdata);
        }
    }
    if (testsimmap != NULL && testsimmap->addr != NULL)
    {
        simdata->mtick++;
        memcpy(testsimmap->addr, simdata, sizeof(SimData));
    }
}

int monocoque_test_update_devices(lua_State *L)
{
    lua_pushstring(L, "simdata");
    lua_gettable(L, LUA_REGISTRYINDEX);
    SimData* simdata = lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "simdevices");
    lua_gettable(L, LUA_REGISTRYINDEX);
    SimDevice* devices = lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "testsimmap");
    lua_gettable(L, LUA_REGISTRYINDEX);
    SimMap* testsimmap = lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "numdevices");
    lua_gettable(L, LUA_REGISTRYINDEX);
    int numdevices = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    slogt("lua called c function monocoque_test_update_devices on %i devices and rpms is %i", numdevices, simdata->rpms);
    update_devices(devices, numdevices, simdata, testsimmap);
}

int monocoque_test_sleep(lua_State *L)
{
    slogt("lua called c function monocoque_test_sleep");

    int sleep_time = lua_tonumber(L, 1);

    sleep(sleep_time);
}

int monocoque_test_set_rpms(lua_State *L)
{
    slogt("lua called c function monocoque_test_set_rpms");

    int rpms = lua_tonumber(L, 1);

    lua_pushstring(L, "simdata");
    lua_gettable(L, LUA_REGISTRYINDEX);
    SimData* simdata = lua_touserdata(L, -1);
    lua_pop(L, 1);

    simdata->rpms = rpms;
}

int monocoque_test_set_max_rpms(lua_State *L)
{
    slogt("lua called c function monocoque_test_set_max_rpms");

    int maxrpms = lua_tonumber(L, 1);

    lua_pushstring(L, "simdata");
    lua_gettable(L, LUA_REGISTRYINDEX);
    SimData* simdata = lua_touserdata(L, -1);
    lua_pop(L, 1);

    simdata->maxrpm = maxrpms;
}
