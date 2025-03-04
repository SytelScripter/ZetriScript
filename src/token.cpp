#include <iostream>

enum toktype {
    keyword,
    left_square,
    right_square,
    semicolon,
    dash,
    colon,
    equals,
    left_paren,
    right_paren,
    integer,
    name,
    exc_mark,
    comma,
    dot
};

std::vector<std::string> keywords_list = {
    "PolyScript",
    "LINE",
    "allocSpace",
    "drawPixel"
};

class Token {
    public:
    toktype type;
    std::string value;

    Token(toktype type_, std::string value_) : type(type_), value(value_) {}
}
