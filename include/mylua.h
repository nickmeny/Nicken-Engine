// A single file that i just include lua libs so not write every time 3 lines of code
#pragma once

#include "lua.h"

//My func for simple lua_absindex because in luaJIT 5.2 this func doesnt exist
#if LUA_VERSION_NUM < 502
static inline int lua_absindex(lua_State *L, int idx) {
    if (idx > 0 || idx <= LUA_REGISTRYINDEX) {
        return idx;
    }
    return lua_gettop(L) + idx + 1;
}
#endif

#include "lauxlib.h"
#include "lualib.h"

