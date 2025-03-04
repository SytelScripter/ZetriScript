#include "token.cpp"
#include <vector>
#include <cctype>

class Lexer {
private:
    int idx = -1;
    char currentChar;
    std::string text;
    inline void advance() {
        idx++;
        if (idx < text.length()) {
            currentChar = text[idx];
        }
    }
public:
    Lexer(std::string text_) : text(text_) {}

    std::vector<Tokens> makeTokens() {
        std::vector<Token> tokens;
        
        while (idx < text.length()) {
            if (currentChar == ' ' || currentChar == '\n' || currentChar == '\t') {
                advance();
            }
            else if (std::isalpha(currentChar)) {
                tokens.push_back(makeText());
            }
            else if (std::isdigit(currentChar)) {
                tokens.push_back(makeNumber());
            }
        }

        return tokens;
    }

    Token makeText() {
        std::string text = "";
        Token result;
        while (idx < text.length() && std::isalpha(currentChar)) {
            text += currentChar;
            advance();
        }
        if (std::count(keywords_list.begin(), keywords_list.end(), text) > 0) {
            result.type = toktype::keyword;
        }
        else {
            result.type = toktype::name;
        }
        result.value = text;
        return result;
    }

    Token makeNumber() {
        std::string num_str = "";
        int dot_count;
        Token result;
        while (idx < text.length() && (std::isdigit(currentChar) || currentChar == '.')) {
            if (currentChar == '.') {
                if (dot_count == 1) break;
                dot_count++;
            }
            num_str += currentChar;
            advance();
        }
        if (dot_count)
    }
}