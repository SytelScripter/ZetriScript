#include "lexer.cpp"
#include "error.cpp"

namespace node {
    struct NodeNumber {};
    struct NodeGoto {};
    struct NodeClass {};
    struct NodeExec {};
    struct NodeVarAssign {};
    struct NodeVarAccess {};
    struct NodePosAssign {};
    struct NodePosAccess {};
    struct NodeLine {};
    struct NodeProgram {};
}

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens) : 
        tokens_(std::move(tokens)) {}

    NodeNumber* number() {
        if (current_token_.type == toktype::number_) {
            auto result = new node::NodeNumber();
            next_token();
            return result;
        }
        Error error = Error(current_pos_, "expected number, got " + current_token_.to_string());
        error.display();
        error("expected number, got " + current_token_.to_string());
        return nullptr;
    }
    
    NodeProgram* parse() {
        return program();
    }

private:
    const std::vector<Token> tokens_;
    size_t idx_ = 0;
    Position current_pos_;
    Token current_token_;

    inline void next_token() {
        if (idx_ < tokens_.size()) {
            current_token_ = tokens_[idx_];
            current_pos_ = current_token_.get_position();
            idx_++;
        } else {
            current_token_ = Token(current_pos_, current_pos_, toktype::eof_);
            current_pos_ = Position(0, 0, 0);
    }

    inline std::optional<Token> peek(toktype type) {
        if (idx_ < tokens_.size() && tokens_[idx_].type == type) {
            return tokens_[idx_];
        }
        return std::nullopt;
    }
};