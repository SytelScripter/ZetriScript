#include <iostream>

enum specialpos {
    ENTRY,
    POS_DECL,
    POSITION
};

class TokenPosition {
    public:
    std::string line = 0;
    std::string col = 0;

    TokenPosition(int x = 0) {}
    TokenPosition(specialpos specialPos_) : specialPos(specialPos_) {}
    TokenPosition(std::string col_) : col(col_) {}
    TokenPosition(std::string xPos_, std::string yPos_, std::string zPos_, std::string col_) : xPos(xPos_), yPos(yPos_), zPos(zPos_), col(col_) {}
};

class ParsePosition {
    public:
    specialpos specialPos;
    std::string x;
    std::string y;
    std::string z;
    TokenPosition realPos;

    ParsePosition(int x = 0) {}
    ParsePosition(specialpos specialPos_) : specialPos(specialPos_) {}
    ParsePosition(std::string xPos_, std::string yPos_, std::string zPos_, TokenPosition realPos_) : x(xPos_), y(yPos_), z(zPos_), realPos(realPos_) {}

    boolean isSpecial() {
        if (!empty(x) && !empty(y) && !empty(z_)) {
            return false;
        }
        return true;
    }
};