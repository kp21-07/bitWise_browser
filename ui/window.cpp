#include "raylib.h"
#include "../includes/window.hpp"
#include <iostream>
#include "../includes/parser.hpp"
#include "../includes/network.hpp"
#include <lua.hpp>

void RunWindow(JSONDocument& doc, lua_State* L){

    const int screenWidth = 1024;
    const int screenHeight = 768;

    InitWindow(screenWidth, screenHeight, "JSML Browser");
    SetTargetFPS(60);

    Rectangle urlBar = { 20, 20, (float)screenWidth - 40, 40 };
    std::string urlText = "http://";
    bool isUrlBarActive = false;

    while (!WindowShouldClose()) {
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

            if(IsKeyPressed(KEY_ENTER)){
                std::cout << "[UI] Navigating to: " << urlText << "\n";
                fetch(urlText, [&](std::string payload) {
                    std::cout << "[UI] Network Response Received. Updating DOM...\n";
                    doc.update(payload);

                    std::cout << "[UI] Executing Lua Director: " << doc.lua_path << "\n";
                    if (luaL_dofile(L, doc.lua_path.c_str())) {
                        std::cout << "[UI Error] Lua Script Failed: " << lua_tostring(L, -1) << "\n";
                    }
                });
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

        EndDrawing();
    }

    CloseWindow();
}

