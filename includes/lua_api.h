#ifndef LUA_API_H
#define LUA_API_H

extern "C" {
#include <lua.h>
}

void RegisterBrowserAPI(lua_State* L);

#endif
