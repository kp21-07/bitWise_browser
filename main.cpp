#include "includes/window.hpp"
#include "includes/lua_api.hpp"
#include "includes/parser.hpp"

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
}

int main() {
    // 1. Initialize the C++ DOM
    JSONDocument document;

    // 2. Initialize Lua Director
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // 3. Connect UI API and Parser API to the Director
    RegisterUIAPI(L);
    RegisterParserAPI(L, &document);

    // 4. Start the Visual Engine
    RunWindow(document, L);

    lua_close(L);
    return 0;
}

