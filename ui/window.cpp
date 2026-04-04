// #include "../includes/raylib.h"
// #include "../includes/window.hpp"
// #include <iostream>
// #include "../includes/parser.hpp"
// #include "../includes/network.hpp"
// #include <string>
// #include <vector>
// #include <mutex>
// #include <map>
// #include <unordered_map>

// extern "C" {
// #include <lua.h>
// #include <lualib.h>
// #include <lauxlib.h>
// }

// // External access for Lua API
// std::mutex g_lua_mutex;
// std::map<int, int> g_click_handlers; // NodeID -> Lua registry reference

// // Texture Map
// static std::unordered_map<std::string, Texture2D> g_texture_cache;

// float backspaceTimer = 0.0f;
// float backspaceRepeatTimer = 0.0f;
// const float BACKSPACE_DELAY = 0.5f;
// const float BACKSPACE_REPEAT = 0.05f;

// static Texture2D GetCachedTexture(const std::string& url) {
//     if (g_texture_cache.find(url) == g_texture_cache.end()) {
//         g_texture_cache[url] = LoadTexture(url.c_str());
//     }
//     return g_texture_cache[url];
// }

// Color ParseHexColor(const std::string& hex) {
//     if (hex.length() >= 7 && hex[0] == '#') {
//         try {
//             int r = std::stoi(hex.substr(1, 2), nullptr, 16);
//             int g = std::stoi(hex.substr(3, 2), nullptr, 16);
//             int b = std::stoi(hex.substr(5, 2), nullptr, 16);
//             return { (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
//         } catch (...) {}
//     }
//     return WHITE;
// }

// int FindRootID(JSONDocument& doc) {
//     if (doc.getNode(0).has_value()) return 0;
//     if (doc.getNode(1).has_value()) return 1;
//     for (int i = 2000; i >= 1000; --i) {
//         if (doc.getNode(i).has_value()) return i;
//     }
//     return -1;
// }

// void RenderNode(JSONDocument& doc, int node_id, Rectangle bounds) {
//     if (node_id == -1) return;
//     auto node_opt = doc.getNode(node_id);
//     if (!node_opt.has_value()) return;

//     Node node = node_opt.value();
    
//     // Store layout bounds for hit-testing and browser.getElemRect
//     doc.setElemRect(node_id, { bounds.x, bounds.y, bounds.width, bounds.height });

//     Color bg = ParseHexColor(node.bgcolour);
//     DrawRectangleRec(bounds, bg);

//     if (node.type == NodeType::TEXT) {
//         if (std::holds_alternative<TextData>(node.specific_data)) {
//             auto text_data = std::get<TextData>(node.specific_data);
//             Color text_col = ParseHexColor(text_data.colour);
//             DrawText(text_data.content.c_str(), bounds.x + 5, bounds.y + 5, text_data.fontsize, text_col);
//         }
//     } else if (node.type == NodeType::IMAGE) {
//         if (std::holds_alternative<ImageData>(node.specific_data)) {
//             auto img_data = std::get<ImageData>(node.specific_data);
//             Texture2D tex = GetCachedTexture(img_data.url);
//             if (tex.id != 0) {
//                 Rectangle sourceRec = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
//                 DrawTexturePro(tex, sourceRec, bounds, {0.0f, 0.0f}, 0.0f, WHITE);
//             } else {
//                 DrawRectangleLinesEx(bounds, 2, GRAY);
//                 DrawText(img_data.alttext.c_str(), bounds.x + 5, bounds.y + 5, 20, DARKGRAY);
//             }
//         }
//     } else if (node.type == NodeType::FLEXV) {
//         int count = node.children.size();
//         if (count > 0) {
//             int child_height = bounds.height / count;
//             int current_y = bounds.y;
//             for (int child_id : node.children) {
//                 Rectangle child_bounds = { bounds.x, (float)current_y, bounds.width, (float)child_height };
//                 RenderNode(doc, child_id, child_bounds);
//                 current_y += child_height;
//             }
//         }
//     } else if (node.type == NodeType::FLEXH) {
//         int count = node.children.size();
//         if (count > 0) {
//             int child_width = bounds.width / count;
//             int current_x = bounds.x;
//             for (int child_id : node.children) {
//                 Rectangle child_bounds = { (float)current_x, bounds.y, (float)child_width, bounds.height };
//                 RenderNode(doc, child_id, child_bounds);
//                 current_x += child_width;
//             }
//         }
//     }
// }

// static JSONDocument* g_doc = nullptr;
// static lua_State* g_L = nullptr;

// static void NetworkCallback(std::string payload) {
//     std::cout << "[UI] Network Response Received. Updating DOM...\n";
//     if (g_doc) g_doc->update(payload);

//     std::cout << "[UI] Executing Lua Director: " << (g_doc ? g_doc->lua_path : "") << "\n";
//     if (g_L && g_doc && !g_doc->lua_path.empty()) {
//         std::lock_guard<std::mutex> lock(g_lua_mutex);
//         if (luaL_dofile(g_L, g_doc->lua_path.c_str())) {
//             std::cout << "[UI Error] Lua Script Failed: " << lua_tostring(g_L, -1) << "\n";
//         }
//     }
// }

// void RunWindow(JSONDocument& doc, lua_State* L){

//     SetConfigFlags(FLAG_WINDOW_RESIZABLE);
//     InitWindow(1024, 768, "JSML Browser");
//     SetTargetFPS(60);

//     std::string urlText = "http://";
//     bool isUrlBarActive = false;

//     while (!WindowShouldClose()) {
//         int screenWidth = GetScreenWidth();
//         int screenHeight = GetScreenHeight();
        
//         Rectangle urlBar = { 20, 20, (float)screenWidth - 40, 40 };
//         if (CheckCollisionPointRec(GetMousePosition(), urlBar)) {
//             if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
//                 isUrlBarActive = true;
//             }
//         }
//         else if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)){
//             isUrlBarActive = true;
//         }
//         else {
//             if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
//                 isUrlBarActive = false;
//             }
//         }

//         if (isUrlBarActive) {
//             int key = GetCharPressed();
//             while (key > 0) {
//                 if ((key >= 32) && (key <= 125)) {
//                     urlText += (char)key;
//                 }
//                 key = GetCharPressed();
//             }

//             if (IsKeyPressed(KEY_BACKSPACE) && urlText.length() > 0) {
//                 urlText.pop_back();
//                 backspaceTimer = 0.0f;
//             }

//             if (IsKeyDown(KEY_BACKSPACE)) {
//                 if (urlText.length() > 0) {
//                     backspaceTimer += GetFrameTime();
//                     if (backspaceTimer >= BACKSPACE_DELAY) {
//                         urlText.pop_back();
//                         backspaceTimer = BACKSPACE_DELAY - BACKSPACE_REPEAT;
//                     }
//                 }
//             } else {
//                 backspaceTimer = 0.0f;
//             }

//             if (IsKeyPressed(KEY_ENTER)) {
//                 std::cout << "[UI] Navigating to: " << urlText << "\n";
//                 g_doc = &doc;
//                 g_L = L;
//                 debugFetch(urlText, NetworkCallback);
//             }
//         }

//         BeginDrawing();
//         ClearBackground(RAYWHITE);

//         DrawRectangleRec(urlBar, isUrlBarActive ? LIGHTGRAY : GRAY);
//         DrawRectangleLinesEx(urlBar, 2, DARKGRAY);

//         DrawText(urlText.c_str(), 30, 30, 20, DARKGRAY);

//         if (isUrlBarActive && ((int)(GetTime()*2) % 2) == 0) {
//             int textWidth = MeasureText(urlText.c_str(), 20);
//             DrawText("_", 30 + textWidth + 2, 30, 20, DARKGRAY);
//         }

//         int root_id = FindRootID(doc);
//         if (root_id != -1) {
//             RenderNode(doc, root_id, { 0, 70, (float)screenWidth, (float)screenHeight - 70 });
//         }

//         // Handle Click Events for registered handlers
//         if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isUrlBarActive) {
//             Vector2 mouse = GetMousePosition();
//             std::lock_guard<std::mutex> lock(g_lua_mutex);
//             for (auto const& [node_id, handler_ref] : g_click_handlers) {
//                 auto rect_opt = doc.getElemRect(node_id);
//                 if (rect_opt.has_value()) {
//                     Rect r = rect_opt.value();
//                     if (CheckCollisionPointRec(mouse, { r.x, r.y, r.width, r.height })) {
//                         // Trigger Lua Callback
//                         lua_rawgeti(L, LUA_REGISTRYINDEX, handler_ref);
//                         if (lua_pcall(L, 0, 0, 0) != 0) {
//                             std::cout << "[Lua Error] Click Handler Failed: " << lua_tostring(L, -1) << "\n";
//                             lua_pop(L, 1);
//                         }
//                         break; // Stop at first handler found (Z-order not handled yet)
//                     }
//                 }
//             }
//         }

//         EndDrawing();
//     }

//     for (auto& pair : g_texture_cache) {
//         if (pair.second.id != 0) {
//             UnloadTexture(pair.second);
//         }
//     }
//     g_texture_cache.clear();

//     CloseWindow();
// }














#include "../includes/raylib.h"
#include "../includes/window.hpp"
#include <iostream>
#include "../includes/parser.hpp"
#include "../includes/network.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <unordered_map>

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

// External access for Lua API
std::mutex g_lua_mutex;
std::map<int, int> g_click_handlers; // NodeID -> Lua registry reference

// Texture Map
static std::unordered_map<std::string, Texture2D> g_texture_cache;

// --- URL Bar Editing State ---
struct UrlBarState {
    std::string text = "http://";
    int cursor = 7;         // cursor position (index into text)
    int selStart = -1;      // selection start (-1 = no selection)
    int selEnd = -1;        // selection end
    bool active = false;

    // Backspace/Delete repeat
    float backspaceTimer = 0.0f;
    float deleteTimer = 0.0f;
    // Arrow key repeat
    float arrowTimer = 0.0f;
    float arrowRepeatTimer = 0.0f;

    const float KEY_DELAY  = 0.4f;
    const float KEY_REPEAT = 0.05f;

    bool hasSelection() const {
        return selStart != -1 && selEnd != -1 && selStart != selEnd;
    }

    // Normalised selection range [lo, hi)
    int selLo() const { return std::min(selStart, selEnd); }
    int selHi() const { return std::max(selStart, selEnd); }

    void clearSelection() { selStart = selEnd = -1; }

    void selectAll() {
        selStart = 0;
        selEnd = (int)text.size();
        cursor = selEnd;
    }

    // Delete selected text, place cursor at lo
    void deleteSelection() {
        if (!hasSelection()) return;
        int lo = selLo(), hi = selHi();
        text.erase(lo, hi - lo);
        cursor = lo;
        clearSelection();
    }

    // Insert a string at cursor (replaces selection if any)
    void insert(const std::string& s) {
        if (hasSelection()) deleteSelection();
        text.insert(cursor, s);
        cursor += (int)s.size();
    }

    // Clamp cursor to valid range
    void clampCursor() {
        cursor = std::max(0, std::min(cursor, (int)text.size()));
    }
};

static Texture2D GetCachedTexture(const std::string& url) {
    if (g_texture_cache.find(url) == g_texture_cache.end()) {
        g_texture_cache[url] = LoadTexture(url.c_str());
    }
    return g_texture_cache[url];
}

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
            Texture2D tex = GetCachedTexture(img_data.url);
            if (tex.id != 0) {
                Rectangle sourceRec = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
                DrawTexturePro(tex, sourceRec, bounds, {0.0f, 0.0f}, 0.0f, WHITE);
            } else {
                DrawRectangleLinesEx(bounds, 2, GRAY);
                DrawText(img_data.alttext.c_str(), bounds.x + 5, bounds.y + 5, 20, DARKGRAY);
            }
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

// -----------------------------------------------------------------------
// Helper: pixel x-offset of a character index inside the URL bar text
// -----------------------------------------------------------------------
static int TextXAt(const std::string& text, int idx, int fontSize) {
    if (idx <= 0) return 0;
    if (idx >= (int)text.size())
        return MeasureText(text.c_str(), fontSize);
    return MeasureText(text.substr(0, idx).c_str(), fontSize);
}

// -----------------------------------------------------------------------
// Helper: closest character index for a pixel x inside the URL bar
// -----------------------------------------------------------------------
static int CharIndexAt(const std::string& text, int pixelX, int fontSize) {
    int best = 0;
    int bestDist = std::abs(pixelX);
    for (int i = 1; i <= (int)text.size(); ++i) {
        int px = MeasureText(text.substr(0, i).c_str(), fontSize);
        if (std::abs(px - pixelX) < bestDist) {
            bestDist = std::abs(px - pixelX);
            best = i;
        }
    }
    return best;
}

void RunWindow(JSONDocument& doc, lua_State* L) {

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, "JSML Browser");
    SetTargetFPS(60);

    UrlBarState url;
    const int FONT_SIZE   = 20;
    const int BAR_PAD_X   = 10; // left text padding inside bar
    const int BAR_Y       = 20;
    const int BAR_HEIGHT  = 40;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        int screenWidth  = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        Rectangle urlBar = { 20, (float)BAR_Y, (float)(screenWidth - 40), (float)BAR_HEIGHT };

        // ---- Focus management ----
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(GetMousePosition(), urlBar)) {
                url.active = true;
                // Place cursor at click position
                int relX = (int)GetMousePosition().x - (int)urlBar.x - BAR_PAD_X;
                url.cursor = CharIndexAt(url.text, relX, FONT_SIZE);
                url.clearSelection();
            } else {
                url.active = false;
                url.clearSelection();
            }
        }

        // Ctrl+E shortcut to focus bar
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E)) {
            url.active = true;
            url.selectAll();
        }

        // ---- Keyboard input (only when bar is active) ----
        if (url.active) {
            bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
            bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

            // --- Ctrl shortcuts ---
            if (ctrl) {
                if (IsKeyPressed(KEY_A)) {
                    url.selectAll();
                }
                if (IsKeyPressed(KEY_C)) {
                    if (url.hasSelection()) {
                        std::string selected = url.text.substr(url.selLo(), url.selHi() - url.selLo());
                        SetClipboardText(selected.c_str());
                    }
                }
                if (IsKeyPressed(KEY_X)) {
                    if (url.hasSelection()) {
                        std::string selected = url.text.substr(url.selLo(), url.selHi() - url.selLo());
                        SetClipboardText(selected.c_str());
                        url.deleteSelection();
                    }
                }
                if (IsKeyPressed(KEY_V)) {
                    const char* clip = GetClipboardText();
                    if (clip) {
                        // Strip newlines from pasted text
                        std::string pasted(clip);
                        pasted.erase(std::remove_if(pasted.begin(), pasted.end(),
                            [](char c){ return c == '\n' || c == '\r'; }), pasted.end());
                        url.insert(pasted);
                    }
                }
                // Ctrl+Left / Ctrl+Right: jump by word
                if (IsKeyPressed(KEY_LEFT)) {
                    int pos = url.cursor;
                    if (pos > 0) {
                        --pos;
                        while (pos > 0 && url.text[pos - 1] != '/' && url.text[pos - 1] != '.' && url.text[pos - 1] != ' ') --pos;
                    }
                    if (shift) {
                        if (url.selStart == -1) url.selStart = url.cursor;
                        url.selEnd = pos;
                    } else {
                        url.clearSelection();
                    }
                    url.cursor = pos;
                }
                if (IsKeyPressed(KEY_RIGHT)) {
                    int pos = url.cursor;
                    int len = (int)url.text.size();
                    if (pos < len) {
                        ++pos;
                        while (pos < len && url.text[pos] != '/' && url.text[pos] != '.' && url.text[pos] != ' ') ++pos;
                    }
                    if (shift) {
                        if (url.selStart == -1) url.selStart = url.cursor;
                        url.selEnd = pos;
                    } else {
                        url.clearSelection();
                    }
                    url.cursor = pos;
                }
                // Ctrl+Home / Ctrl+End
                if (IsKeyPressed(KEY_HOME)) {
                    if (shift) { if (url.selStart == -1) url.selStart = url.cursor; url.selEnd = 0; }
                    else url.clearSelection();
                    url.cursor = 0;
                }
                if (IsKeyPressed(KEY_END)) {
                    int end = (int)url.text.size();
                    if (shift) { if (url.selStart == -1) url.selStart = url.cursor; url.selEnd = end; }
                    else url.clearSelection();
                    url.cursor = end;
                }
            } else {
                // --- Arrow keys (non-ctrl) ---
                if (IsKeyPressed(KEY_LEFT)) {
                    if (url.hasSelection() && !shift) {
                        url.cursor = url.selLo();
                        url.clearSelection();
                    } else {
                        if (shift && url.selStart == -1) url.selStart = url.cursor;
                        if (url.cursor > 0) --url.cursor;
                        if (shift) url.selEnd = url.cursor;
                        else url.clearSelection();
                    }
                    url.arrowTimer = 0.0f;
                }
                if (IsKeyPressed(KEY_RIGHT)) {
                    if (url.hasSelection() && !shift) {
                        url.cursor = url.selHi();
                        url.clearSelection();
                    } else {
                        if (shift && url.selStart == -1) url.selStart = url.cursor;
                        if (url.cursor < (int)url.text.size()) ++url.cursor;
                        if (shift) url.selEnd = url.cursor;
                        else url.clearSelection();
                    }
                    url.arrowTimer = 0.0f;
                }
                // Arrow key repeat (held, no selection extension for simplicity)
                if (IsKeyDown(KEY_LEFT) && !shift) {
                    url.arrowTimer += dt;
                    if (url.arrowTimer >= url.KEY_DELAY) {
                        url.arrowRepeatTimer += dt;
                        if (url.arrowRepeatTimer >= url.KEY_REPEAT) {
                            if (url.cursor > 0) --url.cursor;
                            url.clearSelection();
                            url.arrowRepeatTimer = 0.0f;
                        }
                    }
                } else if (IsKeyDown(KEY_RIGHT) && !shift) {
                    url.arrowTimer += dt;
                    if (url.arrowTimer >= url.KEY_DELAY) {
                        url.arrowRepeatTimer += dt;
                        if (url.arrowRepeatTimer >= url.KEY_REPEAT) {
                            if (url.cursor < (int)url.text.size()) ++url.cursor;
                            url.clearSelection();
                            url.arrowRepeatTimer = 0.0f;
                        }
                    }
                } else {
                    url.arrowTimer = 0.0f;
                    url.arrowRepeatTimer = 0.0f;
                }

                // Home / End
                if (IsKeyPressed(KEY_HOME)) {
                    if (shift) { if (url.selStart == -1) url.selStart = url.cursor; url.selEnd = 0; }
                    else url.clearSelection();
                    url.cursor = 0;
                }
                if (IsKeyPressed(KEY_END)) {
                    int end = (int)url.text.size();
                    if (shift) { if (url.selStart == -1) url.selStart = url.cursor; url.selEnd = end; }
                    else url.clearSelection();
                    url.cursor = end;
                }
            }

            // --- Backspace ---
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (url.hasSelection()) {
                    url.deleteSelection();
                } else if (url.cursor > 0) {
                    url.text.erase(url.cursor - 1, 1);
                    --url.cursor;
                }
                url.backspaceTimer = 0.0f;
            } else if (IsKeyDown(KEY_BACKSPACE)) {
                url.backspaceTimer += dt;
                if (url.backspaceTimer >= url.KEY_DELAY) {
                    if (!url.hasSelection() && url.cursor > 0) {
                        url.text.erase(url.cursor - 1, 1);
                        --url.cursor;
                    }
                    url.backspaceTimer = url.KEY_DELAY - url.KEY_REPEAT;
                }
            } else {
                url.backspaceTimer = 0.0f;
            }

            // --- Delete key ---
            if (IsKeyPressed(KEY_DELETE)) {
                if (url.hasSelection()) {
                    url.deleteSelection();
                } else if (url.cursor < (int)url.text.size()) {
                    url.text.erase(url.cursor, 1);
                }
                url.deleteTimer = 0.0f;
            } else if (IsKeyDown(KEY_DELETE)) {
                url.deleteTimer += dt;
                if (url.deleteTimer >= url.KEY_DELAY) {
                    if (!url.hasSelection() && url.cursor < (int)url.text.size()) {
                        url.text.erase(url.cursor, 1);
                    }
                    url.deleteTimer = url.KEY_DELAY - url.KEY_REPEAT;
                }
            } else {
                url.deleteTimer = 0.0f;
            }

            // --- Printable character input ---
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 125) {
                    std::string ch(1, (char)key);
                    url.insert(ch);
                }
                key = GetCharPressed();
            }

            // --- Enter: navigate ---
            if (IsKeyPressed(KEY_ENTER)) {
                std::cout << "[UI] Navigating to: " << url.text << "\n";
                url.clearSelection();
                g_doc = &doc;
                g_L = L;
                debugFetch(url.text, NetworkCallback);
            }

            // --- Escape: deactivate ---
            if (IsKeyPressed(KEY_ESCAPE)) {
                url.active = false;
                url.clearSelection();
            }
        }

        // ---- Drawing ----
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // URL bar background
        DrawRectangleRec(urlBar, url.active ? LIGHTGRAY : GRAY);
        DrawRectangleLinesEx(urlBar, 2, url.active ? BLUE : DARKGRAY);

        // Draw selection highlight
        if (url.active && url.hasSelection()) {
            int lo = url.selLo(), hi = url.selHi();
            int xLo = (int)urlBar.x + BAR_PAD_X + TextXAt(url.text, lo, FONT_SIZE);
            int xHi = (int)urlBar.x + BAR_PAD_X + TextXAt(url.text, hi, FONT_SIZE);
            DrawRectangle(xLo, BAR_Y + 4, xHi - xLo, BAR_HEIGHT - 8, { 100, 149, 237, 180 });
        }

        // Draw URL text
        DrawText(url.text.c_str(), (int)urlBar.x + BAR_PAD_X, BAR_Y + 10, FONT_SIZE, DARKGRAY);

        // Draw cursor (blinking, only when no selection)
        if (url.active && !url.hasSelection() && ((int)(GetTime() * 2) % 2) == 0) {
            int cx = (int)urlBar.x + BAR_PAD_X + TextXAt(url.text, url.cursor, FONT_SIZE);
            DrawLine(cx, BAR_Y + 6, cx, BAR_Y + BAR_HEIGHT - 6, DARKGRAY);
        }

        // Render DOM
        int root_id = FindRootID(doc);
        if (root_id != -1) {
            RenderNode(doc, root_id, { 0, (float)(BAR_Y + BAR_HEIGHT + 10), (float)screenWidth, (float)(screenHeight - BAR_Y - BAR_HEIGHT - 10) });
        }

        // Handle click events for registered Lua handlers
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !url.active) {
            Vector2 mouse = GetMousePosition();
            std::lock_guard<std::mutex> lock(g_lua_mutex);
            for (auto const& [node_id, handler_ref] : g_click_handlers) {
                auto rect_opt = doc.getElemRect(node_id);
                if (rect_opt.has_value()) {
                    Rect r = rect_opt.value();
                    if (CheckCollisionPointRec(mouse, { r.x, r.y, r.width, r.height })) {
                        lua_rawgeti(L, LUA_REGISTRYINDEX, handler_ref);
                        if (lua_pcall(L, 0, 0, 0) != 0) {
                            std::cout << "[Lua Error] Click Handler Failed: " << lua_tostring(L, -1) << "\n";
                            lua_pop(L, 1);
                        }
                        break;
                    }
                }
            }
        }

        EndDrawing();
    }

    for (auto& pair : g_texture_cache) {
        if (pair.second.id != 0) UnloadTexture(pair.second);
    }
    g_texture_cache.clear();
    CloseWindow();
}