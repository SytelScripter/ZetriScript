#include <iostream>

enum specialpos {
    ENTRY,
    POS_DECL
};

class Position {
    public:
    std::string xPos;
    std::string yPos;
    std::string zPos;
    // std::string Col;

    Position() : xPos("0"), yPos("0"), zPos("0") {}
    Position(std::string xPos_, std::string yPos_, std::string zPos_) : xPos(xPos_), yPos(yPos_), zPos(zPos_) {}
    // Position(std::string xPos_, std::string yPos_, std::string zPos_, std::string Col_) : xPos(xPos_), yPos(yPos_), zPos(zPos_), Col(Col_) {}
};