#include "string_with_arrows.cpp"

enum class errortype {
    none = -1,
    illegalChar,
    syntax,
};

class Error {
    private:
    Token current_token;
    ParsePosition pos;
    errortype errorType;
    std::string description;
    boolean 

    public:
    Error(Token current_token_, ParsePosition pos_, errortype errorType_, std::string description_) : current_token(current_token_), pos(pos_), errorType(errorType_), description(description_) {}
    Error(Token current_token_, errortype errorType_, std::string description_) : current_token(current_token_), errorType(errorType_), description(description_) {}

    inline void display(std::string details) {
        std::visit([](auto&& value) {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, TokenPosition) {
                std::cout << value.to_string();
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, ParsePosition) {
                if (value.isSpecial()) {
                    if (value.)
                }
                else {
                    std::cout << "ERROR OCCURED AT: [" << value.xPos << ":" << value.yPos << ":" << value.zPos << "]\n";
                    std::cout << "IN CODE POSITION AT: " << Col << "\n";
                }
            }
        })

        if (errorType == illegalChar) {
            std::cout << "ILLEGAL CHARACTER: " << details;
        }
        else if (errorType == syntax) {
            std::cout << "INVALID SYNTAX: " << details;
        }
    }

    inline bool empty() {
        if (errorType != none || !description.empty() || std::holds_alternative<Position>(pos) || std::holds_alternative<specialpos>(pos)) {
            return true;
        }
        return false;
    }
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

};