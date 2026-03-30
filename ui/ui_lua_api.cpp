#include "../includes/lua_api.hpp"
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
void RegisterUIAPI(lua_State* L) {
    // 1. Fetch the existing 'browser' table, or create it if missing
    lua_getglobal(L, "browser");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1); 
        lua_newtable(L); 
        lua_pushvalue(L, -1); 
        lua_setglobal(L, "browser");
    }
    
    // Attach our C++ function to the table under the name "getScreenSize"
    lua_pushcfunction(L, l_getScreenSize);
    lua_setfield(L, -2, "getScreenSize");

    // 3. Clean up the stack
    lua_pop(L, 1);
}

