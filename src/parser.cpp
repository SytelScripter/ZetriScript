#include "lexer.cpp"
#include "error.cpp"
#include <memory>
#include <variant>

struct NodeNumber {
    Position pos;
    Token_ numTok;
};

struct NodePositionAccess {
    NodeNumber x;
    NodeNumber y;
    NodeNumber z;
};

struct NodePositionAssign {
    NodeNumber x;
    NodeNumber y;
    NodeNumber z;
};

struct NodeVarAccess {
    Position pos;
    Token_ identName;
};

struct NodeVarAssign {
    Position pos;
    NodeVarAccess ident;
    std::variant<NodeLine, NodeVarAccess, NodeAlloc> value;
};

struct NodeAlloc {
    Position pos;
    NodeVarAccess allocated;
};

struct NodeLine {
    Position pos;
    NodePositionAccess pos1;
    NodePositionAccess pos2;
    NodeNumber start;
    NodeNumber end;
    NodeNumber step;
};

struct NodeGoto {
    Position pos;
    NodePositionAccess nextPos;
};

struct NodeSegment {
    Position pos;
    std::vector<std::variant<NodeAlloc, NodeVarAssign, NodeLine, NodeVarAccess, NodeNumber, NodeGoto>> content;
};

struct NodeExec {
    Position pos;
    NodeVarAccess identName;
};

struct NodeProg {
    NodePositionAccess startingPosition;
    std::vector<NodeSegment> code;
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
    inline T getKnownNode() {
        return std::get<T>(nodeResult);
    }
    
    inline void checkError(ParseResult* result) {
        if (result->hasError()) {
            error = std::move(result->error);
        }
        nodeResult = std::move(result->getValue());
    }
};

class Parser {
    private:
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
            parse_result.setError(error);
            return parse_result;
        }
        advance();
        return std::nullopt;
    }

    public:
    Parser(std::vector<Token_> tokens_) : tokens(tokens_) {
        advance();
    }

    ParseResult parseVarAssign(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();
        temp = checkSyntaxError(toktype::name, "IDENTIFIER");
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
            value_result.checkError(&parseAlloc(position_code));
            if (value_result.hasError()) return value_result;
        }
        else if (isTok(toktype::keyword, "LINE")) {
            value_result.checkError(&parseLine(position_code));
            if (value_result.hasError()) return value_result;
        }
        value = value_result.getValue();

        NodeVarAssign result;
        result.pos = pos;
        result.ident = std::move(identifier);
        result.value = std::move(value);

        parse_result.nodeResult = std::move(result);
        return parse_result;
    }

    ParseResult parseNumber(std::variant<Position, specialpos> pos) {
        ParseResult parse_result = ParseResult();
        if (currentToken.type != toktype::int_lit && currentToken.type != toktype::float_lit) {
            Error error = Error(pos, errortype::syntax, "EXPECTED '['");
            parse_result.setError(error);
            return parse_result;
        }
        Token_ numTok(currentToken.type, currentToken.value);
        advance();
        NodeNumber result;
        result.pos = pos;
        result.numTok = std::move(numTok);
        
        parse_result.nodeResult = std::move(result);
        return parse_result;
    }

    ParseResult parseLine(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();
        if (!isTok(toktype::keyword, "LINE")) {
            Error error = Error(pos, errortype::syntax, "EXPECTED KEYWORD 'LINE'");
            parse_result.setError(error);
            return parse_result;
        }
        advance();
        temp = checkSyntaxError(toktype::left_paren, "'('");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }
        ParseResult temp_result = ParseResult();
        temp_result.checkError(parseAccessPos(pos));
        if (temp_result.hasError()) return temp_result;
        NodePositionAccess pos1;
        pos1 = std::move(temp_result.getValue());
        advance();

        temp = checkSyntaxError(toktype::comma, "','");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }
        ParseResult temp_result1 = ParseResult();
        temp_result1.checkError(parseAccessPos(pos));
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
        
        ParseResult temp_result2 = ParseResult();
        temp_result2.checkError(parseNumber(pos));
        if (temp_result2.hasError()) return temp_result2;
        start = temp_result2.getValue();

        ParseResult temp_result3 = ParseResult();
        temp_result3.checkError(parseNumber(pos));
        if (temp_result3.hasError()) return temp_result3;
        end = temp_result3.getValue();

        ParseResult temp_result4 = ParseResult();
        temp_result4.checkError(parseNumber(pos));
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
        result.pos = pos;
        result.pos1 = std::move(pos1);
        result.pos2 = std::move(pos2);
        result.start = std::move(start);
        result.end = std::move(end);
        result.step = std::move(step);
        parse_result.nodeResult = std::move(result);
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
        parse_result.nodeResult = std::move(result);
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
        NodeVarAccess allocated;
        allocated.identName = currentToken;
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

        NodeAlloc result;
        result.pos = pos;
        result.allocated = std::move(allocated);
        
        parse_result.nodeResult = std::move(result);
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

        NodeNumber posX = parseNumber(pos);
        advance();

        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeNumber posY = parseNumber(pos);
        advance();
        
        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeNumber posZ = parseNumber(pos);
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

        parse_result.nodeResult = std::move(result);
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

        NodeNumber posX = parseNumber(pos);
        advance();

        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeNumber posY = parseNumber(pos);
        advance();
        
        temp = checkSyntaxError(toktype::colon, "':'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeNumber posZ = parseNumber(pos);
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
        ParseResult pos_result = ParseResult();
        pos_result.checkError(parseAccessPos(pos));
        if (pos_result.hasError()) return pos_result;
        advance();

        temp = checkSyntaxError(toktype::exc_mark, "'!'");
        if (temp.has_value()) {
            parse_result.setError(temp.value());
            return parse_result;
        }

        NodeGoto result;
        result.pos = pos;
        result.nextPos = std::move(nextPos);

        parse_result.nodeResult = std::move(result);
        return parse_result;
    }

    ParseResult parseSegment(std::variant<Position, specialpos> pos) {
        std::optional<Error> temp;
        ParseResult parse_result = ParseResult();
        NodeSegment result;
        
        std::variant<Position, specialpos> pos_advance = specialpos::POS_DECL;
        ParseResult pos_result = ParseResult()
        pos_result.checkError(parseAssignPos(pos_advance));
        if (pos_result.hasError()) return pos_result;

        NodePositionAccess segment_position = pos_result.getValue();
        std::variant<Position, specialpos> position_code = Position(segment_position.x.numTok.value, segment_position.y.numTok.value, segment_position.z.numTok.value);
        
        result.pos = std::move(std::get<Position>(position_code));

        while (currentToken.type != toktype::exc_mark) {
            ParseResult content = ParseResult();
            if (isTok(toktype::keyword, "allocSpace")) {
                content.checkError(&parseAlloc(position_code));
                if (content.hasError()) return content;
            }
            else if (look_forward(1).type == toktype::equals) {
                content.checkError(&parseVarAssign(position_code));
                if (content.hasError()) return content;
            }
            else if (isTok(toktype::keyword, "LINE")) {
                content.checkError(&parseLine(position_code));
                if (content.hasError()) return content;
            }
            else if (look_forward(1).type == toktype::exc_mark) {
                content.checkError(&parseExecution(position_code));
                if (content.hasError()) return content;
            }
            else if (isTok(toktype::keyword, "goto")) {
                content.checkError(&parseGoto(position_code));
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

        parse_result.nodeResult = std::move(result);
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
        parse_result.checkError(&parseAccessPos(pos_advance));
        if (parse_result.hasError()) return parse_result;
        NodePositionAccess startingPosition = parse_result.getKnownNode<NodePositionAccess>();

        std::vector<NodeSegment> code;
        while (!isTok(toktype::keyword, "ZetriScript")) {
            advance();
            ParseResult segment_result = ParseResult();
            segment_result.checkError(&parseSegment(pos_advance));
            if (segment_result.hasError()) return segment_result;
            code.push_back(std::move(segment_result.getKnownNode<NodeSegment>()));
            if (idx >= tokens.size()) {
                parse_result.setErrorDirectly(pos_advance, errortype::syntax, "'ZetriScript' KEYWORD AT THE END OF THE FILE");
                return parse_result;
            }
        }
        
        NodeProg nodeProgInstance = {.startingPosition = std::move(startingPosition), .code = std::move(code)};
        parse_result.nodeResult = std::move(nodeProgInstance);
        return parse_result;
    }

};
