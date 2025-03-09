#include <iostream>
#include "position.cpp"

inline std::string token_arrows(std::string str, TokenPosition start, TokenPosition end) {
    int line = 0;
    int col = 0;
    int i = 0;
    std::string result = "";
    
    // example:
    // HELLO WORLD
    //    ^^^^^^^^
    // HELLO USER
    // ^^^


    // there are:
    // starting lines
    // between lines
    // end lines

    int start_line_idx;
    while (i < str.size()) {
        if (str[i] == '\n') {
            line++;
            col = 0;
            start_line_idx = i + 1;
        } else {
            col++;
        }
        if (line == start.line && line == end.line) {
            int next_line_idx = i + 1;
            while (str[next_line_idx] != '\n') {
                next_line_idx++;
            }
            result += str.substr(start_line_idx, next_line_idx - start_line_idx);
            result += "\n";
            result += " ".repeat(start.col - 1) + "^".repeat(end.col - start.col) + "\n";
            break;
        }
        if (line == start.line && end.line != start.line) {
            int next_line_idx = i + 1;
            while (str[next_line_idx] != '\n') {
                next_line_idx++;
            }
            result += str.substr(start_line_idx, next_line_idx - start_line_idx);
            result += "\n";
            result += " ".repeat(start.col - 1) + "^".repeat(next_line_idx - start_line_idx) + "\n";
            i += next_line_idx - start_line_idx;
        }
        else if (line > start.line && line < end.line && col == 0) {
            int next_line_idx = i + 1;
            while (str[next_line_idx] != '\n') {
                next_line_idx++;
            }
            result += str.substr(start_line_idx, next_line_idx - start_line_idx);
            result += "\n";
            result += "^".repeat(next_line_idx - start_line_idx) + "\n";
            i += next_line_idx - start_line_idx;
        }
        else if (line == end.line && line != start.line) {
            int next_line_idx = i + 1;
            while (str[next_line_idx] != '\n') {
                next_line_idx++;
            }
            int end_idx = start_line_idx + end.col;
            result += str.substr(start_line_idx, next_line_idx - start_line_idx);
            result += "\n";
            result += "^".repeat(end_idx - start_line_idx) + "\n";
            break;
        }
        i++;
    }
    return result;
}

inline std::string parser_arrows(ParsePosition pos) {
    std::string result = token_arrows(pos.realStartPos.fileTxt, pos.realStartPos, pos.realEndPos);
    return result;
}