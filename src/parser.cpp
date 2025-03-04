#include "lexer.cpp"
#include <memory>
#include <variant>

struct NodePosition {
    std::string x;
    std::string y;
    std::string z;
};

struct NodeLine {
    NodePosition startPoint;
    NodePosition Vector3;
};

struct NodeIdentifier {
    Token identName;
};

struct NodeVarAssign {
    NodeIdentifier ident;
    Node
}

struct NodeAlloc {
    NodeIdentifier;
};