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
        unique_ptr<ParseResult> temp = move(parse_func());
        if (temp->error.isEmpty()) {
            node = move(temp->extract_node());
            return;
        }
        error = move(temp->error);
        return;
    }

    // extract node extracts from the node variant existing, but it doesn't check every type, it checks only types that are given in the function.
    auto extract_node() {
        if (auto ptr = std::get_if<unique_ptr<NodeProg>>(&node)) return std::move(*ptr);
        if (auto ptr = std::get_if<unique_ptr<NodeStmt>>(&node)) return std::move(*ptr);
        if (auto ptr = std::get_if<unique_ptr<NodePosAccess>>(&node)) return std::move(*ptr);
        if (auto ptr = std::get_if<unique_ptr<NodeVarAccess>>(&node)) return std::move(*ptr);
        if (auto ptr = std::get_if<unique_ptr<NodeVarAssign>>(&node)) return std::move(*ptr);
        if (auto ptr = std::get_if<unique_ptr<NodeClassBuiltIn>>(&node)) return std::move(*ptr);
        if (auto ptr = std::get_if<unique_ptr<NodeExec>>(&node)) return std::move(*ptr);
        if (auto ptr = std::get_if<unique_ptr<NodeBinOp>>(&node)) return std::move(*ptr);
        if (auto ptr = std::get_if<unique_ptr<NodeNumber>>(&node)) return std::move(*ptr);
    }
};

class Parser {
    public:
    inline explicit Parser(const vector<Token_>& tokens_) : 
        tokens(move(tokens_)) {
            advance();
        }

    unique_ptr<ParseResult> parse_factor() {
        unique_ptr<ParseResult> result = make_unique<ParseResult>();

        Token_ value = tokens[idx++];
        unique_ptr<NodeNumber> node = make_unique<NodeNumber>();
        node->num_tok = value;

        result->node = move(node);

        return move(result);
    }

    unique_ptr<ParseResult> parse_term() {
        unique_ptr<NodeBinOp> node = make_unique<NodeBinOp>();
        unique_ptr<ParseResult> result = make_unique<ParseResult>();
        // std::vector<int> types = { get_i<unique_ptr<NodeNumber>>(), get_i<unique_ptr<NodeBinOp>>(), get_i<unique_ptr<NodeVarAccess>>() }; // {8, 7, 3}

        result->register_([this]() { return parse_factor(); });
        if (!result->error.isEmpty()) return result;
        // node->left = move(result->extract_node(types));
        node->left = move(result->extract_node());

        while (is_token_type(toktype::mul) || is_token_type(toktype::minus)) {
            Token_ op_tok = current_tok;
            advance();
            node->op_tok = op_tok;

            result->register_([this]() { return parse_factor(); });
            if (!result->error.isEmpty()) return result;
            // node->right = move(result->extract_node(types));
            node->right = move(result->extract_node());
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

    // template <typename nodeT>
    // int get_i() {
    //     if (std::is_same_v<nodeT, unique_ptr<NodeProg>>) return 0;
    //     if (std::is_same_v<nodeT, unique_ptr<NodeStmt>>) return 1;
    //     if (std::is_same_v<nodeT, unique_ptr<NodePosAccess>>) return 2;
    //     if (std::is_same_v<nodeT, unique_ptr<NodeVarAccess>>) return 3;
    //     if (std::is_same_v<nodeT, unique_ptr<NodeVarAssign>>) return 4;
    //     if (std::is_same_v<nodeT, unique_ptr<NodeClassBuiltIn>>) return 5;
    //     if (std::is_same_v<nodeT, unique_ptr<NodeExec>>) return 6;
    //     if (std::is_same_v<nodeT, unique_ptr<NodeBinOp>>) return 7;
    //     if (std::is_same_v<nodeT, unique_ptr<NodeNumber>>) return 8;
    //     else throw std::runtime_error("Parser::get_index: unsupported type (not included in anyNode)");
    // }

};

int main() {
    // your test cases here
    std::string test = "1 + 2";
    Lexer lexer = Lexer(test);
    std::vector<Token_> tokens = lexer.makeTokens();
    Parser parser(tokens);
    unique_ptr<ParseResult> result = parser.parse_term();
    // if (result->error.isEmpty()) {
    //     // process the parsed result
    //     //...
    // } else {
    //     // handle the error
    //     //...
    // }

    return 0;
}