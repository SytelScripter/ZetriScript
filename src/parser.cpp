#include "lexer.cpp"
#include "error.cpp"
#include <memory>

namespace node {
    struct NodeNumber {};
    struct NodeTerm {};
    struct NodeExpr {};
    struct NodeGoto {};
    struct NodeClassBuiltIn {};
    struct NodeExec {};
    struct NodeVarAssign {};
    struct NodeVarAccess {};
    struct NodePosAssign {};
    struct NodePosAccess {};
    struct NodeStmt {};
    struct NodeProgram {};

    struct NodeNumber {
        Token numberToken;
    };

    struct NodeTerm {
        std::variant<std::unique_ptr<node::NodeNumber>, std::unique_ptr<node::NodeTerm>> left;
        Token opTok;
        std::variant<std::unique_ptr<node::NodeNumber>, std::unique_ptr<node::NodeTerm>> right;
    };

    struct NodeExpr {
        std::variant<std::unique_ptr<node::NodeTerm>, std::unique_ptr<node::NodeExpr>> left;
        Token opTok;
        std::variant<std::unique_ptr<node::NodeTerm>, std::unique_ptr<node::NodeExpr>> right;
    };

    struct NodeGoto {
        std::unique_ptr<node::NodePosAccess> pos;
    };

    struct NodeClassBuiltIn {
        Token className;
        std::vector<std::variant<std::unique_ptr<node::NodeExpr>, std::unique_ptr<node::NodePosAccess>>> argsList;
    };

    struct NodeExec {
        std::unique_ptr<node::NodeVarAccess> var;
    };

    struct NodeVarAssign {
        std::unique_ptr<node::NodeVarAccess> var;
        std::unique_ptr<node::NodeExpr> expr;
    };

    struct NodeVarAccess {
        Token varName;
    };

    struct NodePosAssign {
        std::unique_ptr<node::NodePosAccess> pos;
    };

    struct NodePosAccess {
        std::variant<std::unique_ptr<node::NodeExpr>, std::unique_ptr<node::NodeVarAccess>> x;
        std::variant<std::unique_ptr<node::NodeExpr>, std::unique_ptr<node::NodeVarAccess>> y;
        std::variant<std::unique_ptr<node::NodeExpr>, std::unique_ptr<node::NodeVarAccess>> z;
    };

    struct NodeStmt {
        std::unique_ptr<node::NodePosAccess> pos;
        std::variant<std::unique_ptr<node::NodePosAccess>, std::unique_ptr<node::NodePosAssign>, std::unique_ptr<node::NodeVarAccess>, std::unique_ptr<node::NodeVarAssign>, std::unique_ptr<node::NodeExec>, std::unique_ptr<node::NodeClassBuiltIn>, std::unique_ptr<node::NodeGoto>, std::unique_ptr<node::NodeExpr>> stmt;
    };

    struct NodeProgram {
        std::vector<std::unique_ptr<node::NodeStmt>> stmtList;
    };
}

using anyNode = std::variant<std::unique_ptr<node::NodeNumber>, std::unique_ptr<node::NodeTerm>, std::unique_ptr<node::NodeExpr>, std::unique_ptr<node::NodeGoto>, std::unique_ptr<node::NodeClassBuiltIn>, std::unique_ptr<node::NodeExec>, std::unique_ptr<node::NodeVarAssign>, std::unique_ptr<node::NodeVarAccess>, std::unique_ptr<node::NodePosAssign>, std::unique_ptr<node::NodePosAccess>, std::unique_ptr<node::NodeStmt>, std::unique_ptr<node::NodeProgram>>;

struct ParseResult {
    std::optional<anyNode> result_node;
    std::optional<ErrorSyntax> result_error;

    void clear() {
        result_node.reset();
        result_error.reset();
    }
}

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens) : 
        tokens_(std::move(tokens)) {}


    ParseResult* number() {
        ParseResult* result = new ParseResult();
        if (current_token_.type == toktype::int_lit || current_token_.type == toktype::float_lit) {
            result->result_node = std::make_unique<node::NodeNumber>(current_token_);
            next_token();
            return result;
        }
        ErrorSyntax error = ErrorSyntax(current_pos_, "expected number literal");
        result->result_error = error;
        return result;
    }
    ParseResult* term() {
        ParseResult* result = number();
        if (result->result_error) result_error;
        while (current_token_.type == toktype::mul || current_token_.type == toktype::div) {
            Token op = current_token_;
            next_token();
            ParseResult* right_ = number();
            if (right_->result_error) {
                result->clear();
                result->result_error = right_->result_error;
                return result;
            }
            result->result_node = std::make_unique<node::NodeTerm>(std::move(result->result_node), op, std::move(right_));
        }
        return result;
    }
    ParseResult* expr() {
        ParseResult* result = term();
        if (result->result_error) result_error;
        while (is_token_type(toktype::plus) || is_token_type(toktype::minus)) {
            Token op = current_token_;
            next_token();
            ParseResult* right_ = term();
            if (right_->result_error) {
                result->clear();
                result->result_error = right_->result_error;
                return result;
            }
            result->result_node = std::make_unique<node::NodeExpr>(std::move(result->result_node), op, std::move(right_->result_node));
        }
        return result;
    }
    ParseResult* pos_goto() {
        ParseResult* result = new ParseResult();
        if (!is_token(toktype::keyword, "goto")) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected 'goto'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        ParseResult* pos_access_ = pos_access();
        if (pos_access_->result_error) pos_access_;
        result->result_node = std::make_unique<node::NodeGoto>(std::move(pos_access_->result_node));
        if (!is_token_type(toktype::exc_mark)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected '!'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        return result;
    }
    ParseResult* classbuiltin() {
        ParseResult* result = new ParseResult();
        if (current_token_.type != toktype::keyword) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected keyword");
            result->clear();
            result->result_error = error;
            return result;
        }
        Token className = current_token_;
        next_token();
        if (!is_token_type(toktype::lparen)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected '('");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        std::vector<std::variant<std::unique_ptr<node::NodeExpr>, std::unique_ptr<node::NodePosAccess>>> argsList;
        while (idx < tokens_.size() || !is_token_type(toktype::rparen)) {
            ParseResult* arg_expr = parse_expr();
            ParseResult* arg_pos_access = parse_pos_access();
            if (arg_expr->result_error || arg_pos_access->result_error) {
                ErrorSyntax error = ErrorSyntax(current_pos_, "expected expression or position access");
                result->clear();
                result->result_error = error;
                return result;
            }
            argsList.push_back(std::move(arg->result_node));
            if (!is_token_type(toktype::comma)) {
                ErrorSyntax error = ErrorSyntax(current_pos_, "expected ','");
                result->clear();
                result->result_error = error;
                return result;
            }
            next_token();
        }
        if (idx >= tokens_.size()) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected ')'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        result->result_node = std::make_unique<node::NodeClassBuiltIn>(className, std::move(argsList));
        return result;
    }
    ParseResult* pos_exec() {
        ParseResult* result = new ParseResult();
        std::optional<Token> exc_mark_ = peek(1);
        if (!exc_mark_.has_value()) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected '!'");
            result->clear();
            result->result_error = error;
            return result;
        }
        ParseResult* var_access_ = var_access();
        if (var_access_->result_error) return var_access_;
        result->result_node = std::make_unique<node::NodeExec>(var_access_->result_node);
    }
    ParseResult* var_assign() {
        ParseResult* result = new ParseResult();
        if (current_token_.type!= toktype::identifier) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected identifier");
            result->clear();
            result->result_error = error;
            return result;
        }
        Token id = current_token_;
        next_token();
        if (!is_token_type(toktype::assign)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected '='");
            result->clear();
            result->result_error = error;
            return result;
        }
        ParseResult* expr_ = parse_expr();
        if (expr_->result_error) return expr_;
        result->result_node = std::make_unique<node::NodeVarAssign>(id, std::move(expr_->result_node));
        return result;
    }
    ParseResult* var_access() {
        ParseResult* result = new ParseResult();
        if (current_token_.type!= toktype::identifier) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected identifier");
            result->clear();
            result->result_error = error;
            return result;
        }
        Token id = current_token_;
        next_token();
        result->result_node = std::make_unique<node::NodeVarAccess>(id);
        return result;
    }
    ParseResult* pos_assign() {
        ParseResult* result = new ParseResult();
        if (!is_token_type(toktype::left_square)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected '['");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        ParseResult* expr_ = parse_expr();
        if (expr_->result_error) return expr_;
        if (!is_token_type(toktype::colon)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected ':'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        ParseResult* expr_2_ = parse_expr();
        if (expr_2_->result_error) return expr_2_;
        if (!is_token_type(toktype::colon)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected ':'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        ParseResult* expr_3_ = parse_expr();
        if (expr_3_->result_error) return expr_3_;
        if (!is_token_type(toktype::right_square)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected ']'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        result->result_node = std::make_unique<node::NodePosAssign>(std::move(expr_->result_node), std::move(expr_2_->result_node), std::move(expr_3_->result_node));
        return result;
    }
    ParseResult* pos_access() {
        ParseResult* result = new ParseResult();
        if (!is_token_type(toktype::left_square)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected '['");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        ParseResult* expr_ = parse_expr();
        if (expr_->result_error) return expr_;
        if (!is_token_type(toktype::colon)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected ':'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        ParseResult* expr_2_ = parse_expr();
        if (expr_2_->result_error) return expr_2_;
        if (!is_token_type(toktype::colon)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected ':'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        ParseResult* expr_3_ = parse_expr();
        if (expr_3_->result_error) return expr_3_;
        if (!is_token_type(toktype::right_square)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected ']'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        if (!is_token_type(toktype::colon)) {
            ErrorSyntax error = ErrorSyntax(current_pos_, "expected ':'");
            result->clear();
            result->result_error = error;
            return result;
        }
        next_token();
        result->result_node = std::make_unique<node::NodePosAssign>(std::move(expr_->result_node), std::move(expr_2_->result_node), std::move(expr_3_->result_node));
        return result;
    }
    ParseResult* parse_stmt() {
        ParseResult* result = new ParseResult();
        // first parse the position (pos_assign) and then command (the statement itself)
        current_pos_ = ParsePosition(specialpos::POS_DECL);
        ParseResult* pos_assign_ = pos_assign();
        if (pos_assign_->result_error) return pos_assign_;
        Token token = current_token_;
        current_pos_ = ParsePosition(pos_assign_->result_node->x, pos_assign_->result_node->y, pos_assign_->result_node->z);
        
    }
    ParseResult* parse_program() {
        ParseResult* result = new ParseResult();
    }

private:
    const std::vector<Token> tokens_;
    size_t idx_ = 0;
    ParsePosition current_pos_;
    Token current_token_;

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
};