#include <lauxlib.h>
#include <lualib.h>

#ifdef USE_LUA_55
/* open all libraries */
#define luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)
#endif

int monocoque_test_sleep(lua_State *L);
int monocoque_test_update_devices(lua_State *L);

int monocoque_test_set_rpms(lua_State *L);
int monocoque_test_set_max_rpms(lua_State *L);
