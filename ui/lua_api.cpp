#include "lua_api.h"
#include "raylib.h"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

// 1. The function that Lua will call
int l_getScreenSize(lua_State* L) {
    // Grab the actual dimensions from Raylib
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    // Create a new empty table (array) in Lua's memory
    lua_newtable(L);

    // Put the width at index 1
    lua_pushinteger(L, width);
    lua_rawseti(L, -2, 1);

    // Put the height at index 2
    lua_pushinteger(L, height);
    lua_rawseti(L, -2, 2);

    // Tell Lua we are returning exactly 1 thing (the table we just made)
    return 1;
}

// 2. The function that links C++ to Lua
void RegisterBrowserAPI(lua_State* L) {
    // Create a new empty table called 'browser'
    lua_newtable(L);

    // Attach our C++ function to the table under the name "getScreenSize"
    lua_pushcfunction(L, l_getScreenSize);
    lua_setfield(L, -2, "getScreenSize");

    // Save this table as a global variable in Lua named "browser"
    lua_setglobal(L, "browser");
}
