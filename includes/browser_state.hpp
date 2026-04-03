#pragma once
#include <map>
#include <mutex>
#include <queue>
#include <string>

// -------------------------------------------------------------------
// Element Bounds: populated by renderer each frame for getElemRect()
// -------------------------------------------------------------------
struct ElemRect {
    float x, y, w, h;
};
extern std::map<int, ElemRect> g_elem_bounds;

// -------------------------------------------------------------------
// Click Handlers: element_id -> Lua registry ref
// -------------------------------------------------------------------
extern std::map<int, int> g_click_handlers;

// -------------------------------------------------------------------
// Fetch Callback Queue (thread-safe)
// Network thread pushes results, main thread pops & calls Lua
// -------------------------------------------------------------------
struct FetchResult {
    std::string response_body;
    int lua_callback_ref; // LUA_REGISTRYINDEX ref
};
extern std::mutex g_fetch_mutex;
extern std::queue<FetchResult> g_fetch_queue;

// -------------------------------------------------------------------
// Page Load Queue (thread-safe)
// Replaces the old direct-callback-on-thread approach
// -------------------------------------------------------------------
extern std::mutex g_page_load_mutex;
extern std::queue<std::string> g_page_load_queue;
