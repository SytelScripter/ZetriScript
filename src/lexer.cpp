#include "token.cpp"
#include <vector>
#include <cctype>

class Lexer {
private:
    int idx = -1;
    char currentChar;
    std::string text;
    std::vector<Tokens> tokens;
    inline void advance() {
        idx++;
        if (idx < text.length()) {
            currentChar = text[idx];
        }
    }

    inline void addToken(toktype type_) {
        Token result;
        result.type = type_;
        tokens.push_back(result);
        advance();
    }

public:
    Lexer(std::string text_) : text(text_) {
        advance();
    }

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
            else if (currentChar == '{') addToken(toktype::left_curly);
            else if (currentChar == '}') addToken(toktype::right_curly);
            else if (currentChar == '[') addToken(toktype::left_square);
            else if (currentChar == ']') addToken(toktype::right_square);
            else if (currentChar == '(') addToken(toktype::left_paren);
            else if (currentChar == ')') addToken(toktype::right_paren);
            else if (currentChar == ':') addToken(toktype::colon);
            else if (currentChar == ';') addToken(toktype::semicolon);
            else if (currentChar == '-') addToken(toktype::dash);
            else if (currentChar == '=') addToken(toktype::equals);
            else if (currentChar == '!') addToken(toktype::exc_mark);
            else if (currentChar == ',') addToken(toktype::comma);
            else if (currentChar == '.') addToken(toktype::dot);
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
        if (dot_count == 0) result.type = toktype::int_lit;
        else result.type = toktype::float_lit;
        result.value = num_str;
        return reuslt;
    }
};