#include "lexer.cpp"
#include "error.cpp"
#include <memory>

namespace node {
    struct NodeNumber {};
    struct NodeTerm {};
    struct NodeExpr {};
    struct NodeExec {};
    struct NodeClassBuiltIn {};
    struct NodeVarAssign {};
    struct NodeVarAccess {};
    struct NodePosAccess {};
    struct NodeStmt {};
    struct NodeProgram {};

    
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