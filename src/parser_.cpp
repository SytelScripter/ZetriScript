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

// definitions of all nodes
struct NodeNumber {
    Token_ num_tok;
};

struct NodeBinOp {
    bintype left;
    Token_ op_tok;
    bintype right;
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

    ParseResult() {}

    inline void register_(function<unique_ptr<ParseResult>()> parse_func) {
        unique_ptr<ParseResult> temp = parse_func();
        if (temp->error.empty()) {
            node = move(temp->extract_node());
            return;
        }
        error = move(temp->error);
        return;
    }

    template <typename... Types>
    unique_ptr<std::decay_t<Types>> extract_node() {
        return extract_node_impl<Types...>();
    }
    
    private:
    template <typename T, typename... Types>
    std::unique_ptr<std::decay_t<T>> extract_node_impl() {
        // Check if the node holds the type T
        if (holds_alternative<std::unique_ptr<T>>(node)) {
            return std::move(get<std::unique_ptr<T>>(node));
        }
        // Recursively check the next type in the pack if not matched
        return extract_node_impl<Types...>();
    }

    template <>
    std::unique_ptr<void> extract_node_impl<>() {
        throw std::runtime_error("Unexpected type in variant");
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

        return parse_result(move(node));
    }

    unique_ptr<ParseResult> parse_term() {
        unique_ptr<NodeBinOp> node = make_unique<NodeBinOp>();
        unique_ptr<ParseResult> result = make_unique<ParseResult>();
        result->register_([this]() { return parse_factor(); });
        node->left = move(result->extract_node<NodeNumber, NodeBinOp, NodeVarAccess>());


        while (is_token_type(toktype::mul) || is_token_type(toktype::minus)) {
            Token_ op_tok = current_tok;
            advance();
            node->op_tok = op_tok;

            result->register_([this]() { return parse_factor(); });
            node->right = move(result->extract_node<NodeNumber, NodeBinOp, NodeVarAccess>());
        }

        return parse_result(move(node));
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

    inline unique_ptr<ParseResult> parse_result(anyNode node) {
        anyNode wrapped_node = move(node);
        unique_ptr<ParseResult> result = make_unique<ParseResult>();
        result->node = move(wrapped_node);
        return result;
    }
};