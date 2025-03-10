#include <memory>
#include "lexer.cpp"
#include "error.cpp"
#include <functional>

namespace node {
    struct NodeNumber {};
    struct NodeBinOp {};
    struct NodeExec {};
    struct NodeClassBuiltIn {};
    struct NodeVarAssign {};
    struct NodeVarAccess {};
    struct NodePosAccess {};
    struct NodeStmt {};
    struct NodeProgram {};

    struct NodeBinOp {
        std::variant<std::unique_ptr<NodeNumber>, std::unique_ptr<NodeBinOp>> left_expr;
        Token op;
        std::variant<std::unique_ptr<NodeNumber>, std::unique_ptr<NodeBinOp>> right_expr;
    };

    struct NodeExec {
        std::variant<Token, std::unique_ptr<NodeClassBuiltIn>> executed;
    };

    struct NodeClassBuiltIn {
        Token class_name;
        std::vector<std::unique_ptr<ParsePosition>, std::unique_ptr<NodeBinOp>, std::unique_ptr<NodeNumber>> args;
    };

    struct NodeVarAssign {
        Token var_name_tok;
        std::variant<std::unique_ptr<NodeExec>, std::unique_ptr<NodeClassBuiltIn>, std::unique_ptr<NodeBinOp>, std::unique_ptr<NodeNumber>> value;
    };


    struct NodeStmt {
        ParsePosition pos;
        std::vector<std::variant<std::unique_ptr<NodeClassBUiltIn>, std::unique_ptr<NodeVarAssign>, std::unique_ptr<NodeExec>>> stmts;
    }

    struct NodeProgram {
        NodePosAccess pos_access;
        std::vector<std::unique_ptr<NodeStmt>> statements;
    };
}

using anyNode = std::variant<std::unique_ptr<node::NodeNumber>, std::unique_ptr<node::NodeBinOp>, std::unique_ptr<node::NodeGoto>, std::unique_ptr<node::NodeClassBuiltIn>, std::unique_ptr<node::NodeExec>, std::unique_ptr<node::NodeVarAssign>, std::unique_ptr<node::NodeVarAccess>, std::unique_ptr<node::NodePosAssign>, std::unique_ptr<node::NodePosAccess>, std::unique_ptr<node::NodeStmt>, std::unique_ptr<node::NodeProgram>>;


class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens) : 
        tokens_(std::move(tokens)) {}

    std::unique_ptr<NodeNumber> parse_factor() {
        Token value = tokens_[idx_++];
        return std::make_unique<node::NodeNumber>(value);
    }

    std::variant<std::unique_ptr<NodeNumber>, std::unique_ptr<NodeBinOp>> parse_term() {
        // create the term node
        std::unique_ptr<NodeBinOp> result;
        std::unique_ptr<NodeNumber> node = parse_factor();
        if (is_token_type(toktype::mul) || is_token_type(toktype::div)) {
            result->left_expr = node;
        }
        while (idx_ < tokens_.size() && (is_token_type(toktype::star) || is_token_type(toktype::slash))) {
            Token op = tokens_[idx_++];
            std::unique_ptr<NodeNumber> right_expr = parse_factor();
            result = std::make_unique<node::NodeBinOp>(std::move(result), op, std::move(right_expr));
        }
        return node;
    }

    std::variant<std::unique_ptr<NodeNumber>, std::unique_ptr<NodeBinOp>> parse_expr() {
        // create the binary operation node
        auto left_expr = parse_term();
        while (idx_ < tokens_.size() && is_token_type(toktype::plus) || is_token_type(toktype::minus)) {
            Token op = tokens_[idx_++];
            auto right_expr = parse_term();
            left_expr = std::make_unique<node::NodeBinOp>(std::move(left_expr), op, std::move(right_expr));
        }
        return left_expr;
    }

    std::unique_ptr<NodeExec> parse_exec() {
        // create the execution node
        // parsing expression
        if (is_token_type(toktype::name)) {
            Token name = tokens_[idx_++];
            error(toktype::exc_mark, "!", "Expected '!'");
            return std::make_unique<node::NodeExec>(name);
        }
        else if (is_token_type(toktype::keyword) && tokens_[idx_ + 1].type == toktype::left_paren) {
            std::unique_ptr<NodeClassBuiltIn> result = parse_class_builtin();
            error(toktype::exc_mark, "!", "Expected '!'");
            return std::make_unique<node::NodeExec>(result);
        }
        else if (is_token_type(toktype::keyword)) {
            Token goto_token = tokens_[idx_++];
            error(toktype::exc_mark, "!", "Expected '!'");
            return std::make_unique<node::NodeExec>(goto_token);
        }
        
        std::unique_ptr<NodeBinOp> expr = parse_expr();
        return std::make_unique<node::NodeExec>(expr);
    }

    std::unique_ptr<NodeVarAssign> parse_var_assign() {
        // create the variable assignment node
        // parsing variable name
        Token var_name = error_type(toktype::identifier, "Expected identifier", true);
        error(toktype::equal, "=", "Expected '='");
        // parsing expression
        std::unique_ptr<NodeBinOp> expr = parse_expr();
        return std::make_unique<node::NodeVarAssign>(var_name, expr);
    }

    std::unique_ptr<NodeClassBuiltIn> parse_class_builtin() {
        // create the class built-in node (calling built-in classes)
        // parsing class name
        Token class_name = error_type(toktype::keyword, "Expected keyword", true);
        error(toktype::left_paren, "(", "Expected '('");
        // parsing class arguments
        std::vector<ParsePosition, node::NodeExpr> args;
        while (idx_ < tokens_.size() && !is_token_type(toktype::right_paren)) {
            args.push_back(tokens_[idx_++].value);
            if (!is_token_type(toktype::comma))
                error(toktype::right_paren, ")", "Expected ',' or ')'"); // automatically skips comma
        }
        error(toktype::right_paren, ")", "Expected ')'");
        return std::make_unique<node::NodeClassBuiltIn>(class_name, args);
    }

    std::unique_ptr<NodeStmt> parse_statement() {
        // parsing statements are pretty simple in ZetriScript, they consist of position declaration and statement itself
        // create the statement node
        std::unique_ptr<node::NodeStmt> result = std::make_unique<node::NodeStmt>();
        // parsing position declaration
        current_pos_ = ParsePosition(specialpos::POS_DECL);
        visit_position();
        error(toktype::colon, ":", "Expected ':'");
        result->pos = current_pos_;
        // parsing the statement itself (CLASSBUILTIN, VAR_ASSIGN, EXEC)
        while (!is_token(toktype::exc_mark)) { // the statement stops when execution starts (! is execution script)
            if (is_token_type(toktype::keyword))
                result->stmts.push_back(parse_class_builtin());
            else if (is_token_type(toktype::name) && tokens_[idx_ + 1].type == toktype::equals)
                result->stmts.push_back(parse_var_assign());
            else if (is_token_type(toktype::name) && tokens_[idx_ + 1].type == toktype::exc_mark)
                result->stmts.push_back(parse_exec());
            if (!is_token(toktype::exc_mark))
                error(toktype::semicolon, ";", "Expected ';' after statement");
        }
        return result;
    }

    std::unique_ptr<NodeProg> parse_program() {
        current_pos_ = ParsePosition(specialpos::ENTRY);
        error(toktype::keyword, "ZetriScript", "EXPECTED 'ZetriScript'");
        error(toktype::exc_mark, "!", "Expected '!'");
        visit_position();
        // parsing the program body
        std::vector<std::unique_ptr<NodeStmt>> statements;
        while (!is_token(toktype::keyword, "ZetriScript")) {
            std::unique_ptr<NodeStmt> stmt = parse_statement();
            statements.push_back(std::move(stmt));
        }
        return result;
    }

    void visit_position() {
        /*
        A little part of an interpreter for position access node, but it's not used in the actual interpreter with the way lines of code are used, 
        this is used in parsing to send errors without seeing expression and seeing the actual numbers.
        This part of code is for demonstration purposes and doesn't include actual interpreter logic
        It should be implemented according to the ZetriScript syntax and semantics.
        For example, it could create a Position object, parse expressions, and return the result.
        */
        ParseResult result;
        // parsing position access node
        const ParsePosition entry = ParsePosition(current_pos_);
        current_pos_ = ParsePosition(specialpos::POS_DECL);
        error(toktype::left_square, "[", "Expected '['");
        // parsing expressions
        std::variant<std::unique_ptr<NodeBinOp>, std::unique_ptr<NodeNumber>> expr1 = expr();
        error(toktype::colon, ":", "Expected ':'");
        std::variant<std::unique_ptr<NodeBinOp>, std::unique_ptr<NodeNumber>> expr2 = expr();
        error(toktype::colon, ":", "Expected ':'");
        std::variant<std::unique_ptr<NodeBinOp>, std::unique_ptr<NodeNumber>> expr3 = expr();
        error(toktype::colon, ":", "Expected ':'");
        error(toktype::right_square, "]", "Expected ']'");
        // visiting binary operation nodes
        std::string num1, num2, num3;

        if (std::holds_alternative<std::unique_ptr<NodeBinOp>>(expr1)) {
            NodeBinOp binop1 = std::get<std::unique_ptr<NodeBinOp>>(expr1);
            num1 = evaluate_expression(binop1);
        } else num1 = std::to_string(std::get<std::unique_ptr<NodeNumber>>(expr1).value);
        
        if (std::holds_alternative<std::unique_ptr<NodeBinOp>>(expr2)) {
            NodeBinOp binop2 = std::get<std::unique_ptr<NodeBinOp>>(expr2);
            num2 = evaluate_expression(binop2);
        } else num2 = std::to_string(std::get<std::unique_ptr<NodeNumber>>(expr2).value);
        
        if (std::holds_alternative<std::unique_ptr<NodeBinOp>>(expr3)) {
            NodeBinOp binop3 = std::get<std::unique_ptr<NodeBinOp>>(expr3);
            num3 = evaluate_expression(binop3);
        } else num3 = std::to_string(std::get<std::unique_ptr<NodeNumber>>(expr3).value);

        current_pos_ = ParsePosition(num1, num2, num3);
    }

    std::string evaluate_expression(NodeBinOp node) {
        /*
        A little part of an interpreter for binary operation node
        This part of code is for demonstration purposes and doesn't include actual interpreter logic
        It should be implemented according to the ZetriScript syntax and semantics.
        For example, it could create a BinaryOperation object, parse expressions, and return the result.
        This function doesn't check if the node is binary operation or not,
        it assumes that the node is a binary operation.
        */
        // parsing binary operation node
        std::string left_expr, right_expr;
        if (std::holds_alternative<NodeBinOp>(node.left_expr)) left_expr = evaluate_expression(std::get<NodeBinOp>(node.left_expr));
        else if (std::holds_alternative<NodeNumber>(node.left_expr)) left_expr = std::to_string(std::get<NodeNumber>(node.left_expr).value);
        else throw std::runtime_error("Unknown left expression type");
        if (std::holds_alternative<NodeBinOp>(node.right_expr)) right_expr = evaluate_expression(std::get<NodeBinOp>(node.right_expr));
        else if (std::holds_alternative<NodeNumber>(node.right_expr)) right_expr = std::to_string(std::get<NodeNumber>(node.right_expr).value);
        else throw std::runtime_error("Unknown right expression type");
        // applying binary operation
        if (node->op.type == toktype::plus) {
            return std::to_string(std::stoi(left_expr) + std::stoi(right_expr));
        }
        else if (node->op.type == toktype::minus) {
            return std::to_string(std::stoi(left_expr) - std::stoi(right_expr));
        }
        else if (node->op.type == toktype::mult) {
            return std::to_string(std::stoi(left_expr) * std::stoi(right_expr));
        }
        else if (node->op.type == toktype::div) {
            if (std::stoi(right_expr) == 0) {
                error(current_token_.pos, toktype::div, "Division by zero");
                stop_parse();
            }
            return std::to_string(std::stoi(left_expr) / std::stoi(right_expr));
        }
        throw std::runtime_error("Unknown binary operation");
    }

private:
    const std::vector<Token> tokens_;
    size_t idx_ = 0;
    ParsePosition current_pos_;
    Token current_token_;
    ErrorSyntax current_error_;

    inline void next_token() {
        if (idx_ < tokens_.size()) {
            current_token_ = tokens_[idx_];
            idx_++;
        } else {
            current_token_ = Token(current_pos_, current_pos_, toktype::eof_);
        }
    }

    inline std::optional<Token> peek(toktype type) {
        if (idx_ < tokens_.size() && tokens_[idx_].type == type) {
            return tokens_[idx_];
        }
        return std::nullopt;
    }

    inline boolean is_token_type(toktype type) {
        return current_token_.type == type;
    }

    inline boolean is_token(toktype type, const std::string &token) {
        return current_token_.type == type && current_token_.value == token;
    }

    inline void stop_parse() {
        throw std::runtime_error("PARSING STOPPED DUE TO A SYNTAX ERROR");
    }

    inline void error(toktype type, const std::string &value, const std::string &msg) {
        if (!is_token(type, value)) {
            current_error_ = ErrorSyntax(current_pos_, msg);
            stop_parse();
        }
        next_token();
    }

    inline std::optional<Token> error_type(toktype type, const std::string &msg, bool getTok=false) {
        if (!is_token_type(type)) {
            current_error_ = ErrorSyntax(current_pos_, msg);
            stop_parse();
        }
        if (getTok) {
            Token tok = current_token_;
            next_token();
            return tok;
        }
        next_token();
        return std::nullopt;
    }

    inline void expected_symbol(toktype type, const std::string &value, const std::string &msg) {
        // checking if current token is inside the string value
        if (!is_token_type(type) || current_token_.value.find(value) == std::string::npos) {
            current_error_ = ErrorSyntax(current_pos_, msg);
            stop_parse();
        }
        next_token();
    }
};