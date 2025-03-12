#include <memory>
#include <variant>
#include <optional>
#include <stdexcept>
#include <typeindex>
#include <functional>
#include <type_traits>
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
};

class Parser {
    public:
    inline explicit Parser(const vector<Token_>& tokens_) : 
        tokens(move(tokens_)) {
            advance();
        }

    unique_ptr<ParseResult> parse_factor() {
        unique_ptr<ParseResult> result_factor = make_unique<ParseResult>();
        TokenPosition start_pos = current_tok.posStart;
        TokenPosition end_pos = current_tok.posEnd;

        if (is_tok_type(toktype::int_lit) || is_tok_type(toktype::float_lit)) {
            Token_ value = tokens[idx++];
            unique_ptr<NodeNumber> node = make_unique<NodeNumber>();
            node->num_tok = value;
            result_factor->node = move(node);
            return move(result_factor);
        }
        else if (is_tok_type(toktype::name)) {
            Token_ name_tok = tokens[idx++];
            unique_ptr<NodeVarAccess> node = make_unique<NodeVarAccess>();
            node->var_name_tok = name_tok;
            result_factor->node = move(node);
            return move(result_factor);
        }
        else if (is_tok_type(toktype::left_paren)) {
            advance(); // '('
            unique_ptr<ParseResult> result_expr = parse_expr();
            unique_ptr<NodeBinOp> node = make_unique<NodeBinOp>();
            if (!result_expr->error.isEmpty()) return result_expr;

            if (!is_tok_type(toktype::right_paren)) {
                ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN); // temporary
                result_factor->error = ErrorSyntax(parse_position, std::string("EXPECTED ')'"));
                return move(result_factor);
            }
            advance(); // ')'
            return move(result_expr);
        }

        ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN); // temporary
        result_factor->error = ErrorSyntax(parse_position, std::string("EXPECTED INT_LIT, FLOAT_LIT, NAME, OR '(', BUT GOT: '") + current_tok.to_string() + std::string("'"));
        return move(result_factor);
    }

    unique_ptr<ParseResult> parse_term() {
        unique_ptr<NodeBinOp> node = make_unique<NodeBinOp>();
        unique_ptr<ParseResult> result_term = move(parse_factor());
        
        if (!result_term->error.isEmpty()) return result_term;
        node->left = move(convert_node<bintype>(result_term->node));

        while (is_tok_type(toktype::mul) || is_tok_type(toktype::minus)) {
            Token_ op_tok = current_tok;
            advance();
            node->op_tok = op_tok;

            result_term = move(parse_factor());
            if (!result_term->error.isEmpty()) return result_term;
            node->right = move(convert_node<bintype>(result_term->node));
        }

        return parse_result<unique_ptr<NodeBinOp>>(move(node));
    }

    unique_ptr<ParseResult> parse_expr() {
        unique_ptr<NodeBinOp> node = make_unique<NodeBinOp>();
        unique_ptr<ParseResult> result_expr = move(parse_term());

        if (!result_expr->error.isEmpty()) return result_expr;
        node->left = move(convert_node<bintype>(result_expr->node));

        while (is_tok_type(toktype::plus) || is_tok_type(toktype::minus)) {
            Token_ op_tok = current_tok;
            advance();
            node->op_tok = op_tok;

            result_expr = move(parse_term());
            if (!result_expr->error.isEmpty()) return result_expr;
            node->right = move(convert_node<bintype>(result_expr->node));
        }

        return parse_result<unique_ptr<NodeBinOp>>(move(node));
    }

    unique_ptr<ParseResult> parse_exec() {
        unique_ptr<ParseResult> result_exec = make_unique<ParseResult>();
        unique_ptr<NodeExec> node = make_unique<NodeExec>();
        variant<unique_ptr<NodeVarAccess>, unique_ptr<NodeClassBuiltIn>> executed;

        if (is_tok_type(toktype::name)) {
            Token_ name = tokens[idx++];
            std::optional<unique_ptr<ParseResult>> temp = move(check_error(toktype::exc_mark));
            if (temp.has_value()) return move(temp.value());
            executed = make_unique<NodeVarAccess>();
            get<unique_ptr<NodeVarAccess>>(executed)->var_name_tok = name;
            node->executed = move(executed);
            result_exec->node = move(node);
            return move(result_exec);
        }
        else if (is_tok_type(toktype::keyword) && tokens[idx+1].type == toktype::left_paren) {
            executed = parse_class_builtin();
            std::optional<unique_ptr<ParseResult>> temp = move(check_error(toktype::exc_mark));
            if (temp.has_value()) return move(temp.value());
            node->executed = move(executed);
            result_exec->node = move(node);
            return move(result_exec);
        }

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

    inline bool is_tok_type(toktype type) const {
        return current_tok.type == type;
    }

    inline bool is_tok(toktype type, const string& value) const {
        return current_tok.type == type && current_tok.value == value;
    }

    inline std::optional<unique_ptr<ParseResult>> check_error(toktype type) {
        unique_ptr<ParseResult> result = make_unique<ParseResult>();
        if (!is_tok_type(type)) {
            ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN); // temporary
            result->error = ErrorSyntax(parse_position, std::string("EXPECTED'") + toktype_to_string(type) + "' BUT GOT: '" + current_tok.to_string() + std::string("'"));
            return move(result);
        }
        advance();
        return std::nullopt;
    }

    template <typename T>
    inline unique_ptr<ParseResult> parse_result(T node) {
        unique_ptr<ParseResult> result = make_unique<ParseResult>();
        result->node = move(node);
        return result;
    }

    
    template <typename variantT, size_t Index = 0>
    variantT convert_node_impl(const anyNode& value) {
        constexpr size_t variant_size = std::variant_size_v<variantT>;

        if constexpr (Index < variant_size) {
            using typeT = typename std::variant_alternative<Index, variantT>::type;
            if constexpr (std::is_same_v<typeT, std::decay_t<decltype(value)>>) {
                return std::get<Index>(value);  // Return the value wrapped in variant
            } else {
                return convert_node_impl<variantT, Index + 1>(value);  // Recurse to next index
            }
        } else {
            throw std::runtime_error("Invalid token type in parse_term");
        }
    }

    template <typename variantT>
    variantT convert_node(const anyNode& nodeAssigned) {
        return convert_node_impl<variantT>(nodeAssigned);  // Start recursion from index 0
    }
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