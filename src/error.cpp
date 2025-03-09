#include "string_with_arrows.cpp"

enum class errortype {
    none = -1,
    illegalChar,
    syntax,
};

class ErrorIllegalChar {
    private:
    Token current_token;
    std::string details;

    public:
    ErrorIllegalChar(Token current_token_, std::string details_) : current_token(current_token_), details(details_) {}

    inline void display() {
        std::cout << "ILLEGAL CHARACTER: " << details << " AT LINE: " << current_token.line << " COLUMN: " << current_token.column << "\n";
    }
};

class ErrorSyntax {
    private:
    ParsePosition pos;
    std::string details;

    public:_
    ErrorSyntax(ParsePosition pos_, std::string details_) : pos(pos_), details(details_) {}

    inline void display() {
        std::cout << "ERROR OCCURED AT [" << pos.x << ", " << pos.y << ", " << pos.z << "]\n:";
        std::cout << "\t" << parser_arrows(pos.realStartPos.fileTxt, pos) << "\n";
        std::cout << "SYNTAX ERROR: ";
    }
};
