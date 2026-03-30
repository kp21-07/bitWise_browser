#include "parser.hpp"
#include "lua_bridge.hpp"
#include <iostream>
#include <shared_mutex>
#include <string>
#include <variant>

void JSONDocument::dump() {
    std::shared_lock<std::shared_mutex> lock(dom_mutex);  // Lock for reading
    
    std::cout << "\n[DOM DUMP] Printing all successfully discovered nodes:\n";
    for (Node* n : index_map) {
        if (n == nullptr) continue;
        
        std::cout << "  -> Node ID [" << n->id << "] Type: ";
        switch(n->type) {
            case ROOT: std::cout << "ROOT"; break;
            case TEXT: std::cout << "TEXT"; break;
            case IMAGE: std::cout << "IMAGE"; break;
            case FLEXV: std::cout << "FLEXV"; break;
            default: std::cout << "UNKNOWN"; break;
        }

        if (!n->tag.empty()) std::cout << " | Tag: [" << n->tag << "]";
        std::cout << " | BG: " << n->bgcolour;
        if (n->spacing != 1) std::cout << " | Spacing: " << n->spacing;
        if (!n->onclick.empty()) std::cout << " | OnClick: " << n->onclick;

        if (n->type == TEXT) {
            if (auto* txt = std::get_if<TextData>(&n->specific_data)) {
                std::cout << " | Content: \"" << txt->content << "\"";
            }
        }
        std::cout << "\n";
    }
}

int main() {

    std::string jsml_payload = R"jsml(

{
    "lua": "script.lua",
    "root": {
        "flexv": [
            {
                "text": {
                    "content": "Hello, world!",
                    "fontsize": 14,
                    "colour": "#000000"
                },
                "id" : 1,
                "bgcolour": "#fffffff",
                "spacing": 1,
                "onclick" : "functionabcd()"
            },
            {
                "image": {
                    "url": "/cat.png",
                    "alttext": "Cat Image"
                },
                "bgcolour": "#fffffff",
                "spacing": 1
            }
        ]
    }
}
)jsml";

    JSONDocument document;
    std::cout << "[SYSTEM] Network Engineer parsing new page...\n";
    document.update(jsml_payload);

    // 3. RUN THE DIRECTOR (LUA SCRIPT)
    std::cout << "[SYSTEM] Starting Lua Director...\n";
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // Register our C++ API into the Lua world (Snapshot & Commit Bridge)
    open_browser_api(L, &document);

    // Run the specified script
    if (luaL_dofile(L, document.lua_path.c_str())) {
        std::cout << " [ERROR] Lua: " << lua_tostring(L, -1) << "\n";
    }

    // 4. VERIFY FINAL STATE
    document.dump();
    
    lua_close(L);
    return 0;
}
