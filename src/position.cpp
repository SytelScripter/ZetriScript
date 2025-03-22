#include <iostream>
#pragma once

enum specialpos {
    UNKNOWN = -1,
    ENTRY,
    POS_DECL,
    POSITION
};

class Position {
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
