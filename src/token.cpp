#include <iostream>
#include <vector>
#include <array>
#include <string>
#include "TokenPosition.cpp"

enum class toktype {
    none = -1,
    eof_,
    keyword,
    left_square,
    right_square,
    left_curly,
    right_curly,
    semicolon,
    dash,
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

    Token_() : type(toktype::none), value("") {}
    Token_(TokenPosition posStart_, TokenPosition posEnd_, toktype type_) : posStart(posStart_), posEnd(posEnd_), type(type_), value("") {}
    Token_(TokenPosition posStart_, TokenPosition posEnd_, toktype type_, std::string value_) : posStart(posStart_), posEnd(posEnd_), type(type_), value(value_) {}

    std::array<2, TokenPosition> get_position() {
        std::array<2, TokenPosition> pos = {{posStart, posEnd}};
        return pos;
    }

    std::string to_string() {
        std::string result = "";
        result += "Token: ";
        result += tok_type_to_string(type);
        result += ", Value: ";
        result += value;
        result += ", Position: ";
        result += "(" + std::to_string(posStart.line) + ", " + std::to_string(posStart.column) + ") - (" + std::to_string(posEnd.line) + ", " + std::to_string(posEnd.column) + ")";
        return result;
    }
};
