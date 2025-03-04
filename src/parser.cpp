#include "lexer.cpp"
#include "error.cpp"
#include <memory>
#include <variant>

struct NodeNumber {
    Token numTok;
}

struct NodePosition {
    NodeNumber x;
    NodeNumber y;
    NodeNumber z;
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

    NodeNumber parseNumber() {

    }

    NodePosition parsePos() {
        if (currentToken.type != '[') {
            std::variant<Position, specialpos> pos = specialpos::ENTRY; // temporary, it's Position class.
            Error error(pos, errortype::syntax, "EXPECTED '['");
        }
        advance();
        NodeNumber posX = parseNumber();
        advance();
        if (currentToken.type != ':') {
            std::variant<Position, specialpos> pos = specialpos::ENTRY; // temporary, it's Position class.
            Error error(pos, errortype::syntax, "EXPECTED ':'");
        }
        advance();
        NodeNumber posY = parseNumber();
        advance();
        if (currentToken.type != ':') {
            std::variant<Position, specialpos> pos = specialpos::ENTRY; // temporary, it's Position class.
            Error error(pos, errortype::syntax, "EXPECTED ':'");
        }
        advance();
        NodeNumber posZ = parseNumber();
        advance();
        if (currentToken.type != ']') {
            std::variant<Position, specialpos> pos = specialpos::ENTRY; // temporary, it's Position class.
            Error error(pos, errortype::syntax, "EXPECTED ']'");
        }
        advance();
        NodePosition result();
        result.x = posX;
        result.y = posY;
        result.z = posZ;

        return result;
    }

    NodeProg parse() {
        if (!isTok(toktype::keyword, "ZetriScript")) {
            std::variant<Position, specialpos> pos = specialpos::ENTRY;
            Error error(pos, errortype::syntax, "EXPECTED 'ZetriScript' KEYWORD AT THE MAIN ENTERANCE");
        }
        advance();
        if (currentToken.type != toktype::exc_mark) {
            std::variant<Position, specialpos> pos = specialpos::ENTRY;
            Error error(pos, errortype::syntax, "EXPECTED ENTRY CALL FOR EXECUTION");
        }
        advance();

        
    }

}
