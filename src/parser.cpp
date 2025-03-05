#include "lexer.cpp"
#include "error.cpp"
#include <memory>
#include <variant>

struct NodeNumber {
    Position pos;
    Token numTok;
}

struct NodePosition {
    NodeNumber x;
    NodeNumber y;
    NodeNumber z;
};

struct NodeLine {
    Position pos;
    NodePosition startPoint;
    NodePosition Vector3;
};

struct NodeIdentifier {
    Position pos;
    Token identName;
};

struct NodeVarAssign {
    Position pos;
    NodeIdentifier ident;
    std::variant<NodeLine, NodeIdentifier, NodeAlloc> value;
};

struct NodeAlloc {
    Position pos;
    NodeIdentifier allocated;
};

struct NodeLine {
    Position pos
    NodePosition pos1;
    NodePosition pos2;
};

struct NodeGoto {
    Position pos;
    NodePosition nextPos;
}

struct NodeSegment {
    Position pos;
    std::variant<NodeAlloc, NodeVarAssign, NodeLine, NodeIdentifier, NodeNumber, NodeGoto> content;
};

struct NodeExec {
    Position pos;
    NodeIdentifier identName;
}

struct NodeProg {
    NodePosition startingPosition;
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

    NodeVarAssign parseVarAssign(std::variant<Position, specialpos> pos) {
        if (currentToken.type != toktype::name) {
            Error error(pos, errortype::syntax, "EXPECTED IDENTIFIER");
        }
        NodeIdentifier identifier;
        identifier.identName = currentToken;
        advance();
        if (currentToken.type != toktype::equals) {
            Error error(pos, errortype::syntax, "EXPECTED '='");
        }
        advance();
        std::variant<NodeLine, NodeIdentifier, NodeAlloc> value;
        if (isTok(toktype::keyword, "allocSpace")) {
            value = parseAlloc(position_code);
        }
        else if (isTok(toktype::keyword, "LINE")) {
            value = parseLine(position_code);
        }
        NodeVarAssign result;
        result.ident = identifier;
        result.value = value;
        return result;
    }

    NodeNumber parseNumber(std::variant<Position, specialpos> pos) {
        if (currentToken.type != toktype::int_lit && currentToken.type != toktype::float_lit) {
            Error error(pos, errortype::syntax, "EXPECTED '['");
        }
        Token numTok(currentToken.type, currentToken.value);
        advance();
        NodeNumber result;
        result.pos = pos;
        result.numTok = std::move(numTok);
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
        NodePosition pos1(parsePos(pos));
        advance();
        if (currentToken.type != toktype::comma) {
            Error error(pos, errortype::syntax, "EXPECTED ','");
        }
        advance();
        NodePosition pos2(parsePos(pos));
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
        result.pos = pos;
        result.pos1 = std::move(pos1);
        result.pos2 = std::move(pos2);
        return result;
    }

    NodeExec parseExecution(std::variant<Position, specialpos> pos) {
        if (currentToken.type != toktype::name) {
            Error error(pos, errortype::syntax, "EXPECTED IDENTIFIER BEFORE CALL");
        }
        NodeIdentifier ident;
        ident.identName = currentToken;
        advance();
        if (currentToken.type != toktype::exc_mark) {
            Error error(pos, errotype::syntax, "EXPECTED '!'");
        }
        advance();
        NodeExec result;
        result.identName = ident;
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
        result.pos = pos;
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

    NodeGoto parseGoto(std::variant<Position, specialpos> pos) {
        if (!isTok(toktype::keyword, "goto")) {
            Error error(pos, errortype::syntax, "EXPECTED 'goto'");
        }
        advance();
        NodePosition nextPos = parsePos(pos);
        advance();
        if (currentToken.type != toktype::exc_mark) {
            Error error(pos, errortype::syntax, "'goto' INSTRUCTION IS EXECUTABLE");
        }
        advance();
        NodeGoto result;
        result.pos = pos;
        result.nextPos = std::move(nextPos);
        return result;
    }

    NodeSegment parseSegment(std::variant<Position, specialpos> pos) {
        NodeSegment result;
        std::variant<Position, specialpos> pos_advance = specialpos::POS_DECL;
        std::variant<Position, specialpos> position_code = parsePos(pos_advance);
        std::variant<NodeAlloc, NodeVarAssign, NodeLine, NodeIdentifier, NodeNumber, NodeGoto> content;
        if (isTok(toktype::keyword, "allocSpace")) {
            content = parseAlloc(position_code);
        }
        else if (look_forward(1).type == toktype::equals) {
            content = parseVarAssign(position_code);
        }
        else if (isTok(toktype::keyword, "LINE")) {
            content = parseLine(position_code);
        }
        else if (look_forward(1).type == toktype::exc_mark) {
            content = parseExecution(position_code);
        }
        else if (isTok(toktype::keyword, "goto")) {
            content = parseGoto(position_code);
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
        std::variant<Position, specialpos> pos_advance = specialpos::POS_DECL;
        NodePosition startingPosition = parsePos(pos_advance);
        std::vector<NodeSegment> code;

        while (!isTok(toktype::keyword, "ZetriScript")) {
            advance();
            NodeSegment content = std::move(parseSegment(pos_advance));
            code.push_back(std::move(content));
        }
        NodeProg result;
        result.startingPosition = startingPosition;
        result.code = code;
        return result;
    }

}
