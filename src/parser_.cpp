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
using std::variant, std::vector, std::unique_ptr, std::make_unique, std::move;
using anyNode = variant<NodeProg, NodeStmt, NodePosAccess, NodeVarAccess, NodeVarAssign, NodeClassBuiltIn, NodeExec, NodeBinOp, NodeNumber>;

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
    vector<variant<unique_ptr<ParsePosition>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>> args;
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

    ParseResult(ErrorSyntax error_) : node(nullptr), error(error_) {}
    ParseResult(anyNode node_) : node(node_), error(ErrorSyntax{}) {}
};

class Parser {
    public:
    inline explicit Parser(vector<Token_> tokens) : 
        tokens_(move(tokens)) {
            advance();
        }

    ParseResult parse_factor() {
        Token_ value = tokens[idx_++];
        unique_ptr<NodeNumber> result = make_unique<NodeNumber>(value);
        return move(result);
    }

    private:
    int idx_ = -1;

    inline void advance() {
        if (idx_ < tokens_.size()) {
            current_token_ = tokens_[idx_];
        }
    }
};