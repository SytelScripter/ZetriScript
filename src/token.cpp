#include <iostream>
#include <vector>

enum toktype {
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

std::vector<std::string> keywords_list = {
    "ZetriScript",
    "LINE",
    "allocSpace",
    "drawPixel",
    "goto"
};

class Token {
    public:
    toktype type;
    std::string value;

    Token(toktype type_, std::string value_) : type(type_), value(value_) {}
};
