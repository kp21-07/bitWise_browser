#include "../includes/raylib.h"
#include "../includes/window.hpp"
#include <iostream>
#include "../includes/parser.hpp"
#include "../includes/network.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <map>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

// External access for Lua API
std::mutex g_lua_mutex;
std::map<int, int> g_click_handlers; // NodeID -> Lua registry reference

Color ParseHexColor(const std::string& hex) {
    if (hex.length() >= 7 && hex[0] == '#') {
        try {
            int r = std::stoi(hex.substr(1, 2), nullptr, 16);
            int g = std::stoi(hex.substr(3, 2), nullptr, 16);
            int b = std::stoi(hex.substr(5, 2), nullptr, 16);
            return { (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
        } catch (...) {}
    }
    return WHITE;
}

int FindRootID(JSONDocument& doc) {
    if (doc.getNode(0).has_value()) return 0;
    if (doc.getNode(1).has_value()) return 1;
    for (int i = 2000; i >= 1000; --i) {
        if (doc.getNode(i).has_value()) return i;
    }
    return -1;
}

void RenderNode(JSONDocument& doc, int node_id, Rectangle bounds) {
    if (node_id == -1) return;
    auto node_opt = doc.getNode(node_id);
    if (!node_opt.has_value()) return;

    Node node = node_opt.value();
    
    // Store layout bounds for hit-testing and browser.getElemRect
    doc.setElemRect(node_id, { bounds.x, bounds.y, bounds.width, bounds.height });

    Color bg = ParseHexColor(node.bgcolour);
    DrawRectangleRec(bounds, bg);

    if (node.type == NodeType::TEXT) {
        if (std::holds_alternative<TextData>(node.specific_data)) {
            auto text_data = std::get<TextData>(node.specific_data);
            Color text_col = ParseHexColor(text_data.colour);
            DrawText(text_data.content.c_str(), bounds.x + 5, bounds.y + 5, text_data.fontsize, text_col);
        }
    } else if (node.type == NodeType::IMAGE) {
        if (std::holds_alternative<ImageData>(node.specific_data)) {
            auto img_data = std::get<ImageData>(node.specific_data);
            DrawRectangleLinesEx(bounds, 2, GRAY);
            DrawText(img_data.alttext.c_str(), bounds.x + 5, bounds.y + 5, 20, DARKGRAY);
        }
    } else if (node.type == NodeType::FLEXV) {
        int count = node.children.size();
        if (count > 0) {
            int child_height = bounds.height / count;
            int current_y = bounds.y;
            for (int child_id : node.children) {
                Rectangle child_bounds = { bounds.x, (float)current_y, bounds.width, (float)child_height };
                RenderNode(doc, child_id, child_bounds);
                current_y += child_height;
            }
        }
    } else if (node.type == NodeType::FLEXH) {
        int count = node.children.size();
        if (count > 0) {
            int child_width = bounds.width / count;
            int current_x = bounds.x;
            for (int child_id : node.children) {
                Rectangle child_bounds = { (float)current_x, bounds.y, (float)child_width, bounds.height };
                RenderNode(doc, child_id, child_bounds);
                current_x += child_width;
            }
        }
    }
}

static JSONDocument* g_doc = nullptr;
static lua_State* g_L = nullptr;

static void NetworkCallback(std::string payload) {
    std::cout << "[UI] Network Response Received. Updating DOM...\n";
    if (g_doc) g_doc->update(payload);

    std::cout << "[UI] Executing Lua Director: " << (g_doc ? g_doc->lua_path : "") << "\n";
    if (g_L && g_doc && !g_doc->lua_path.empty()) {
        std::lock_guard<std::mutex> lock(g_lua_mutex);
        if (luaL_dofile(g_L, g_doc->lua_path.c_str())) {
            std::cout << "[UI Error] Lua Script Failed: " << lua_tostring(g_L, -1) << "\n";
        }
    }
}

void RunWindow(JSONDocument& doc, lua_State* L){

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, "JSML Browser");
    SetTargetFPS(60);

    std::string urlText = "http://";
    bool isUrlBarActive = false;

    while (!WindowShouldClose()) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        
        Rectangle urlBar = { 20, 20, (float)screenWidth - 40, 40 };
        if (CheckCollisionPointRec(GetMousePosition(), urlBar)) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                isUrlBarActive = true;
            }
        } else {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                isUrlBarActive = false;
            }
        }

        if (isUrlBarActive) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125)) {
                    urlText += (char)key;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && urlText.length() > 0) {
                urlText.pop_back();
            }

            if (IsKeyPressed(KEY_ENTER)) {
                std::cout << "[UI] Navigating to: " << urlText << "\n";
                g_doc = &doc;
                g_L = L;
                fetch(urlText, NetworkCallback);
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangleRec(urlBar, isUrlBarActive ? LIGHTGRAY : GRAY);
        DrawRectangleLinesEx(urlBar, 2, DARKGRAY);

        DrawText(urlText.c_str(), 30, 30, 20, DARKGRAY);

        if (isUrlBarActive && ((int)(GetTime()*2) % 2) == 0) {
            int textWidth = MeasureText(urlText.c_str(), 20);
            DrawText("_", 30 + textWidth + 2, 30, 20, DARKGRAY);
        }

        int root_id = FindRootID(doc);
        if (root_id != -1) {
            RenderNode(doc, root_id, { 0, 70, (float)screenWidth, (float)screenHeight - 70 });
        }

        // Handle Click Events for registered handlers
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isUrlBarActive) {
            Vector2 mouse = GetMousePosition();
            std::lock_guard<std::mutex> lock(g_lua_mutex);
            for (auto const& [node_id, handler_ref] : g_click_handlers) {
                auto rect_opt = doc.getElemRect(node_id);
                if (rect_opt.has_value()) {
                    Rect r = rect_opt.value();
                    if (CheckCollisionPointRec(mouse, { r.x, r.y, r.width, r.height })) {
                        // Trigger Lua Callback
                        lua_rawgeti(L, LUA_REGISTRYINDEX, handler_ref);
                        if (lua_pcall(L, 0, 0, 0) != 0) {
                            std::cout << "[Lua Error] Click Handler Failed: " << lua_tostring(L, -1) << "\n";
                            lua_pop(L, 1);
                        }
                        break; // Stop at first handler found (Z-order not handled yet)
                    }
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
}

