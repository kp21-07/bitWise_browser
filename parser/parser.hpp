#pragma once
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <optional>


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
// Node Structure
// -------------------------------------------------------------------
struct Node {
    int id;
    NodeType type;
   
    // Common Properties
    std::string bgcolour = "#ffffff";
    int spacing = 1;
    std::string onclick = "";
    
    std::variant<std::monostate, TextData, ImageData, FlexData> specific_data; 
    std::vector<int> children; 
};

// -------------------------------------------------------------------
// JsonValue: Recursive structure to hold nested JSON data
// -------------------------------------------------------------------
struct JsonValue {
    enum Type { STRING, NUMBER, OBJECT, ARRAY, BOOLEAN, NULL_VAL };
    Type type = NULL_VAL;
    std::string_view string_val; 
    double number_val = 0;
    std::unordered_map<std::string_view, JsonValue> object_val; 
    std::vector<JsonValue> array_val;
};

// -------------------------------------------------------------------
// Lexer: Recursive JSON Parser
// -------------------------------------------------------------------
struct Lexer {
    std::string_view source; 
    size_t pos = 0;

    JsonValue parse();

private:
    void skip_whitespace();
    std::string_view read_string(); 
    bool is_space(char c);
    bool is_digit(char c);
};

// -------------------------------------------------------------------
// JSONDocument: Browser's Memory
// -------------------------------------------------------------------
class JSONDocument {
private:
    std::vector<Node*> index_map;
    int next_safe_fallback_id = 1000;
    mutable std::shared_mutex dom_mutex; // For Atomicity

    // Internal Recursive Methods
    int buildNode(const JsonValue& element, std::vector<Node*>& working_map, int& working_id);
    void registerNode(Node* node, std::vector<Node*>& working_map, int& working_id, int explicit_id = -1);

public:
    std::string lua_path;

    // Declarations for methods
    std::optional<Node> getNode(int id);
    void update(std::string raw_jsml);
    
    void dump();
};
