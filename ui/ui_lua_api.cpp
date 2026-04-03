#include "../includes/lua_api.hpp"
#include "../includes/parser.hpp"
#include "../includes/network.hpp"
#include "../includes/telemetry.hpp"
#include "raylib.h"
#include <iostream>
#include <mutex>
#include <map>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

// Shared with window.cpp
extern std::mutex g_lua_mutex;
extern std::map<int, int> g_click_handlers;

// 1. browser.getScreenSize()
int l_getScreenSize(lua_State* L) {
    lua_newtable(L);
    lua_pushinteger(L, GetScreenWidth());
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, GetScreenHeight());
    lua_rawseti(L, -2, 2);
    return 1;
}

// 2. browser.getElemRect(id)
int l_getElemRect(lua_State* L) {
    JSONDocument* doc = (JSONDocument*)lua_touserdata(L, lua_upvalueindex(1));
    int id = (int)luaL_checkinteger(L, 1);
    auto rect_opt = doc->getElemRect(id);
    if (rect_opt.has_value()) {
        Rect r = rect_opt.value();
        lua_newtable(L);
        lua_pushstring(L, "x"); lua_pushnumber(L, r.x); lua_settable(L, -3);
        lua_pushstring(L, "y"); lua_pushnumber(L, r.y); lua_settable(L, -3);
        lua_pushstring(L, "width"); lua_pushnumber(L, r.width); lua_settable(L, -3);
        lua_pushstring(L, "height"); lua_pushnumber(L, r.height); lua_settable(L, -3);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

// 3. browser.addClickHandler(id, callback)
int l_addClickHandler(lua_State* L) {
    int id = (int)luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    std::lock_guard<std::mutex> lock(g_lua_mutex);
    // Remove old handler if exists
    if (g_click_handlers.count(id)) {
        luaL_unref(L, LUA_REGISTRYINDEX, g_click_handlers[id]);
    }
    
    // Store new handler in registry
    lua_pushvalue(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    g_click_handlers[id] = ref;
    
    return 0;
}

// 4. browser.log(message)
int l_log(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    std::cout << "[LUA LOG] " << msg << std::endl;
    return 0;
}

// 5. browser.getTime()
int l_getTime(lua_State* L) {
    lua_pushnumber(L, (double)getTime());
    return 1;
}

// 6. browser.getCurrentUrl()
int l_getCurrentUrl(lua_State* L) {
    lua_pushstring(L, getCurrentUrl().c_str());
    return 1;
}

// 7. browser.fetch(url, callback)
struct FetchContext {
    int callback_ref;
    lua_State* L;
};

int l_fetch(lua_State* L) {
    const char* url = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    lua_pushvalue(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // Captured in lambda
    fetch(url, [L, ref](std::string payload) {
        std::lock_guard<std::mutex> lock(g_lua_mutex);
        
        // Strip headers (simplified)
        size_t body_start = payload.find("\r\n\r\n");
        std::string body = (body_start != std::string::npos) ? payload.substr(body_start + 4) : payload;
        
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        PushJsonToLua(L, body);
        if (lua_pcall(L, 1, 0, 0) != 0) {
            std::cout << "[Lua Error] fetch callback failed: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    });
    
    return 0;
}

void RegisterUIAPI(lua_State* L, JSONDocument* doc) {
    lua_getglobal(L, "browser");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "browser");
    }

    // Register all UI/System functions
    lua_pushcfunction(L, l_getScreenSize); lua_setfield(L, -2, "getScreenSize");
    lua_pushcfunction(L, l_addClickHandler); lua_setfield(L, -2, "addClickHandler");
    lua_pushcfunction(L, l_log); lua_setfield(L, -2, "log");
    lua_pushcfunction(L, l_getTime); lua_setfield(L, -2, "getTime");
    lua_pushcfunction(L, l_getCurrentUrl); lua_setfield(L, -2, "getCurrentUrl");
    lua_pushcfunction(L, l_fetch); lua_setfield(L, -2, "fetch");

    // With Upvalue (doc)
    lua_pushstring(L, "getElemRect");
    lua_pushlightuserdata(L, doc);
    lua_pushcclosure(L, l_getElemRect, 1);
    lua_settable(L, -3);

    lua_pop(L, 1);
}

