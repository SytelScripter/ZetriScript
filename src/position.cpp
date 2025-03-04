#include <iostream>

enum specialpos {
    ENTRY,
    POS_DECL
};

class Position {
    private:
    std::string xPos;
    std::string yPos;
    std::string zPos;
    std::string Col;

    public:
    Position(std::string xPos_, std::string yPos_, std::string zPos_, std::string Col_) : xPos(xPos_), yPos(yPos_), zPos(zPos_), Col(Col_) {}
}