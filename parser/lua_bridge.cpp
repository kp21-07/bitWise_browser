#include "lua_bridge.hpp"
#include <iostream>

// Helper to push a Node's data onto the Lua stack as a table
void push_node_to_lua(lua_State* L, const Node& node) {
    lua_newtable(L);
    
    // Set ID
    lua_pushstring(L, "id");
    lua_pushinteger(L, node.id);
    lua_settable(L, -3);
    
    // Set Tag
    lua_pushstring(L, "tag");
    lua_pushstring(L, node.tag.c_str());
    lua_settable(L, -3);
    
    // Set Background Color
    lua_pushstring(L, "bgcolour");
    lua_pushstring(L, node.bgcolour.c_str());
    lua_settable(L, -3);
}

// C++ Implementation of browser.getElem(id)
static int l_getElem(lua_State* L) {
    // 1. Extract the JSONDocument pointer from the "Upvalue" 
    // (This is how we give Lua a reference to our C++ object)
    JSONDocument* doc = (JSONDocument*)lua_touserdata(L, lua_upvalueindex(1));
    
    // 2. Get the ID from the first argument passed by Lua
    int id = (int)luaL_checkinteger(L, 1);
    
    // 3. Look up the Node
    auto node = doc->getNode(id);
    if (node) {
        push_node_to_lua(L, *node);
        return 1; // Returning 1 value (the table) to Lua
    }
    
    lua_pushnil(L);
    return 1;
}

void open_browser_api(lua_State* L, JSONDocument* doc) {
    // Create the 'browser' global table
    lua_newtable(L);
    
    // Add getElem function to the 'browser' table
    lua_pushstring(L, "getElem");
    lua_pushlightuserdata(L, doc);       // Arg 1: Pointer to our C++ Doc
    lua_pushcclosure(L, l_getElem, 1);  // The '1' means l_getElem gets the Doc pointer as an 'upvalue'
    lua_settable(L, -3);
    
    // Set the table globally as 'browser'
    lua_setglobal(L, "browser");
}
