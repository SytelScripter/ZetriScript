#include <iostream>
#include "position.cpp"


namespace pre_str {
    std::string repeat(const std::string& str, int count) {
        std::string result = "";
        for (int i = 0; i < count; i++) {
            result += str;
        }
        return result;
    }
}

inline std::string token_arrows(std::string str, Token_ token) {
    return string_with_arrows(str, token.posStart, token.posEnd);
}

inline std::string string_with_arrows(std::string str, Position start, Position end) {
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
            result += pre_str::repeat(" ", start.col - 1) + pre_str::repeat("^", end.col - start.col) + "\n";
            break;
        }
        if (line == start.line && end.line != start.line) {
            int next_line_idx = i + 1;
            while (str[next_line_idx] != '\n') {
                next_line_idx++;
            }
            result += str.substr(start_line_idx, next_line_idx - start_line_idx);
            result += "\n";
            result += pre_str::repeat(" ", start.col - 1) + pre_str::repeat("^", next_line_idx - start_line_idx) + "\n";
            i += next_line_idx - start_line_idx;
        }
        else if (line > start.line && line < end.line && col == 0) {
            int next_line_idx = i + 1;
            while (str[next_line_idx] != '\n') {
                next_line_idx++;
            }
            result += str.substr(start_line_idx, next_line_idx - start_line_idx);
            result += "\n";
            result += pre_str::repeat("^", next_line_idx - start_line_idx) + "\n";
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
            result += pre_str::repeat("^", end_idx - start_line_idx) + "\n";
            break;
        }
        i++;
    }
    return result;
}
