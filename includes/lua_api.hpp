#pragma once
#include <lua.hpp>
#include <string>

// Forward Declarations
class JSONDocument;

// Parser Methods (Implemented in parser/parser_lua_api.cpp)
void RegisterParserAPI(lua_State* L, JSONDocument* doc);

// UI Methods (Implemented in ui/ui_lua_api.cpp)
void RegisterUIAPI(lua_State* L, JSONDocument* doc);

// JSON -> Lua table helper (Implemented in parser/parser_lua_api.cpp)
// Used by window.cpp to process fetch responses
void PushJsonToLua(lua_State* L, const std::string& json_str);
