#include <iostream>
#pragma once

enum specialpos {
    UNKNOWN = -1,
    ENTRY,
    POS_DECL,
    POSITION
};

class TokenPosition {
    public:
    int line = 0;
    int col = 0;
    int idx = 0;
    std::string fileTxt = "";

    TokenPosition(int x = 0) {}
    TokenPosition(std::string fileTxt_, int idx_) : fileTxt(fileTxt_), idx(idx_) {}

    void findLineCol() {
        int i = 0;
        while (i < idx) {
            if (fileTxt[i] == '\n') {
                line++;
                col = 0;
            } else {
                col++;
            }
            i++;
        }
    }
};

class ParsePosition {
    public:
    specialpos specialPos;
    std::string x;
    std::string y;
    std::string z;
    TokenPosition realStartPos;
    TokenPosition realEndPos;

    ParsePosition(int x = 0) {}
    ParsePosition(specialpos specialPos_) : specialPos(specialPos_) {}
    ParsePosition(std::string xPos_, std::string yPos_, std::string zPos_) : x(xPos_), y(yPos_), z(zPos_) {}
    ParsePosition(std::string xPos_, std::string yPos_, std::string zPos_, TokenPosition realStartPos_, TokenPosition realEndPos_) : x(xPos_), y(yPos_), z(zPos_), realStartPos(realStartPos_), realEndPos(realEndPos_) {}

    bool isSpecial() {
        if (!std::empty(x) && !std::empty(y) && !std::empty(z)) {
            return false;
        }
        return true;
    }
};

inline std::string specialPosToString(specialpos pos) {
    switch (pos) {
        case ENTRY:
            return "ENTRY";
        case POS_DECL:
            return "POS_DECL";
        case POSITION:
            return "POSITION";
        default:
            return "UNKNOWN";
    }
}
