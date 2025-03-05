#include <iostream>
#include <vector>
#include <array>
#include <string>

enum class toktype {
    none = -1,
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

    Token_() : type(toktype::none), value("") {}
    Token_(toktype type_) : type(type_), value("") {}
    Token_(toktype type_, std::string value_) : type(type_), value(value_) {}
};
