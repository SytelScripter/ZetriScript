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
using expr_node = variant<unique_ptr<NodeNumber>, unique_ptr<NodeBinOp>, unique_ptr<NodeVarAccess>>;

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

    ParseResult(const ErrorSyntax& error_) : error(error_) {}
    ParseResult(anyNode node_) : node(move(node_)) {}

    inline bool hasError() {
        return error.has_value();
    }
};

class Parser {
    public:
    inline explicit Parser(const vector<Token_>& tokens_) : 
        tokens(move(tokens_)) {
            advance();
    }

    unique_ptr<ParseResult> parse_factor() {
        if (cur_tok.type == toktype::int_lit || cur_tok.type == toktype::float_lit) {
            Token_ tok = cur_tok;
            advance();
            return returnNode<NodeNumber>(cur_tok);
        }
        if (cur_tok.type == toktype::name) {
            Token_ tok = cur_tok;
            advance();
            return returnNode<NodeVarAccess>(cur_tok);
        }
    }

    unique_ptr<ParseResult> parse_term() {
        unique_ptr<ParseResult> factor_res = move(parse_factor());
        if (factor_res.hasError()) return factor_res;
        while (cur_tok.type == toktype::mul || cur_tok.type == toktype::div) {
            Token_ op_tok = cur_tok;
            advance();
            unique_ptr<ParseResult> right_res = move(parse_factor());
            if (right_res.hasError()) return right_res;
            factor_res = returnNode<NodeBinOp>(
                convertNode(std::move(factor_res->node)),
                op_tok,
                convertNode(std::move(right_res->node))
            );
        }
        return factor_res;
    }

    private:
    Token_ cur_tok;
    vector<Token_> tokens;
    int idx = -1;
    inline void advance() {
        idx++;
        if (idx < tokens.size()) {
            cur_tok = tokens[idx];
        } else {
            cur_tok = Token_{"EOF", "", -1, -1};
        }
    }

    template<typename Node>
    inline optional<unique_ptr<Node>> getNode(unique_ptr<ParseResult> input) {
        if (input.)
    }

    template<typename Node, typename... Args>
    inline unique_ptr<ParseResult> returnNode(Args&&... args) {
        unique_ptr<Node> node = make_unique<Node>(std::forward<Args>(args)...);

        return make_unique<ParseResult>(std::forward<Args>(args)...);
    }

    template<typename T>
    inline anyNode convertNode(T node) {
        return std::visit([](auto&& value) -> anyNode {
            return std::move(value);
        }, std::move(node));
    }

    

};
