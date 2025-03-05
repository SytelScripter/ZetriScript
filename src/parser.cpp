#include "lexer.cpp"
#include "error.cpp"
#include <memory>
#include <optional>
#include <variant>

class NodeNumber {
    public:
    Position pos;
    Token_ numTok;

    NodeNumber() {}
    NodeNumber(Position pos_, Token_ numTok_) : pos(pos_), numTok(numTok_) {}
};

class NodePositionAccess {
    public:
    NodeNumber x;
    NodeNumber y;
    NodeNumber z;

    NodePositionAccess() {}
    NodePositionAccess(NodeNumber x_, NodeNumber y_, NodeNumber z_) : x(x_), y(y_), z(z_) {}
};

class NodePositionAssign {
    public:
    NodeNumber x;
    NodeNumber y;
    NodeNumber z;

    NodePositionAssign() {}
    NodePositionAssign(NodeNumber x_, NodeNumber y_, NodeNumber z_) : x(x_), y(y_), z(z_) {}
};

class NodeVarAccess {
    public:
    Position pos;
    Token_ identName;

    NodeVarAccess() {}
    NodeVarAccess(Position pos_, Token_ identName_) : pos(pos_), identName(identName_) {}
};

class NodeVarAssign {
    public:
    Position pos;
    NodeVarAccess ident;
    std::variant<NodeLine, NodeVarAccess, NodeAlloc> value;

    NodeVarAssign() {}
    NodeVarAssign(Position pos_, NodeVarAccess ident_, std::variant<NodeLine, NodeVarAccess, NodeAlloc> value_) : pos(pos_), ident(ident_), value(value_) {}
};

class NodeAlloc {
    public:
    Position pos;
    NodeVarAccess allocated;

    NodeGoto() {}
    NodeAlloc(Position pos_, NodeVarAccess allocated_) : pos(pos_), allocated(allocated_) {}
};

class NodeLine {
    public:
    Position pos;
    NodePositionAccess pos1;
    NodePositionAccess pos2;
    NodeNumber start;
    NodeNumber end;
    NodeNumber step;

    NodeLine() {}
    NodeLine(Position pos_, NodePositionAccess pos1_, NodePositionAccess pos2_, NodeNumber start_, NodeNumber end_, NodeNumber step_) : pos(pos_), pos1(pos1_), pos2(pos2_), start(start_), end(end_), step(step_) {}
};

class NodeGoto {
    public:
    Position pos;
    NodePositionAccess nextPos;

    NodeGoto() {}
    NodeGoto(Position pos_, NodePositionAccess nextPos_) : pos(pos_), nextPos(nextPos_) {}
};

class NodeSegment {
    public:
    Position pos;
    std::vector<std::variant<NodeAlloc, NodeVarAssign, NodeLine, NodeVarAccess, NodeNumber, NodeGoto>> content;

    NodeSegment() {}
    NodeSegment(Position pos_, std::vector<std::variant<NodeAlloc, NodeVarAssign, NodeLine, NodeVarAccess, NodeNumber, NodeGoto>> content_) : pos(pos_), content(content_) {}
};

class NodeExec {
    public:
    Position pos;
    NodeVarAccess identName;

    NodeExec() {}
    NodeExec(Position pos_, NodeVarAccess identName_) : pos(pos_), identName(identName_) {}
};

class NodeProg {
    public:
    NodePositionAccess startingPosition;
    std::vector<NodeSegment> code;

    NodeProg() {}
    NodeProg(NodePositionAccess startingPosition_, std::vector<NodeSegment> code_) : startingPosition(startingPosition_), code(code_) {}
};

using anynode = std::variant<NodeProg, NodeSegment, NodeExec, NodeLine, NodeAlloc, NodeVarAssign, NodeVarAccess, NodePositionAsign, NodePositionAccess, NodeNumber, NodeGoto>;

class ParseResult {
    public:
    anynode nodeResult;
    Error error;

    ParseResult() {}

    template<typename T>
    ParseResult(T node) {
        nodeResult = node;
    }

    inline bool hasError() {
        return !error.empty();
    }

    inline auto getValue() {
        if (std::holds_alternative<NodeProg>(nodeResult))             return std::get<NodeProg>(nodeResult);
        if (std::holds_alternative<NodeSegment>(nodeResult))          return std::get<NodeSegment>(nodeResult);
        if (std::holds_alternative<NodeExec>(nodeResult))             return std::get<NodeExec>(nodeResult);
        if (std::holds_alternative<NodeLine>(nodeResult))             return std::get<NodeLine>(nodeResult);
        if (std::holds_alternative<NodeAlloc>(nodeResult))            return std::get<NodeAlloc>(nodeResult);
        if (std::holds_alternative<NodeVarAssign>(nodeResult))        return std::get<NodeVarAssign>(nodeResult);
        if (std::holds_alternative<NodeVarAccess>(nodeResult))       return std::get<NodeVarAccess>(nodeResult);
        if (std::holds_alternative<NodePositionAsign>(nodeResult))    return std::get<NodePositionAsign>(nodeResult);
        if (std::holds_alternative<NodePositionAccess>(nodeResult))   return std::get<NodePositionAccess>(nodeResult);
        if (std::holds_alternative<NodeNumber>(nodeResult))           return std::get<NodeNumber>(nodeResult);
        if (std::holds_alternative<NodeGoto>(nodeResult))             return std::get<NodeGoto>(nodeResult);
    }

    inline void setError(Error error_) {
        error = std::move(error_);
    }

    inline void setErrorDirectly(std::variant<Position, specialpos> pos, errortype errorType, std::string description) {
        error = Error(pos, errorType, description);
    }

    template<typename T>
    inline void setNode(T node) {
        nodeResult = node;
    }

    template<typename T>
    inline T getKnownNode() {
        return std::get<T>(nodeResult);
    }
};

class Parser {
    public:
    int idx = -1;
    Token_ currentToken;
    std::vector<Token_> tokens;

    inline void advance() {
        idx++;
        if (idx < tokens.size()) {
            currentToken = tokens[idx];
        }
    }

    inline Token_ look_forward(int j) {
        return tokens[idx + j];
    }

    inline bool isTok(toktype type_, std::string value_) {
        if (type_ == currentToken.type && value_ == currentToken.value) return true;
        return false;
    }

    inline std::optional<Error> checkSyntaxError(toktype tokenType, std::string msg) {
        if (currentToken.type != tokenType) {
            Error error = Error(pos, errortype::syntax, std::string("EXPECTED ") + msg);
            return error;
        }
        advance();
        return std::nullopt;
    }

    
    Parser(std::vector<Token_> tokens_) : tokens(tokens_) {
        advance();
    }

    ParseResult parseVarAssign(std::variant<Position, specialpos> pos) {
        ParseResult parse_result = ParseResult();
        std::optional<Error> temp = checkSyntaxError(toktype::name, "IDENTIFIER");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeVarAccess identifier;
        identifier.identName = currentToken;
        advance();

        temp = checkSyntaxError(toktype::equals, "'='");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        std::variant<NodeLine, NodeVarAccess, NodeAlloc, NodeGoto> value;
        
        ParseResult value_result = ParseResult();
        if (isTok(toktype::keyword, "allocSpace")) {
            value_result = parseAlloc(position_code);
            if (value_result.hasError()) return value_result;
        }
        else if (isTok(toktype::keyword, "LINE")) {
            value_result = parseLine(position_code);
            if (value_result.hasError()) return value_result;
        }
        value = value_result.getValue();

        NodeVarAssign result;
        result.pos = std::get<Position>(pos);
        result.ident = std::move(identifier);
        result.value = std::move(value);

        parse_result.setNode(std::move(result));
        return parse_result;
    }

    ParseResult parseNumber(std::variant<Position, specialpos> pos) {
        ParseResult parse_result = ParseResult();
        if (currentToken.type != toktype::int_lit && currentToken.type != toktype::float_lit) {
            parse_result.setErrorDirectly(pos, errortype::syntax, "EXPECTED '['");
            return parse_result;
        }
        Token_ numTok(currentToken.type, currentToken.value);
        advance();
        NodeNumber result;
        result.pos = std::get<Position>(pos);
        result.numTok = std::move(numTok);
        
        parse_result.setNode(std::move(result));
        return parse_result;
    }

    ParseResult parseLine(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();
        if (!isTok(toktype::keyword, "LINE")) {
            parse_result.setErrorDirectly(pos, errortype::syntax, "EXPECTED KEYWORD 'LINE'");
            return parse_result;
        }
        advance();
        temp = checkSyntaxError(toktype::left_paren, "'('");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }
        ParseResult temp_result = parseAccessPos(pos);
        if (temp_result.hasError()) return temp_result;
        NodePositionAccess pos1;
        pos1 = std::move(temp_result.getValue());
        advance();

        temp = checkSyntaxError(toktype::comma, "','");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }
        ParseResult temp_result1 = parseAccessPos(pos);
        if (temp_result1.hasError()) return temp_result1;
        NodePositionAccess pos2;
        pos2 = std::move(temp_result1.getValue());
        advance();

        temp = checkSyntaxError(toktype::comma, "','");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeNumber start, end, step;
        
        ParseResult temp_result2 = parseNumber(pos);
        if (temp_result2.hasError()) return temp_result2;
        start = temp_result2.getValue();

        ParseResult temp_result3 = parseNumber(pos);
        if (temp_result3.hasError()) return temp_result3;
        end = temp_result3.getValue();

        ParseResult temp_result4 = parseNumber(pos);
        if (temp_result4.hasError()) return temp_result4;
        step = temp_result4.getValue();

        temp = checkSyntaxError(toktype::right_paren, "')'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        temp = checkSyntaxError(toktype::semicolon, "';'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeLine result;
        result.pos = std::get<Position>(pos);
        result.pos1 = std::move(pos1);
        result.pos2 = std::move(pos2);
        result.start = std::move(start);
        result.end = std::move(end);
        result.step = std::move(step);
        parse_result.setNode(std::move(result));
        return parse_result;
    }

    ParseResult parseExecution(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();

        if (currentToken.type != toktype::name) {
            parse_result.setErrorDirectly(pos, errortype::syntax, "EXPECTED IDENTIFIER BEFORE CALL");
            return parse_result;
        }
        NodeVarAccess ident;
        ident.identName = currentToken;
        advance();

        temp = checkSyntaxError(toktype::exc_mark, "'!'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeExec result;
        result.identName = ident;
        parse_result.setNode(std::move(result));
        return parse_result;
    }

    ParseResult parseAlloc(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();

        if (!isTok(toktype::keyword, "allocSpace")) {
            parse_result.setErrorDirectly(pos, errortype::syntax, "EXPECTED KEYWORD 'allocSpace'");
            return parse_result;
        }
        advance();

        temp = checkSyntaxError(toktype::left_paren, "'('");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        if (currentToken.type != toktype::name) {
            parse_result.setErrorDirectly(pos, errortype::syntax, "EXPECTED IDENTIFIER");
            return parse_result;
        }
        NodeVarAccess allocated(std::get<Position>(pos), currentToken);
        advance();

        temp = checkSyntaxError(toktype::right_paren, "')'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        temp = checkSyntaxError(toktype::semicolon, "';'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeAlloc result(std::get<Position>(pos), allocated);
        
        parse_result.setNode(std::move(result));
        return parse_result;
    }

    ParseResult parseAccessPos(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();

        temp = checkSyntaxError(toktype::left_square, "'['");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        ParseResult result_x = parseNumber(pos);
        if (result_x.hasError()) return result_x;
        NodeNumber posX = result_x.getValue();
        advance();

        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        ParseResult result_y = parseNumber(pos);
        if (result_y.hasError()) return result_y;
        NodeNumber posY = result_y.getValue();
        advance();
        
        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        ParseResult result_z = parseNumber(pos);
        if (result_z.hasError()) return result_z;
        NodeNumber posZ = result_z.getValue();
        advance();

        temp = checkSyntaxError(toktype::right_square, "']'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodePositionAccess result;
        result.x = std::move(posX);
        result.y = std::move(posY);
        result.z = std::move(posZ);

        parse_result.setNode(std::move(result));
        return parse_result;
    }
    
    ParseResult parseAssignPos(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();

        temp = checkSyntaxError(toktype::left_square, "'['");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        ParseResult result_x = parseNumber(pos);
        if (result_x.hasError()) return result_x;
        NodeNumber posX = result_x.getValue();
        advance();

        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        ParseResult result_y = parseNumber(pos);
        if (result_y.hasError()) return result_y;
        NodeNumber posY = result_y.getValue();
        advance();
        
        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        ParseResult result_z = parseNumber(pos);
        if (result_z.hasError()) return result_z;
        NodeNumber posZ = result_z.getValue();
        advance();

        temp = checkSyntaxError(toktype::right_square, "']'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodePositionAssign result;
        result.x = std::move(posX);
        result.y = std::move(posY);
        result.z = std::move(posZ);

        parse_result.setNode(std::move(result));
        return parse_result;
    }

    ParseResult parseGoto(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();

        if (!isTok(toktype::keyword, "goto")) {
            parse_result.setErrorDirectly(pos, errortype::syntax, "EXPECTED 'goto'");
            return parse_result;
        }
        advance();
        ParseResult pos_result = parseAccessPos(pos);
        if (pos_result.hasError()) return pos_result;
        advance();
        NodePositionAccess nextPos = pos_result.getKnownNode<NodePositionAccess>();

        temp = checkSyntaxError(toktype::exc_mark, "'!'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeGoto result;
        result.pos = std::get<Position>(pos);
        result.nextPos = std::move(nextPos);

        parse_result.setNode(std::move(result));
        return parse_result;
    }

    ParseResult parseSegment(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();
        NodeSegment result;
        
        std::variant<Position, specialpos> pos_advance = specialpos::POS_DECL;
        ParseResult pos_result = parseAssignPos(pos_advance);
        if (pos_result.hasError()) return pos_result;

        NodePositionAccess segment_position = pos_result.getValue();
        std::variant<Position, specialpos> position_code = Position(segment_position.x.numTok.value, segment_position.y.numTok.value, segment_position.z.numTok.value);
        
        result.pos = std::move(std::get<Position>(position_code));

        while (currentToken.type != toktype::exc_mark) {
            ParseResult content = ParseResult();
            if (isTok(toktype::keyword, "allocSpace")) {
                content = parseAlloc(position_code);
                if (content.hasError()) return content;
            }
            else if (look_forward(1).type == toktype::equals) {
                content = parseVarAssign(position_code);
                if (content.hasError()) return content;
            }
            else if (isTok(toktype::keyword, "LINE")) {
                content = parseLine(position_code);
                if (content.hasError()) return content;
            }
            else if (look_forward(1).type == toktype::exc_mark) {
                content = parseExecution(position_code);
                if (content.hasError()) return content;
            }
            else if (isTok(toktype::keyword, "goto")) {
                content = parseGoto(position_code);
                if (content.hasError()) return content;
            }

            temp = checkSyntaxError(toktype::semicolon, "';'");
            if (temp.has_value()) {
                parse_result.setError(temp.value());
                return parse_result;
            }

            if (idx >= tokens.size() || isTok(toktype::keyword, "ZetriScript")) {
                parse_result.setErrorDirectly(pos, errortype::syntax, "'!' AT THE END OF THE INSTRUCTION");
                return parse_result;
            }

            result.content.push_back(std::move(content.getValue()));
        }

        parse_result.setNode(std::move(result));
        return parse_result;
    }

    ParseResult parse(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();
        

        if (!isTok(toktype::keyword, "ZetriScript")) {
            parse_result.setErrorDirectly(pos, errortype::syntax, "EXPECTED 'ZetriScript' KEYWORD AT THE MAIN ENTERANCE");
            return parse_result;
        }
        advance();
        temp = checkSyntaxError(toktype::exc_mark, "ENTRY CALL FOR EXECUTION '!'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        std::variant<Position, specialpos> pos_advance = specialpos::POS_DECL;
        ParseResult accessResult = parseAccessPos(pos_advance);
        if (parse_result.hasError()) return parse_result;
        NodePositionAccess startingPosition = parse_result.getKnownNode<NodePositionAccess>();

        std::vector<NodeSegment> code;
        while (!isTok(toktype::keyword, "ZetriScript")) {
            advance();
            ParseResult segment_result = parseSegment(pos_advance);
            if (segment_result.hasError()) return segment_result;
            code.push_back(segment_result.getKnownNode<NodeSegment>());
            if (idx >= tokens.size()) {
                parse_result.setErrorDirectly(pos_advance, errortype::syntax, "'ZetriScript' KEYWORD AT THE END OF THE FILE");
                return parse_result;
            }
        }
        
        NodeProg nodeProgInstance = NodeProg(std::move(startingPosition), std::move(code));
        parse_result.setNode(std::move(nodeProgInstance));
        return parse_result;
    }

};
