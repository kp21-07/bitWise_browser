#include "../includes/parser.hpp"
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>


// -------------------------------------------------------------------
// Lexer implementation
// -------------------------------------------------------------------
bool Lexer::is_space(char c) {
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f');
}

bool Lexer::is_digit(char c) {
    return (c >= '0' && c <= '9');
}

void Lexer::skip_whitespace() {
    while (pos < source.length() && (is_space(source[pos]) || source[pos] == '/')) {
        if (source[pos] == '/') {
            if (pos + 1 < source.length() && source[pos + 1] == '/') {
                while (pos < source.length() && source[pos] != '\n') pos++;
            } else break;
        } else pos++;
    }
}

std::string_view Lexer::read_string() {
    if (source[pos] == '"') pos++; 
    size_t start = pos;
    while (pos < source.length() && source[pos] != '"') pos++;
    std::string_view result = source.substr(start, pos - start);
    if (pos < source.length()) pos++; 
    return result;
}

JsonValue Lexer::parse() {
    skip_whitespace();
    JsonValue val;
    
    if (source[pos] == '{') {
        val.type = JsonValue::OBJECT;
        pos++;
        while (pos < source.length() && source[pos] != '}') {
            skip_whitespace();
            if (source[pos] == '"') {
                std::string_view key = read_string();
                skip_whitespace();
                if (source[pos] == ':') pos++;
                val.object_val[key] = parse();
            }
            skip_whitespace();
            if (source[pos] == ',') pos++;
            skip_whitespace();
        }
        if (pos < source.length()) pos++;
    }
    else if (source[pos] == '[') {
        val.type = JsonValue::ARRAY;
        pos++;
        while (pos < source.length() && source[pos] != ']') {
            val.array_val.push_back(parse());
            skip_whitespace();
            if (source[pos] == ',') pos++;
            skip_whitespace();
        }
        if (pos < source.length()) pos++;
    }
    else if (source[pos] == '"') {
        val.type = JsonValue::STRING;
        val.string_val = read_string();
    }
    else {
        val.type = JsonValue::NUMBER;
        size_t start = pos;
        while (pos < source.length() && (is_digit(source[pos]) || source[pos] == '.' || source[pos] == '-')) pos++;
        try { 
            std::string s(source.substr(start, pos - start)); // stod needs a string
            if (!s.empty()) val.number_val = std::stod(s);
        } catch (...) {}
    }
    return val;
}

// -------------------------------------------------------------------
// JSONDocument implementation (Snapshot & Commit)
// -------------------------------------------------------------------
std::optional<Node> JSONDocument::getNode(int id) {
    std::shared_lock<std::shared_mutex> lock(dom_mutex);
    if (id >= 0 && id < (int)index_map.size() && index_map[id] != nullptr) {
        // Returns a COPY of the node.
        return *index_map[id];
    }
    return std::nullopt;
}

std::vector<int> JSONDocument::getElemsByTag(std::string_view tag_name) {
    // Search API for Lua scripts.
    std::shared_lock<std::shared_mutex> lock(dom_mutex);
    std::vector<int> found_ids;
    for (Node* n : index_map) {
        if (n != nullptr && n->tag == tag_name) {
            found_ids.push_back(n->id);
        }
    }
    return found_ids;
}

void JSONDocument::update(std::string_view raw_jsml) {
    // {Snapshot] Build new DOM in a private working map, so as to not affect UI during this process
    std::vector<Node*> working_map;
    int working_id = 1000;

    Lexer lexer;
    lexer.source = raw_jsml;

    JsonValue root_document = lexer.parse();
    
    std::string new_lua_path = "";
    if (root_document.object_val.count("lua")) {
        new_lua_path = std::string(root_document.object_val.at("lua").string_val);
    }

    if (root_document.object_val.count("root")) {
        buildNode(root_document.object_val.at("root"), working_map, working_id);
    }

    // [Commit] Swap pointers during lock
    std::vector<Node*> old_map_to_delete;
    {
        std::unique_lock<std::shared_mutex> lock(dom_mutex);
        
        old_map_to_delete = std::move(index_map); 
        index_map = std::move(working_map);
        
        this->lua_path = std::move(new_lua_path);
        this->next_safe_fallback_id = working_id;
    } // Unlock happens automatically here

    for (Node* n : old_map_to_delete) {
        delete n;
    }
}

int JSONDocument::buildNode(const JsonValue& element, std::vector<Node*>& working_map, int& working_id) {
    if (element.type != JsonValue::OBJECT) return -1;
    
    Node* current_node = new Node();
    int explicit_id = -1; 
    
    // Parse Standard Base Properties
    if (element.object_val.count("id")) {
        explicit_id = (int)element.object_val.at("id").number_val;
    }
    if (element.object_val.count("tag")) {
        current_node->tag = std::string(element.object_val.at("tag").string_val);
    }
    if (element.object_val.count("bgcolour")) {
        current_node->bgcolour = std::string(element.object_val.at("bgcolour").string_val);
    }
    if (element.object_val.count("spacing")) {
        current_node->spacing = (int)element.object_val.at("spacing").number_val;
    }
    if (element.object_val.count("onclick")) {
        current_node->onclick = std::string(element.object_val.at("onclick").string_val);
    }
    
    // Parse Type Logic
    if (element.object_val.count("text")) {
        current_node->type = NodeType::TEXT;
        const JsonValue& text_data = element.object_val.at("text");
        TextData td;
        if (text_data.object_val.count("content")) 
            td.content = std::string(text_data.object_val.at("content").string_val);
        if (text_data.object_val.count("colour")) 
            td.colour = std::string(text_data.object_val.at("colour").string_val);
        if (text_data.object_val.count("fontsize")) 
            td.fontsize = (int)text_data.object_val.at("fontsize").number_val;
        current_node->specific_data = td;
    } 
    else if (element.object_val.count("image")) {
        current_node->type = NodeType::IMAGE;
        const JsonValue& img_data = element.object_val.at("image");
        ImageData idat;
        if (img_data.object_val.count("url")) 
            idat.url = std::string(img_data.object_val.at("url").string_val);
        if (img_data.object_val.count("alttext")) 
            idat.alttext = std::string(img_data.object_val.at("alttext").string_val);
        current_node->specific_data = idat;
    }
    else if (element.object_val.count("flexv")) {
        current_node->type = NodeType::FLEXV;
        const JsonValue& children_array = element.object_val.at("flexv");
        if (children_array.type == JsonValue::ARRAY) {
            for (const auto& child_json : children_array.array_val) {
                int child_id = buildNode(child_json, working_map, working_id); 
                if (child_id != -1) {
                    current_node->children.push_back(child_id);
                }
            }
        }
    }
    else if (element.object_val.count("flexh")) {
        current_node->type = NodeType::FLEXH;
        const JsonValue& children_array = element.object_val.at("flexh");
        if (children_array.type == JsonValue::ARRAY) {
            for (const auto& child_json : children_array.array_val) {
                int child_id = buildNode(child_json, working_map, working_id); 
                if (child_id != -1) {
                    current_node->children.push_back(child_id);
                }
            }
        }
    }
    
    registerNode(current_node, working_map, working_id, explicit_id);
    return current_node->id;
}

void JSONDocument::registerNode(Node* node, std::vector<Node*>& working_map, int& working_id, int explicit_id) {
    if (explicit_id != -1) {
        node->id = explicit_id;
        if (explicit_id >= working_id) {
            working_id = explicit_id + 1;
        }
    } else {
        node->id = working_id++;
    }
    
    if (node->id >= (int)working_map.size()) {
        working_map.resize(node->id + 1, nullptr);
    }
    working_map[node->id] = node;
}
