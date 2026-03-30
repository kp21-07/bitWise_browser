#include "window.h"
#include "lua_api.h"

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
}

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    RegisterBrowserAPI(L);

    RunWindow();

    lua_close(L);
    return 0;
}
