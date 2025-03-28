#include <iostream>
#include <vector>
#include <array>
#include <string>
#include "position.cpp"

enum class toktype {
    none = -1,
    eof_,
    keyword,
    const_builtin,
    class_builtin,
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

std::array<std::string, 3> keywords_list = {
    "system",
    "command",
    "recall"
};

std::array<std::string, 3> const_list = {
    "Euclidean",
    "Equation",
    "Parametric",
};

std::array<std::string, 3> class_list = {
    "Point",
    "Line",
    "Plane"
};

class Token_ {
    public:
    toktype type;
    std::string value;
    Position posStart, posEnd;
    bool hasValue;

    Token_(int x = 0) : type(toktype::none), value("") {}
    Token_(Position posStart_, Position posEnd_, toktype type_) : posStart(posStart_), posEnd(posEnd_), type(type_), value("") { hasValue = false; }
    Token_(Position posStart_, Position posEnd_, toktype type_, std::string value_) : posStart(posStart_), posEnd(posEnd_), type(type_), value(value_) { hasValue = true; }

    std::array<Position, 2> get_position() {
        std::array<Position, 2> pos = {{posStart, posEnd}};
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

    bool operator==(const Token_& other) const {
        return type == other.type && value == other.value && posStart == other.posStart && posEnd == other.posEnd;
    }
};

inline std::string toktype_to_string(toktype type) {
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