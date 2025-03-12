#include "error.cpp"
#include <algorithm>
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

    inline void addToken_(toktype type_, char value_) {
        std::string value = std::string(1, value_);
        TokenPosition posStart = TokenPosition(idx);
        advance();
        TokenPosition posEnd = TokenPosition(idx - 1);
        Token_ result = Token_(posStart, posEnd, type_, value);
        tokens.push_back(result);
    }

public:
    Lexer(std::string text_) : text(text_) {
        advance();
    }

    std::vector<Token_> makeTokens() {
        std::vector<Token_> tokens;
        
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
            else if (currentChar == '{') addToken_(toktype::left_curly, '{');
            else if (currentChar == '}') addToken_(toktype::right_curly, '}');
            else if (currentChar == '[') addToken_(toktype::left_square, '[');
            else if (currentChar == ']') addToken_(toktype::right_square, ']');
            else if (currentChar == '+') addToken_(toktype::plus, '+');
            else if (currentChar == '-') addToken_(toktype::minus, '-');
            else if (currentChar == '*') addToken_(toktype::mul, '*');
            else if (currentChar == '/') addToken_(toktype::div, '/');
            else if (currentChar == '(') addToken_(toktype::left_paren, '(');
            else if (currentChar == ')') addToken_(toktype::right_paren, ')');
            else if (currentChar == ':') addToken_(toktype::colon, ':');
            else if (currentChar == ';') addToken_(toktype::semicolon, ';');
            else if (currentChar == '=') addToken_(toktype::equals, '=');
            else if (currentChar == '!') addToken_(toktype::exc_mark, '!');
            else if (currentChar == ',') addToken_(toktype::comma, ',');
            else if (currentChar == '.') addToken_(toktype::dot, '.');
        }

        addToken_(toktype::eof_, '\0');

        return tokens;
    }

    Token_ makeText() {
        std::string text = "";
        TokenPosition posStart = TokenPosition(idx);
        while (idx < text.length() && std::isalpha(currentChar)) {
            text += currentChar;
            advance();
        }
        TokenPosition posEnd = TokenPosition(idx - 1);
        if (std::find(keywords_list.begin(), keywords_list.end(), text) != keywords_list.end()) {
            Token_ result = Token_(posStart, posEnd, toktype::keyword, text);
            return result;
        }
        Token_ result = Token_(posStart, posEnd, toktype::name, text);
        return result;
    }

    Token_ makeNumber() {
        std::string num_str = "";
        int dot_count = 0;
        TokenPosition posStart = TokenPosition(idx);

        while (idx < text.length() && (std::isdigit(currentChar) || currentChar == '.')) {
            if (currentChar == '.') {
                if (dot_count == 1) break;
                dot_count++;
            }
            num_str += currentChar;
            advance();
        }
        TokenPosition posEnd = TokenPosition(idx - 1);
        if (dot_count == 0) {
            Token_ result = Token_(posStart, posEnd, toktype::int_lit, num_str);
            return result;
        }
        Token_ result = Token_(posStart, posEnd, toktype::float_lit, num_str);
        return result;
    }
};