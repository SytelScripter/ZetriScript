#include <iostream>

enum specialpos {
    ENTRY,
    POS_DECL
};

class Position {
    public:
    std::string xPos = "0";
    std::string yPos = "0";
    std::string zPos = "0";
    // std::string Col;
    Position(std::string xPos_, std::string yPos_, std::string zPos_) : xPos(xPos_), yPos(yPos_), zPos(zPos_) {}
    // Position(std::string xPos_, std::string yPos_, std::string zPos_, std::string Col_) : xPos(xPos_), yPos(yPos_), zPos(zPos_), Col(Col_) {}
};