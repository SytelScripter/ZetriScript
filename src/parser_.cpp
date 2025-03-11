#include <memory>
#include <variant>
#include <functional>
#include "lexer.cpp"
#include "error.cpp"

using std::variant, std::vector, std::unique_ptr;

struct NodeNumber;
struct NodeBinOp;
struct NodeExec;
struct NodeClassBuiltIn;
struct NodeVarAssign;
struct NodeVarAccess;
struct NodePosAccess;
struct NodeStmt;
struct NodeProg;

struct NodeNumber {
    Token_ num_tok;
};

struct NodeBinOp {
    variant<NodeNumber> left;
    Token_ op_tok;
    variant<NodeNumber> right;
};
    