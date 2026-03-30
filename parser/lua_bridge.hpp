#pragma once
#include <lua.hpp>
#include "parser.hpp"

// Registers the 'browser' table and its functions into the given Lua state.
void open_browser_api(lua_State* L, JSONDocument* doc);
