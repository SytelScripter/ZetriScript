#include <memory>
#include <variant>
#include <functional>
#include "lexer.cpp"
#include "error.cpp"

// declaration of all nodes
struct NodeNumber;
struct NodeBinOp;
struct NodeExec;
struct NodeClassBuiltIn;
struct NodeVarAssign;
struct NodeVarAccess;
struct NodePosAccess;
struct NodeStmt;
struct NodeProg;

// usings
using std::variant, std::vector, std::unique_ptr, std::make_unique, std::move, std::string;
using anyNode = variant<
    unique_ptr<NodeProg>, 
    unique_ptr<NodeStmt>, 
    unique_ptr<NodePosAccess>, 
    unique_ptr<NodeVarAccess>, 
    unique_ptr<NodeVarAssign>, 
    unique_ptr<NodeClassBuiltIn>, 
    unique_ptr<NodeExec>, 
    unique_ptr<NodeBinOp>, 
    unique_ptr<NodeNumber>
>;

// definitions of all nodes
struct NodeNumber {
    Token_ num_tok;
};

struct NodeBinOp {
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>> left;
    Token_ op_tok;
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>> right;
};

struct NodeExec {
    variant<unique_ptr<NodeVarAccess>, unique_ptr<NodeClassBuiltIn>> executed;
};

struct NodeClassBuiltIn {
    Token_ class_name_tok;
    vector<variant<unique_ptr<ParsePosition>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>>> args;
};

struct NodeVarAssign {
    Token_ var_name_tok;
    variant<unique_ptr<NodeExec>, unique_ptr<NodeClassBuiltIn>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>> value;
};

struct NodeVarAccess {
    Token_ var_name_tok;
};

struct NodePosAccess {
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> x;
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> y;
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> z;
};

struct NodeStmt {
    ParsePosition pos;
    vector<variant<unique_ptr<NodeClassBuiltIn>, unique_ptr<NodeVarAssign>, unique_ptr<NodeExec>>> stmts;
};

struct NodeProg {
    NodePosAccess entry_pos;
    vector<variant<unique_ptr<NodeClassBuiltIn>, unique_ptr<NodeVarAssign>, unique_ptr<NodeExec>>> prog;
};


// parse result
class ParseResult {
    public:
    anyNode node;
    ErrorSyntax error;

    ParseResult(ErrorSyntax error_) : error(error_) {}
    ParseResult(anyNode node_) : node(move(node_)), error(ErrorSyntax()) {}
};

class Parser {
    public:
    inline explicit Parser(const vector<Token_>& tokens_) : 
        tokens(move(tokens_)) {
            advance();
        }

    unique_ptr<ParseResult> parse_factor() {
        Token_ value = tokens[idx++];
        unique_ptr<NodeNumber> node = make_unique<NodeNumber>();
        node->num_tok = value;

        anyNode wrapped_node = move(node);
        unique_ptr<ParseResult> result = make_unique<ParseResult>(move(wrapped_node));
        return result;
    }

    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> parse_term() {
        auto left = parse_factor();
        while (idx < tokens.size() && (is_token_type(toktype::mul) || is_token_type(toktype::div))) {
            Token_ op_tok = tokens[idx++];
            unique_ptr<NodeBinOp> bin_op = make_unique<NodeBinOp>();
            bin_op->op_tok = op_tok;
            bin_op->left = move(left);
            bin_op->right = parse_factor();
            left = move(bin_op);
        }
        return move(left);
    }

    private:
    int idx = -1;
    vector<Token_> tokens;
    Token_ current_tok;

    inline void advance() {
        if (idx < tokens.size()) {
            current_tok = tokens[idx];
        }
    }

    inline bool is_token_type(toktype type) const {
        return current_tok.type == type;
    }

    inline bool is_token(toktype type, const string& value) const {
        return current_tok.type == type && current_tok.value == value;
    }
};