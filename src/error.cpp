#include "position.cpp"

enum errortype {
    illegalChar,
    syntax,
}

class Error {
    private:
    std::variant<Position, specialpos> pos;
    errortype errorType;
    std::string description;

    public:
    Error(std::variant<Position, specialpos> pos_, errortype errorType_, std::string description_) : pos(pos_), errorType(errorType_), description(description_) {}

    inline void display(std::string details) {
        std::visit([](auto&& value) {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, Position) {
                std::cout << "ERROR OCCURED AT: [" << value.xPos << ":" << value.yPos << ":" << value.zPos << "]\n";
                std::cout << "IN CODE POSITION AT: " << Col << "\n";
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, specialpos) {
                if (value == specialpos::ENTRY) {
                    std::cout << "ERROR OCCURED AT: ENTRY";
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
}