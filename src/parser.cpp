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
struct NodeVarAccess;
struct NodeNodeParam;
struct NodeBinOp;
struct NodePosition;
struct NodeMethodAccess;
struct NodeClassBuiltIn;
struct NodeFunction;
struct NodeCommand;
struct NodeSystem;
struct NodeVarAssign;
struct NodeAllocation;
struct NodeStmt;
struct NodeProg;

// usings
using std::variant, 
    std::get, 
    std::vector, 
    std::unique_ptr, 
    std::make_unique, 
    std::shared_ptr,
    std::move, 
    std::string, 
    std::function, 
    std::holds_alternative, 
    std::visit;
using anyNode = variant<
    unique_ptr<NodeNumber>, 
    unique_ptr<NodeVarAccess>, 
    unique_ptr<NodeParam>, 
    unique_ptr<NodeBinOp>, 
    unique_ptr<NodePosition>, 
    unique_ptr<NodeMethodAccess>, 
    unique_ptr<NodeClassBuiltIn>, 
    unique_ptr<NodeFunction>, 
    unique_ptr<NodeCommand>, 
    unique_ptr<NodeSystem>, 
    unique_ptr<NodeVarAssign>, 
    unique_ptr<NodeAllocation>, 
    unique_ptr<NodeStmt>, 
    unique_ptr<NodeProg>
>;

struct NodeNumber {
    Token_ num_tok;
};

struct NodeVarAccess {
    Token_ var_tok;
};

struct NodeParam {
    Token_ param_tok;
};

struct NodeBinOp {
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> left;
    Token_ op_tok;
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> right;
};

struct NodePosition {
    Token_ x;
    Token_ y;
    Token_ z;
};

struct NodeAllocation {
    NodePosition pos;
    vector<unique_ptr<NodeStmt>> body;
};

struct NodeFunction {
    Token_ func_tok;
    vector<unique_ptr<NodeStmt>> body;
};

struct NodeSystem {
    Token_ sys_tok;
    vector<unique_ptr<NodeParam>> params;
    vector<unique_ptr<NodeCommand>> body;
};

struct NodeCommand {
    Token_ sys_tok;
    vector<unique_ptr<NodeParam>> params;
    vector<unique_ptr<NodeStmt>> body;
};

struct NodeClassBuiltIn {
    Token_ class_tok;
    Token_ usage;
    vector<unique_ptr<NodeBinOp>> params;
};

struct NodeMethodAccess {
    Token_ method_tok;
    Token_ class_tok;
};

struct NodeVarAssign {
    Token_ var_tok;
    unique_ptr<NodeBinOp> value;
};

struct NodeStmt {
    Token_ tok;
    variant<unique_ptr<NodeVarAccess>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAssign>, unique_ptr<NodeSystem>, unique_ptr<NodeCommand>, unique_ptr<NodeAllocation>, unique_ptr<NodeFunction>, unique_ptr<NodeMethodAccess>> stmt;
};

struct NodeProg {
    vector<unique_ptr<NodeStmt>> statements;
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
        if (is_tok_type(toktype::int_lit) || is_tok_type(toktype::float_lit)) {
            ParsePosition parse_position = ParsePosition(current_tok.pos);
            unique_ptr<NodeNumber> node = make_unique<NodeNumber>();
            node->num_tok = current_tok;
            advance();
            return parse_result(move(node));
        }

        else if (is_tok_type(toktype::name)) {
            ParsePosition parse_position = ParsePosition(current_tok.pos);
            unique_ptr<NodeVarAccess> node = make_unique<NodeVarAccess>();
            node->var_tok = current_tok;
            advance();
            return parse_result(move(node));
        }

        else if (is_tok(toktype::left_paren)) {
            advance();
            ParseResult parse_expr = parse_expr();
            if (!parse_expr.error.isEmpty()) return parse_expr;
            if (check_error(toktype::right_paren)) {
                return parse_expr;
            }
            else {
                return ParseResult{nullptr, ErrorSyntax(parse_position, "UNEXPECTED TOKEN AFTER EXPR")};
            }
        }
    }

    unique_ptr<ParseResult> parse_term() {
        ParseResult parse_factor_result = parse_factor();
        if (!parse_factor_result.error.isEmpty()) return parse_factor_result;
        while (is_tok(toktype::mul, "*") || is_tok(toktype::div, "/")) {
            advance();
            ParseResult right_result = parse_factor();
            if (!right_result.error.isEmpty()) return right_result;
            unique_ptr<NodeBinOp> bin_op_node = make_unique<NodeBinOp>();
            bin_op_node->left = move(convert_node<variant<unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>, unique_ptr<NodeBinOp>>>(parse_factor_result.node));
            bin_op_node->op_tok = current_tok;
            bin_op_node->right = move(convert_node<variant<unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>, unique_ptr<NodeBinOp>>>(parse_factor_result.node));
            parse_factor_result->node = move(bin_op_node);
        }

        return parse_factor_result;
    }

    unique_ptr<ParseResult> parse_expr() {
        ParseResult parse_term_result = parse_term();
        if (!parse_term_result.error.isEmpty()) return parse_term_result;
        while (is_tok(toktype::plus, "+") || is_tok(toktype::minus, "-")) {
            advance();
            ParseResult right_result = parse_term();
            if (!right_result.error.isEmpty()) return right_result;
            unique_ptr<NodeBinOp> bin_op_node = make_unique<NodeBinOp>();
            bin_op_node->left = move(convert_node<variant<unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>, unique_ptr<NodeBinOp>>>(parse_term_result.node));
            bin_op_node->op_tok = current_tok;
            bin_op_node->right = move(convert_node<variant<unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>, unique_ptr<NodeBinOp>>>(parse_term_result.node));
            parse_term_result.node = move(bin_op_node);
        }

        return parse_term_result;
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
