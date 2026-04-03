#include "../includes/lua_api.hpp"
#include "../includes/parser.hpp"
#include <iostream>
#include <unordered_map>

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

    // Set Spacing
    lua_pushstring(L, "spacing");
    lua_pushinteger(L, node.spacing);
    lua_settable(L, -3);

    // Set OnClick
    lua_pushstring(L, "onclick");
    lua_pushstring(L, node.onclick.c_str());
    lua_settable(L, -3);
    
    // Specific Data (Text)
    if (std::holds_alternative<TextData>(node.specific_data)) {
        auto td = std::get<TextData>(node.specific_data);
        lua_pushstring(L, "text");
        lua_pushstring(L, td.content.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "colour");
        lua_pushstring(L, td.colour.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "fontsize");
        lua_pushinteger(L, td.fontsize);
        lua_settable(L, -3);
    }

    // Specific Data (Image)
    if (std::holds_alternative<ImageData>(node.specific_data)) {
        auto idat = std::get<ImageData>(node.specific_data);
        lua_pushstring(L, "image_url");
        lua_pushstring(L, idat.url.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "alttext");
        lua_pushstring(L, idat.alttext.c_str());
        lua_settable(L, -3);
    }
    
    // Children
    lua_pushstring(L, "children");
    lua_newtable(L);
    for (size_t i = 0; i < node.children.size(); ++i) {
        lua_pushinteger(L, node.children[i]);
        lua_rawseti(L, -2, i + 1);
    }
    lua_settable(L, -3);
}

// C++ Implementation of browser.getElemsByTag(tag)
static int l_getElemsByTag(lua_State* L) {
    JSONDocument* doc = (JSONDocument*)lua_touserdata(L, lua_upvalueindex(1));
    const char* tag = luaL_checkstring(L, 1);
    
    auto ids = doc->getElemsByTag(tag);
    lua_newtable(L);
    for (size_t i = 0; i < ids.size(); ++i) {
        lua_pushinteger(L, ids[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

// C++ Implementation of browser.updateElem(id, table)
static int l_updateElem(lua_State* L) {
    JSONDocument* doc = (JSONDocument*)lua_touserdata(L, lua_upvalueindex(1));
    int id = (int)luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    
    std::unordered_map<std::string, std::string> props;
    lua_pushnil(L);
    while (lua_next(L, 2) != 0) {
        if (lua_type(L, -2) == LUA_TSTRING) {
            std::string key = lua_tostring(L, -2);
            if (lua_type(L, -1) == LUA_TSTRING) {
                props[key] = lua_tostring(L, -1);
            } else if (lua_type(L, -1) == LUA_TNUMBER) {
                props[key] = std::to_string((int)lua_tonumber(L, -1));
            }
        }
        lua_pop(L, 1);
    }
    
    doc->updateNode(id, props);
    return 0;
}

// C++ Implementation of browser.getElem(id)
static int l_getElem(lua_State* L) {
    JSONDocument* doc = (JSONDocument*)lua_touserdata(L, lua_upvalueindex(1));
    int id = (int)luaL_checkinteger(L, 1);
    auto node = doc->getNode(id);
    if (node) {
        push_node_to_lua(L, *node);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

void RegisterParserAPI(lua_State* L, JSONDocument* doc) {
    lua_getglobal(L, "browser");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1); 
        lua_newtable(L); 
        lua_pushvalue(L, -1); 
        lua_setglobal(L, "browser");
    }
    
    // Add getElem
    lua_pushstring(L, "getElem");
    lua_pushlightuserdata(L, doc);
    lua_pushcclosure(L, l_getElem, 1);
    lua_settable(L, -3);

    // Add getElemsByTag
    lua_pushstring(L, "getElemsByTag");
    lua_pushlightuserdata(L, doc);
    lua_pushcclosure(L, l_getElemsByTag, 1);
    lua_settable(L, -3);

    // Add updateElem
    lua_pushstring(L, "updateElem");
    lua_pushlightuserdata(L, doc);
    lua_pushcclosure(L, l_updateElem, 1);
    lua_settable(L, -3);
    
    lua_pop(L, 1);
}

void push_json_value_to_lua(lua_State* L, const JsonValue& val) {
    switch (val.type) {
        case JsonValue::STRING:
            lua_pushstring(L, val.string_val.c_str());
            break;
        case JsonValue::NUMBER:
            lua_pushnumber(L, val.number_val);
            break;
        case JsonValue::BOOLEAN:
            lua_pushboolean(L, (int)val.number_val);
            break;
        case JsonValue::OBJECT:
            lua_newtable(L);
            for (auto const& [key, value] : val.object_val) {
                lua_pushstring(L, key.c_str()); // Corrected to use .c_str() for std::string
                push_json_value_to_lua(L, value);
                lua_settable(L, -3);
            }
            break;
        case JsonValue::ARRAY:
            lua_newtable(L);
            for (size_t i = 0; i < val.array_val.size(); ++i) {
                push_json_value_to_lua(L, val.array_val[i]);
                lua_rawseti(L, -2, i + 1);
            }
            break;
        case JsonValue::NULL_VAL:
        default:
            lua_pushnil(L);
            break;
    }
}

void PushJsonToLua(lua_State* L, const std::string& json_str) {
    Lexer lexer;
    lexer.source = json_str;
    JsonValue root = lexer.parse();
    push_json_value_to_lua(L, root);
}

