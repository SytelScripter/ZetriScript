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

struct NodeLine {
    Position pos1;
    Position pos2;
};

struct NodeSegment {
    Position pos;
    std::variant<NodeAlloc, NodeVarAssign, NodeLine, NodeIdentifier, NodeNumber> content;
};

struct NodeProg {
    Token header;
    std::string startingPosition;
    std::vector<NodeLine> code;
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

    inline Token look_forward(int j) {
        return tokens[idx + j];
    }

    inline bool isTok(toktype type_, std::string value_) {
        if (type_ == currentToken.type && value_ == currentToken.value) return true;
        return false;
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

    NodeLine parseLine(std::variant<Position, specialpos> pos) {
        if (!isTok(toktype::keyword, "LINE")) {
            Error error(pos, errortype::syntax, "EXPECTED KEYWORD 'LINE'");
        }
        advance();
        if (currentToken.type != toktype::left_paren) {
            Error error(pos, errortype::syntax, "EXPECTED '('");
        }
        advance();
        Position pos1(parsePos(pos));
        advance();
        if (currentToken.type != toktype::comma) {
            Error error(pos, errortype::syntax, "EXPECTED ','");
        }
        advance();
        Position pos2(parsePos(pos));
        advance();
        if (currentToken.type != toktype::right_paren) {
            Error error(pos, errortype::syntax, "EXPECTED ')'");
        }
        advance();
        if (currentToken.type != toktype::semicolon) {
            Error error(pos, errortype::syntax, "EXPECTED ';'");
        }
        advance();
        NodeLine result;
        result.pos1 = std::move(pos1);
        result.pos2 = std::move(pos2);
        return result;
    }

    NodeAlloc parseAlloc(std::variant<Position, specialpos> pos) {
        if (!isTok(toktype::keyword, "allocSpace")) {
            Error error(pos, errortype::syntax, "EXPECTED KEYWORD 'allocSpace'");
        }
        advance();
        if (currentToken.type != toktype::left_paren) {
            Error error(pos, errortype::syntax, "EXPECTED '('");
        }
        advance();
        if (currentToken.type != toktype::name) {
            Error error(pos, errortype::syntax, "EXPECTED IDENTIFIER");
        }
        NodeIdentifier allocated;
        allocated.identName = currentToken;
        advance();
        if (currentToken.type != toktype::right_paren) {
            Error error(pos, errortype::syntax, "EXPECTED ')'");
        }
        advance();
        if (currentToken.type != toktype::semicolon) {
            Error error(pos, errortype::syntax, "EXPECTED ';'");
        }
        advance();
        NodeAlloc result;
        result.allocated = std::move(allocated);
        return result;
    }

    NodePosition parsePos(std::variant<Position, specialpos> pos) {
        if (currentToken.type != toktype::left_square) {
            Error error(pos, errortype::syntax, "EXPECTED '['");
        }
        advance();
        NodeNumber posX = parseNumber(pos);
        advance();
        if (currentToken.type != toktype::colon) {
            Error error(pos, errortype::syntax, "EXPECTED ':'");
        }
        advance();
        NodeNumber posY = parseNumber(pos);
        advance();
        if (currentToken.type != toktype::colon) {
            Error error(pos, errortype::syntax, "EXPECTED ':'");
        }
        advance();
        NodeNumber posZ = parseNumber(pos);
        advance();
        if (currentToken.type != toktype::right_square) {
            Error error(pos, errortype::syntax, "EXPECTED ']'");
        }
        advance();
        if (currentToken.type != toktype::colon) {
            Error error(pos, errortype::syntax, "EXPECTED ':'");
        }
        advance();
        NodePosition result;
        result.x = std::move(posX);
        result.y = std::move(posY);
        result.z = std::move(posZ);

        return result;
    }

    NodeSegment parseSegment(std::variant<Position, specialpos> pos) {
        NodeSegment result;
        std::variant<Position, specialpos> pos_advance = specialpos::POS_DECL;
        std::variant<Position, specialpos> position_code = parsePos(pos_advance);
        std::variant<NodeAlloc, NodeVarAssign, NodeLine, NodeIdentifier, NodeNumber> content;
        if (isTok(toktype::keyword, "allocSpace")) {
            content = parseAlloc(position_code);
        }
        else if (look_forward(1).type == toktype::equals) {
            content = parseVarAssign(position_code);
        }
        else if (isTok(toktype::keyword, "LINE")) {
            content = parseLine(position_code);
        }

        result.pos = std::move(position_code);
        result.content = std::move(content);
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

        std::variant<Position, specialpos> pos_advance = specialpos::POS_DECL;
        advance();
        std::variant<NodeAlloc, NodeVarAssign, NodeLine, NodeIdentifier, NodeNumber> content = parseSegment(pos_advance);
    }

}
