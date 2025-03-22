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
struct NodeParam;
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
using expr_node = variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>>

struct NodeNumber {
    Token_ num_tok;

    NodeNumber(const Token_ &tok) : num_tok(tok) {}
};

struct NodeVarAccess {
    Token_ var_tok;

    NodeVarAccess(const Token_ &tok) : var_tok(tok) {}
};

struct NodeParam {
    Token_ param_tok;

    NodeParam(const Token_ &tok) : param_tok(tok) {}
};

struct NodeBinOp {
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> left;
    Token_ op_tok;
    variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>> right;

    NodeBinOp(expr_node left_, const Token_ &op_tok_, expr_node right_) : left(move(left_)), op_tok(op_tok_), right(move(right_)) {}
};

struct NodePosition {
    Token_ x;
    Token_ y;
    Token_ z;

    NodePosition(const Token_ &x_, const Token_ &y_, const Token_ &z_) : x(x_), y(y_), z(z_) {}
};

struct NodeAllocation {
    unique_ptr<NodePosition> pos;
    vector<unique_ptr<NodeStmt>> body;

    NodeAllocation(unique_ptr<NodePosition> pos_, vector<unique_ptr<NodeStmt>> body_) : pos(move(pos_)), body(move(body_)) {}
};

struct NodeFunction {
    Token_ func_tok;
    vector<unique_ptr<NodeStmt>> body;

    NodeFunction(const Token_ &func_tok_, vector<unique_ptr<NodeStmt>> body_) : func_tok(func_tok_), body(move(body_)) {}
};

struct NodeCommand {
    Token_ sys_tok;
    vector<unique_ptr<NodeParam>> params;
    vector<unique_ptr<NodeStmt>> body;

    NodeCommand(const Token_ &sys_tok_, vector<unique_ptr<NodeParam>> params_, vector<unique_ptr<NodeStmt>> body_) : sys_tok(sys_tok_), params(move(params_)), body(move(body_)) {}
};

struct NodeSystem {
    Token_ sys_tok;
    vector<unique_ptr<NodeParam>> params;
    vector<unique_ptr<NodeCommand>> body;

    NodeSystem(const Token_ &sys_tok_, vector<unique_ptr<NodeParam>> params_, vector<unique_ptr<NodeCommand>> body_) : sys_tok(sys_tok_), params(move(params_)), body(move(body_)) {}
};

struct NodeClassBuiltIn {
    Token_ class_tok;
    Token_ usage;
    vector<unique_ptr<NodeBinOp>> params;

    NodeClassBuiltIn(const Token_ &class_tok_, const Token_ &usage_, vector<unique_ptr<NodeBinOp>> params_) : class_tok(class_tok_), usage(usage_), params(move(params_)) {}
};

struct NodeMethodAccess {
    Token_ method_tok;
    Token_ class_tok;

    NodeMethodAccess(const Token_ &method_tok_, const Token_ &class_tok_) : method_tok(method_tok_), class_tok(class_tok_) {}
};

struct NodeVarAssign {
    Token_ var_tok;
    unique_ptr<NodeBinOp> value;

    NodeVarAssign(const Token_ &var_tok_, unique_ptr<NodeBinOp> value_) : var_tok(var_tok_), value(move(value_)) {}
};

struct NodeStmt {
    Token_ tok;
    variant<unique_ptr<NodeVarAccess>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAssign>, unique_ptr<NodeSystem>, unique_ptr<NodeCommand>, unique_ptr<NodeAllocation>, unique_ptr<NodeFunction>, unique_ptr<NodeMethodAccess>> stmt;

    NodeStmt(const Token_ &tok_, variant<unique_ptr<NodeVarAccess>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAssign>, unique_ptr<NodeSystem>, unique_ptr<NodeCommand>, unique_ptr<NodeAllocation>, unique_ptr<NodeFunction>, unique_ptr<NodeMethodAccess>> stmt_) : tok(tok_), stmt(move(stmt_)) {}
};

struct NodeProg {
    vector<unique_ptr<NodeStmt>> stmts;

    NodeProg(vector<unique_ptr<NodeStmt>> stmts_) : stmts(move(stmts_)) {}
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
            unique_ptr<NodeNumber> result = make_unique<NodeNumber>(current_tok);
            advance();
            return parse_result<unique_ptr<NodeNumber>>(move(result));
        } else if (is_tok_type(toktype::name)) {
            unique_ptr<NodeVarAccess> result = make_unique<NodeVarAccess>(current_tok);
            advance();
            return parse_result<unique_ptr<NodeVarAccess>>(move(result));
        } else if (is_tok_type(toktype::left_paren)) {
            advance();
            unique_ptr<ParseResult> result = parse_expr();
            if (!is_tok_type(toktype::right_paren)) {
                ErrorSyntax error = ErrorSyntax(current_tok, "Expected ')'");
                return make_unique<ParseResult>(nullptr, error);
            }
            advance();
            return parse_result<unique_ptr<NodeBinOp>>(move(result->node), current_tok, parse_factor()->node);
        }
        ErrorSyntax error = ErrorSyntax(current_tok, std::string("Expected factor"));
        return make_unique<ParseResult>(nullptr, error);
    }

    unique_ptr<ParseResult> parse_term() {
        unique_ptr<ParseResult> result = parse_factor();
        if (!result.error.isEmpty()) return result;
        while (is_tok_type(toktype::mul) || is_tok_type(toktype::div)) {
            advance();
            unique_ptr<ParseResult> next_result = parse_factor();
            if (!next_result.error.isEmpty()) return next_result;
            result->node = make_unique<NodeBinOp>(move(result->node), current_tok, move(next_result->node));
        }
        return result;
    }

    unique_ptr<ParseResult> parse_expr() {
        unique_ptr<ParseResult> result = parse_term();
        if (!result.error.isEmpty()) return result;
        while (is_tok_type(toktype::plus) || is_tok_type(toktype::minus)) {
            advance();
            unique_ptr<ParseResult> next_result = parse_term();
            if (!next_result.error.isEmpty()) return next_result;
            result->node = make_unique<NodeBinOp>(move(result->node), current_tok, move(next_result->node));
        }
        return result;
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
