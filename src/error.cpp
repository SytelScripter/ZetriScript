#include "string_with_arrows.cpp"
#include "token.cpp"
#pragma once

enum class errortype {
    none = -1,
    illegalChar,
    syntax,
};

class ErrorIllegalChar {
    private:
    Position current_token;
    std::string details;

    public:
    ErrorIllegalChar(Position current_token_, std::string details_) : current_token(current_token_), details(details_) {}

    inline void display() {
        std::cout << "ILLEGAL CHARACTER: " << details << " AT LINE: " << current_token.line << " COLUMN: " << current_token.col << "\n";
    }
};

class ErrorSyntax {
    private:
    Token_ pos;
    std::string details;

    public:
    ErrorSyntax() : details("") {}
    ErrorSyntax(Token_ pos_, const std::string &details_) : pos(pos_), details(details_) {}

    inline void display() {
        std::cout << "ERROR OCCURED AT [" << pos.x << ", " << pos.y << ", " << pos.z << "]\n:";
        std::cout << "\t" << parser_arrows(pos) << "\n";
        std::cout << "SYNTAX ERROR: " << details;
    }

    inline bool isEmpty() const {
        return details.empty();
    }
};
