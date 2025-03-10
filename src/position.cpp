#include <iostream>

enum specialpos {
    ENTRY,
    POS_DECL,
    POSITION
};

class TokenPosition {
    public:
    int line = 0;
    int col = 0;
    std::string fileTxt = "";

    TokenPosition(int x = 0) {}
    TokenPosition(std::string fileTxt_, int col_, int line_) : fileTxt(fileTxt_), col(col_), line(line_) {}
};

class ParsePosition {
    public:
    specialpos specialPos;
    std::string x;
    std::string y;
    std::string z;

    ParsePosition(int x = 0) {}
    ParsePosition(specialpos specialPos_) : specialPos(specialPos_) {}
    ParsePosition(std::string xPos_, std::string yPos_, std::string zPos_) : x(xPos_), y(yPos_), z(zPos_) {}

    bool isSpecial() {
        if (!std::empty(x) && !std::empty(y) && !std::empty(z_)) {
            return false;
        }
        return true;
    }
};