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

    inline NodeProg gotoPos(Position pos) {
        // looking forward without parsing.
        int tempIdx, oldIdx = idx;
        while (tempIdx < tokens.size()) {
            std::variant<Position, specialpos> advance_pos = specialpos::POS_DECL;
            NodePosition currentPos = parsePos(advance_pos);
            if (currentPos.posX == pos.posX && currentPos.posY == pos.posY && currentPos.posZ == pos.posZ) {
                break;
            }
            tempIdx++;
        }
        std::variant<Position, specialpos> posTemp = currentPos;
        idx = tempIdx;
        NodeProg parseResult = parse(posTemp);
        idx = oldIdx;
        return parseResult;
    }

    public:
    Parser(std::vector<Token> tokens_) : tokens(tokens_) {
        advance();
    }

    NodeNumber parseNumber(std::variant<Position, specialpos> pos) {
        if (currentToken.type != toktype::int_lit && currentToken.type != toktype::float_lit) {
            Error error(pos, errortype::syntax, "EXPECTED '['");
        }
        Token numTok(currentToken.type, currentToken.value);
        advance();
        NodeNumber result;
        result.numTok = numTok;
        return result;
    }

    NodePosition parsePos(std::variant<Position, specialpos> pos) {
        if (currentToken.type != '[') {
            Error error(pos, errortype::syntax, "EXPECTED '['");
        }
        advance();
        NodeNumber posX = parseNumber(pos);
        advance();
        if (currentToken.type != ':') {
            Error error(pos, errortype::syntax, "EXPECTED ':'");
        }
        advance();
        NodeNumber posY = parseNumber(pos);
        advance();
        if (currentToken.type != ':') {
            Error error(pos, errortype::syntax, "EXPECTED ':'");
        }
        advance();
        NodeNumber posZ = parseNumber(pos);
        advance();
        if (currentToken.type != ']') {
            Error error(pos, errortype::syntax, "EXPECTED ']'");
        }
        advance();
        if (currentToken.type != ':') {
            Error error(pos, errortype::syntax, "EXPECTED ':'");
        }
        advance();
        NodePosition result;
        result.x = posX;
        result.y = posY;
        result.z = posZ;

        return result;
    }

    NodeProg parse(std::variant<Position, specialpos> pos) {
        if (!isTok(toktype::keyword, "ZetriScript")) {
            Error error(pos, errortype::syntax, "EXPECTED 'ZetriScript' KEYWORD AT THE MAIN ENTERANCE");
        }
        advance();
        if (currentToken.type != toktype::exc_mark) {
            Error error(pos, errortype::syntax, "EXPECTED ENTRY CALL FOR EXECUTION");
        }
        advance();
        NodePosition startingPosition;

        std::variant<Position, specialpos> advance_pos = specialpos::POS_DECL;
        startingPosition = parsePos(advance_pos);
        
    }

}
