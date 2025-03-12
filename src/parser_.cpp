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
    variant<unique_ptr<NodeVarAccess>, unique_ptr<NodePosAccess>, unique_ptr<NodeClassBuiltIn>> executed;
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
    variant<bintype> x;
    variant<bintype> y;
    variant<bintype> z;
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
};

class Parser {
    public:
    inline explicit Parser(const vector<Token_>& tokens_) : 
        tokens(move(tokens_)) {
            advance();
        }

    unique_ptr<ParseResult> parse_position() {
        unique_ptr<ParseResult> result_position = make_unique<ParseResult>();
        if (!is_tok_type(toktype::left_square)) {
            if (!is_tok_type(toktype::int_lit) && !is_tok_type(toktype::float_lit)) {
                ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN); // temporary
                result_position->error = ErrorSyntax(parse_position, std::string("EXPECTED '['"));
                return move(result_position);
            }
            unique_ptr<ParseResult> node = move(parse_expr());
            return move(node);
        }
        advance(); // '['
        unique_ptr<NodePosAccess> node = make_unique<NodePosAccess>();
        unique_ptr<ParseResult> xResult = move(parse_expr());
        if (!xResult->error.isEmpty()) return move(xResult);
        node->x = move(convert_node<bintype>(xResult->node));
        std::optional<unique_ptr<ParseResult>> temp1 = check_error(toktype::comma);
        if (temp1.has_value()) return move(temp1.value());
        advance(); // ','
        unique_ptr<ParseResult> yResult = move(parse_expr());
        if (!yResult->error.isEmpty()) return move(yResult);
        node->y = move(convert_node<bintype>(yResult->node));
        std::optional<unique_ptr<ParseResult>> temp2 = check_error(toktype::comma);
        if (temp2.has_value()) return move(temp2.value());
        unique_ptr<ParseResult> zResult = move(parse_expr());
        if (!zResult->error.isEmpty()) return move(zResult);
        node->z = move(convert_node<bintype>(zResult->node));
        std::optional<unique_ptr<ParseResult>> temp3 = check_error(toktype::right_square);
        if (temp3.has_value()) return move(temp3.value());
        advance(); // ']'
        result_position->node = move(node);
        return move(result_position);
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

    unique_ptr<ParseResult> parse_class_builtin() {
        unique_ptr<ParseResult> result_class_builtin = make_unique<ParseResult>();
        unique_ptr<NodeClassBuiltIn> node = make_unique<NodeClassBuiltIn>();
        if (!is_tok_type(toktype::keyword)) {
            ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN);
            result_class_builtin->error = ErrorSyntax(parse_position, "EXPECTED 'class'");
            return move(result_class_builtin);
        }
        Token_ class_name = tokens[idx++];
        std::optional<unique_ptr<ParseResult>> temp = move(check_error(toktype::left_paren));
        if (temp.has_value()) return move(temp.value());
        advance(); // '('

        vector<variant<unique_ptr<ParsePosition>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>>> args;
        while (idx < tokens.size() && !is_tok_type(toktype::right_paren)) {
            unique_ptr<ParseResult> parse_result = move(parse_position());
            if (!parse_result->error.isEmpty()) return move(parse_result);
            args.push_back(move(convert_node<variant<unique_ptr<ParsePosition>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>>>(parse_result->node)));
            if (idx < tokens.size() && is_tok_type(toktype::comma)) {
                advance();
            }
        }

        node->class_name_tok = class_name;
        node->args = move(args);
        result_class_builtin->node = move(node);
        return move(result_class_builtin);
    }

    unique_ptr<ParseResult> parse_exec() {
        unique_ptr<ParseResult> result_exec = make_unique<ParseResult>();
        unique_ptr<NodeExec> node = make_unique<NodeExec>();
        variant<unique_ptr<NodeVarAccess>, unique_ptr<NodePosAccess>, unique_ptr<NodeClassBuiltIn>> executed;

        if (!is_tok(toktype::keyword, "goto")) {
            unique_ptr<ParseResult> expr_result = move(parse_expr());
            return move(expre_result);
        }
        advance(); // 'goto'

        if (is_tok_type(toktype::left_square)) {
            unique_ptr<ParseResult> pos_result = move(parse_pos());
            if (!pos_result->error.isEmpty()) return move(pos_result);

            std::optional<unique_ptr<ParseResult>> temp = move(check_error(toktype::exc_mark));
            if (temp.has_value()) return move(temp.value());
            idx--;

            executed = move(get<unique_ptr<NodePosAccess>>(pos_result->node));

            node->executed = move(executed);
            result_exec->node = move(node);
            return move(result_exec);
        }
        else if (is_tok_type(toktype::name)) {
            Token_ name = tokens[idx++];
            
            std::optional<unique_ptr<ParseResult>> temp = move(check_error(toktype::exc_mark));
            if (temp.has_value()) return move(temp.value());
            idx--;

            executed = make_unique<NodeVarAccess>();
            get<unique_ptr<NodeVarAccess>>(executed)->var_name_tok = name;

            node->executed = move(executed);
            result_exec->node = move(node);
            return move(result_exec);
        }
        else if (is_tok_type(toktype::keyword) && tokens[idx+1].type == toktype::left_paren) {
            unique_ptr<ParseResult> class_result = move(parse_class_builtin());

            std::optional<unique_ptr<ParseResult>> temp = move(check_error(toktype::exc_mark));
            if (temp.has_value()) return move(temp.value());
            idx--;

            executed = move(get<unique_ptr<NodeClassBuiltIn>>(class_result->node));
            
            node->executed = move(executed);
            result_exec->node = move(node);
            return move(result_exec);
        }
        else {
            ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN); // temporary
            result_exec->error = ErrorSyntax(parse_position, std::string("EXPECTED NAME OR 'goto' OR 'class builtin', BUT GOT: '") + current_tok.to_string() + std::string("'"));
            return move(result_exec);
        }
    }

    unique_ptr<ParseResult> parse_var_assign() {
        unique_ptr<ParseResult> result_var_assign = make_unique<ParseResult>();
        unique_ptr<NodeVarAssign> node = make_unique<NodeVarAssign>();
        if (!is_tok_type(toktype::name)) {
            unique_ptr<ParseResult> result_exec = move(parse_exec());
            return move(result_exec);
        }
        Token_ name = tokens[idx++];
        std::optional<unique_ptr<ParseResult>> temp = move(check_error(toktype::equal));
        if (temp.has_value()) return move(temp.value());
        
        variant<unique_ptr<NodeExec>, unique_ptr<NodeClassBuiltIn>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>> value;
        unique_ptr<ParseResult> parse_result = move(parse_exec());
        if (!parse_result->error.isEmpty()) return move(parse_result);

        value = move(convert_node<variant<unique_ptr<NodeExec>, unique_ptr<NodeClassBuiltIn>, unique_ptr<NodeBinOp>, unique_ptr<NodeNumber>, unique_ptr<NodeVarAccess>>>(parse_result->node));
        node->var_name_tok = name;
        node->value = move(value);
        result_var_assign->node = move(node);
        return move(result_var_assign);
    }

    unique_ptr<ParseResult> parse_stmt() {
        unique_ptr<ParseResult> result_stmt = make_unique<ParseResult>();
        unique_ptr<NodeStmt> node = make_unique<NodeStmt>();
        unique_ptr<NodePosAccess> pos_access = move(parse_position());
        if (!pos_access->error.isEmpty()) return move(pos_access);
        node->pos = move(pos_access);

        while (!is_tok_type(toktype::exc_mark)) {
            unique_ptr<ParseResult> var_assign_result = move(parse_var_assign());
            if (!var_assign_result->error.isEmpty()) return move(var_assign_result);
            node->stmts.push_back(move(var_assign_result->node));

            if (is_tok_type(toktype::semi_colon)) {
                advance();
            }
            else {
                ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN); // temporary
                result_stmt->error = ErrorSyntax(parse_position, std::string("EXPECTED ';' BUT GOT: '") + current_tok.to_string() + std::string("'"));
                return move(result_stmt);
            }
        }
        std::optional<unique_ptr<ParseResult>> temp = move(check_error(toktype::semi_colon));
    }

    unique_ptr<ParseResult> parse_program() {
        unique_ptr<ParseResult> result_program = make_unique<ParseResult>();

        unique_ptr<NodeProg> node = make_unique<NodeProg>();
        if (!is_tok_type(toktype::keyword, "ZetriScript")) {
            ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN); // temporary
            node->error = ErrorSyntax(parse_position, "EXPECTED 'ZetriScript' BUT GOT: '" + current_tok.to_string() + "'");
            return move(node);
        }
        advance(); // 'ZetriScript'
        unique_ptr<ParseResult> entry_pos_result = move(parse_position());
        if (!entry_pos_result->error.isEmpty()) return move(entry_pos_result);
        node->entry_pos = move(get<unique_ptr<NodePosAccess>>(entry_pos_result->node));

        if (!is_tok_type(toktype::exc_mark)) {
            ParsePosition parse_position = ParsePosition(specialpos::UNKNOWN); // temporary
            node->error = ErrorSyntax(parse_position, "EXPECTED END OF FILE BUT GOT: '" + current_tok.to_string() + "'");
            return move(node);
        }
        advance(); // '!'

        while (!is_tok_type(toktype::eof_)) {
            unique_ptr<ParseResult> stmt_result = move(parse_stmt());
            if (!stmt_result->error.isEmpty()) return move(stmt_result);
            node->stmts.push_back(move(stmt_result->node));
        }

        result_program->node = move(node);
        return move(result_program);
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