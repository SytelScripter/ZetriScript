#include "token.cpp"
#include <vector>
#include <cctype>

class Lexer {
private:
    int idx = -1;
    char currentChar;
    std::string text;
    std::vector<Token_> tokens;
    inline void advance() {
        idx++;
        if (idx < text.length()) {
            currentChar = text[idx];
        }
    }

    inline void addToken_(toktype type_) {
        Token_ result = Token_(type_);
        tokens.push_back(result);
        advance();
    }

public:
    Lexer(std::string text_) : text(text_) {
        advance();
    }

    std::vector<Token_> makeToken_s() {
        std::vector<Token_> tokens;
        
        while (idx < text.length()) {
            if (currentChar == ' ' || currentChar == '\n' || currentChar == '\t') {
                advance();
            }
            else if (std::isalpha(currentChar)) {
                Token_s.push_back(makeText());
            }
            else if (std::isdigit(currentChar)) {
                Token_s.push_back(makeNumber());
            }
            else if (currentChar == '{') addToken_(toktype::left_curly);
            else if (currentChar == '}') addToken_(toktype::right_curly);
            else if (currentChar == '[') addToken_(toktype::left_square);
            else if (currentChar == ']') addToken_(toktype::right_square);
            else if (currentChar == '(') addToken_(toktype::left_paren);
            else if (currentChar == ')') addToken_(toktype::right_paren);
            else if (currentChar == ':') addToken_(toktype::colon);
            else if (currentChar == ';') addToken_(toktype::semicolon);
            else if (currentChar == '-') addToken_(toktype::dash);
            else if (currentChar == '=') addToken_(toktype::equals);
            else if (currentChar == '!') addToken_(toktype::exc_mark);
            else if (currentChar == ',') addToken_(toktype::comma);
            else if (currentChar == '.') addToken_(toktype::dot);
        }

        return Token_s;
    }

    Token_ makeText() {
        std::string text = "";
        while (idx < text.length() && std::isalpha(currentChar)) {
            text += currentChar;
            advance();
        }
        if (std::count(keywords_list.begin(), keywords_list.end(), text) > 0) {
            Token_ result = Token_(toktype::keyword, text);
            return result;
        }
        Token_ result = Token_(toktype::name, text);
        return result;
    }

    Token_ makeNumber() {
        std::string num_str = "";
        int dot_count;
        
        while (idx < text.length() && (std::isdigit(currentChar) || currentChar == '.')) {
            if (currentChar == '.') {
                if (dot_count == 1) break;
                dot_count++;
            }
            num_str += currentChar;
            advance();
        }
        if (dot_count == 0) Token_ result = Token_(toktype::int_lit);
        else result.type = toktype::float_lit;
        result.value = num_str;
        return result;
    }
};