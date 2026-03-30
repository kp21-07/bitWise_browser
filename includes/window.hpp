#ifndef WINDOW_HPP
#define WINDOW_HPP

// Forward declarations to avoid complex includes in headers
class JSONDocument;
struct lua_State;

void RunWindow(JSONDocument& doc, lua_State* L);

#endif