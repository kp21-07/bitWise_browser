#pragma once
#include <lua.hpp>

// Forward Delcarations
class JSONDocument;

// Parser Methods (Implemented in parser/parser_lua_api.cpp)
void RegisterParserAPI(lua_State* L, JSONDocument* doc);

// UI Methods (Implemented in ui/ui_lua_api.cpp)
void RegisterUIAPI(lua_State* L);
