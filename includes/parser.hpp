#pragma once
#include <optional>     // For safe thread-safe copies
#include <shared_mutex> // For dom_mutex
#include <string>       // For specific_data content
#include <string_view>  // For Zero-Copy parsing
#include <unordered_map>// For JsonValue objects
#include <variant>      // For Node specific_data
#include <vector>       // For Node children and index_map

// -------------------------------------------------------------------
// NodeType: Types of Elements
// -------------------------------------------------------------------
enum NodeType {
    ROOT,
    TEXT,
    IMAGE,
    FLEXV,
    FLEXH,
    UNKNOWN
};

// -------------------------------------------------------------------
// Specific Data for different Node types
// -------------------------------------------------------------------
struct TextData {
    std::string content;
    std::string colour = "#000000";
    int fontsize = 14;
};

struct ImageData {
    std::string url;
    std::string alttext;
};

struct FlexData {
    int spacing = 1;
};

// -------------------------------------------------------------------
// Common Utilities
// -------------------------------------------------------------------
struct Rect {
    float x, y, width, height;
};

// -------------------------------------------------------------------
// Node Structure
// -------------------------------------------------------------------
struct Node {
    int id;
    NodeType type;
    std::string tag = ""; // NEW: For browser.getElemsByTag()
    
    // Common Properties
    std::string bgcolour = "#ffffff";
    int spacing = 1;
    std::string onclick = "";
    Rect last_layout = { 0, 0, 0, 0 }; // Stores results of last render pass
    
    std::variant<std::monostate, TextData, ImageData, FlexData> specific_data; 
    std::vector<int> children; 
};

// -------------------------------------------------------------------
// JsonValue: Recursive structure to hold nested JSON data
// -------------------------------------------------------------------
struct JsonValue {
    enum Type { STRING, NUMBER, OBJECT, ARRAY, BOOLEAN, NULL_VAL };
    Type type = NULL_VAL;
    std::string string_val; 
    double number_val = 0;
    std::unordered_map<std::string, JsonValue> object_val; 
    std::vector<JsonValue> array_val;
};

// -------------------------------------------------------------------
// Lexer
// -------------------------------------------------------------------
struct Lexer {
    std::string_view source; 
    size_t pos = 0;
    
    JsonValue parse();

private:
    void skip_whitespace();
    std::string read_string(); 
    bool is_space(char c);
    bool is_digit(char c);
};

// -------------------------------------------------------------------
// JSONDocument: The Browser's DOM Model
// -------------------------------------------------------------------
class JSONDocument {
private:
    // Core Data
    std::vector<Node*> index_map;
    int next_safe_fallback_id = 1000;
    mutable std::shared_mutex dom_mutex;

    // Internal Recursive Builders
    int buildNode(const JsonValue& element, std::vector<Node*>& working_map, int& working_id);
    void registerNode(Node* node, std::vector<Node*>& working_map, int& working_id, int explicit_id = -1);

public:
    std::string lua_path;

    // Public API for UI and Scripts
    std::optional<Node> getNode(int id);
    std::vector<int> getElemsByTag(std::string_view tag_name);
    void update(std::string_view raw_jsml);
    void updateNode(int id, const std::unordered_map<std::string, std::string>& props);
    std::optional<Rect> getElemRect(int id);
    void setElemRect(int id, Rect rect);
    void dump();
};

