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
struct NodePosition;
struct NodeAllocation;
struct NodeFunction;
struct NodeSystem;
struct NodeCommand;
struct NodeClassBuiltIn;
struct NodeMethodAccess;
struct NodeVarAccess;
struct NodeVarAssign;
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
    unique_ptr<NodeNumber>,
    unique_ptr<NodeBinOp>,
    unique_ptr<NodePosition>,
    unique_ptr<NodeAllocation>,
    unique_ptr<NodeFunction>,
    unique_ptr<NodeSystem>,
    unique_ptr<NodeCommand>,
    unique_ptr<NodeClassBuiltIn>,
    unique_ptr<NodeMethodAccess>,
    unique_ptr<NodeVarAccess>,
    unique_ptr<NodeVarAssign>,
    unique_ptr<NodeStmt>,
    unique_ptr<NodeProg>
>


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

    ParseResult parse_factor() {
        if (is_tok_type(toktype::int_lit) || is_tok_type(toktype::float_lit)) {
            ParsePosition parse_position = ParsePosition(current_tok.pos);
            unique_ptr<NodeNumber> node = make_unique<NodeNumber>(current_tok.value);
            advance();
            return parse_result(move(node));
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
