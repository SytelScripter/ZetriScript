#include <iostream>
#include <vector>
#include <array>
#include <string>
#include "position.cpp"

enum class toktype {
    none = -1,
    eof_,
    keyword,
    left_square,
    right_square,
    left_curly,
    right_curly,
    semicolon,
    plus,
    minus,
    mul,
    div,
    colon,
    equals,
    left_paren,
    right_paren,
    int_lit,
    float_lit,
    name,
    exc_mark,
    comma,
    dot
};

std::array<std::string, 5> keywords_list = {
    "ZetriScript",
    "LINE",
    "allocSpace",
    "drawPixel",
    "goto"
};

class Token_ {
    public:
    toktype type;
    std::string value;
    TokenPosition posStart, posEnd;
    boolean hasValue;

    Token_() : type(toktype::none), value("") {}
    Token_(TokenPosition posStart_, TokenPosition posEnd_, toktype type_) : posStart(posStart_), posEnd(posEnd_), type(type_), value("") { hasValue = false; }
    Token_(TokenPosition posStart_, TokenPosition posEnd_, toktype type_, std::string value_) : posStart(posStart_), posEnd(posEnd_), type(type_), value(value_) { hasValue = true; }

    std::array<TokenPosition, TokenPosition> get_position() {
        std::array<TokenPosition, TokenPosition> pos = {{posStart, posEnd}};
        return pos;
    }

    inline std::string tok_type_to_string(toktype type) {
        switch (type) {
            case toktype::none: return "none";
            case toktype::eof_: return "EOF";
            case toktype::keyword: return "keyword";
            case toktype::left_square: return "[";
            case toktype::right_square: return "]";
            case toktype::left_curly: return "{";
            case toktype::right_curly: return "}";
            case toktype::semicolon: return ";";
            case toktype::plus: return "+";
            case toktype::minus: return "-";
            case toktype::mul: return "*";
            case toktype::div: return "/";
            case toktype::colon: return ":";
            case toktype::equals: return "=";
            case toktype::left_paren: return "(";
            case toktype::right_paren: return ")";
            case toktype::int_lit: return "int_lit";
            case toktype::float_lit: return "float_lit";
            case toktype::name: return "name";
            case toktype::exc_mark: return "!";
            case toktype::comma: return ",";
            case toktype::dot: return ".";
            default: return "Unknown";
        }
    }

    std::string to_string() {
        std::string result = "";
        result += "Token: ";
        result += tok_type_to_string(type);
        result += ", Value: ";
        result += value;
        result += ", Position: ";
        result += "(" + std::to_string(posStart.line) + ", " + std::to_string(posStart.col) + ") - (" + std::to_string(posEnd.line) + ", " + std::to_string(posEnd.col) + ")";
        return result;
    }
};
