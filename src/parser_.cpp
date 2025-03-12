#include <memory>
#include <variant>
#include <typeindex>
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
using std::variant, 
    std::get, 
    std::vector, 
    std::unique_ptr, 
    std::make_unique, 
    std::move, 
    std::string, 
    std::function, 
    std::holds_alternative, 
    std::visit;
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
using bintype = variant<
    unique_ptr<NodeNumber>,
    unique_ptr<NodeBinOp>,
    unique_ptr<NodeVarAccess>
>;
const int nodesLen = 9;

// definitions of all nodes
struct NodeNumber {
    int type = 8;
    Token_ num_tok;
};

struct NodeBinOp {
    int type = 7;
    bintype left;
    Token_ op_tok;
    bintype right;
};

struct NodeExec {
    int type = 6;
    variant<unique_ptr<NodeVarAccess>, unique_ptr<NodeClassBuiltIn>> executed;
};

struct NodeClassBuiltIn {
    int type = 5;
    Token_ class_name_tok;
    vector<variant<unique_ptr<ParsePosition>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>>> args;
};

struct NodeVarAssign {
    int type = 4;
    Token_ var_name_tok;
    variant<unique_ptr<NodeExec>, unique_ptr<NodeClassBuiltIn>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>> value;
};

struct NodeVarAccess {
    int type = 3;
    Token_ var_name_tok;
};

struct NodePosAccess {
    int type = 2;
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> x;
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> y;
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> z;
};

struct NodeStmt {
    int type = 1;
    ParsePosition pos;
    vector<variant<unique_ptr<NodeClassBuiltIn>, unique_ptr<NodeVarAssign>, unique_ptr<NodeExec>>> stmts;
};

struct NodeProg {
    int type = 0;
    NodePosAccess entry_pos;
    vector<variant<unique_ptr<NodeClassBuiltIn>, unique_ptr<NodeVarAssign>, unique_ptr<NodeExec>>> prog;
};


// parse result
class ParseResult {
    public:
    anyNode node;
    ErrorSyntax error;

    ParseResult() {}

    inline void register_(function<unique_ptr<ParseResult>()> parse_func) {
        unique_ptr<ParseResult> temp = parse_func();
        if (temp->error.isEmpty()) {
            node = move(temp->extract_node());
            return;
        }
        error = move(temp->error);
        return;
    }

    // extract node extracts from the node variant existing, but it doesn't check every type, it checks only types that are given in the function.
    auto extract_node(std::vector<int> types) {
        int existing_type = node.index();
        for (int type : types) {
            if (type == existing_type) {
                switch (type) {
                    case 0: return std::move(std::get<std::unique_ptr<NodeProg>>(node));
                    case 1: return std::move(std::get<std::unique_ptr<NodeStmt>>(node));
                    case 2: return std::move(std::get<std::unique_ptr<NodePosAccess>>(node));
                    case 3: return std::move(std::get<std::unique_ptr<NodeVarAccess>>(node));
                    case 4: return std::move(std::get<std::unique_ptr<NodeVarAssign>>(node));
                    case 5: return std::move(std::get<std::unique_ptr<NodeClassBuiltIn>>(node));
                    case 6: return std::move(std::get<std::unique_ptr<NodeExec>>(node));
                    case 7: return std::move(std::get<std::unique_ptr<NodeBinOp>>(node));
                    case 8: return std::move(std::get<std::unique_ptr<NodeNumber>>(node));
                    default:
                        throw std::runtime_error("Invalid type index in variant");
                }
            }
        }
        throw std::runtime_error("Invalid node type");
    }
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

        return parse_result<unique_ptr<NodeNumber>>(move(node));
    }

    unique_ptr<ParseResult> parse_term() {
        unique_ptr<NodeBinOp> node = make_unique<NodeBinOp>();
        unique_ptr<ParseResult> result = make_unique<ParseResult>();
        std::vector<int> types = { get_i<NodeNumber>(), get_i<NodeBinOp>(), get_i<NodeVarAccess>() }; // {8, 7, 3}

        result->register_([this]() { return parse_factor(); });
        if (!result->error.isEmpty()) return result;
        node->left = move(result->extract_node(types));


        while (is_token_type(toktype::mul) || is_token_type(toktype::minus)) {
            Token_ op_tok = current_tok;
            advance();
            node->op_tok = op_tok;

            result->register_([this]() { return parse_factor(); });
            if (!result->error.isEmpty()) return result;
            node->right = move(result->extract_node(types));
        }

        return parse_result<unique_ptr<NodeBinOp>>(move(node));
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

    template <typename T>
    inline unique_ptr<ParseResult> parse_result(T node) {
        unique_ptr<ParseResult> result = make_unique<ParseResult>();
        result->node = move(node);
        return result;
    }

    template <typename nodeT>
    int get_i() {
        if (std::is_same_v<nodeT, NodeNumber>) return 8;
        else if (std::is_same_v<nodeT, NodeBinOp>) return 7;
        else if (std::is_same_v<nodeT, NodeExec>) return 6;
        else if (std::is_same_v<nodeT, NodeClassBuiltIn>) return 5;
        else if (std::is_same_v<nodeT, NodeVarAssign>) return 4;
        else if (std::is_same_v<nodeT, NodeVarAccess>) return 3;
        else if (std::is_same_v<nodeT, NodePosAccess>) return 2;
        else if (std::is_same_v<nodeT, NodeStmt>) return 1;
        else if (std::is_same_v<nodeT, NodeProg>) return 0;
        else throw std::runtime_error("Parser::get_index: unsupported type (not included in anyNode)");
    }

};