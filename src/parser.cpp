#include "lexer.cpp"
#include "error.cpp"
#include <memory>
#include <variant>

struct NodeNumber {
    Token numTok;
}

struct NodePosition {
    std::string x;
    std::string y;
    std::string z;
};

struct NodeLine {
    NodePosition startPoint;
    NodePosition Vector3;
};

struct NodeIdentifier {
    Token identName;
};

struct NodeVarAssign {
    NodeIdentifier ident;
    std::variant<NodeLine, NodeIdentifier> value;
};

struct NodeAlloc {
    NodeIdentifier allocated;
};

struct NodeProg {
    Token header;
    std::string startingPosition;
    std::variant<NodeAlloc, NodeVarAssign, NodeIdentifier, NodeLine, NodePosition, NodeNumber> content;
};

class Parser {
    private:
    int idx = -1;
    Token currentToken;
    std::vector<Token> tokens;

    inline void advance() {
        idx++;
        if (idx < tokens.size()) {
            currentToken = tokens[idx];
        }
    }

    inline bool isTok(toktype type_, std::string value_) {
        if (type_ == currentToken.type && value_ == currentToken.value) return true;
        return false;
    }

    public:
    Parser(std::vector<Token> tokens_) : tokens(tokens_) {
        advance();
    }

    NodeProg parse() {
        if (!isTok(toktype::keyword, "ZetriScript")) {
            std::variant<Position, specialpos> pos = specialpos::ENTRY;
            Error error(pos, errortype::syntax, "EXPECTED 'ZetriScript' keyword at main enterance");

        }
    }

}
