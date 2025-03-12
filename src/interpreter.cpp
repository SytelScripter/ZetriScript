#include "parser.cpp"
#include <string>
#include <unordered_map>
#include <optional>

using string, std::optional;

class SymbolTable {
    public:
    std::unordered_map<Token_, string> variables;

    void setValue(Token_ ident, string value) {
        variables[ident] = value;
    }

    optional<string> getValue(Token_ ident) {
        if (!variables.find(ident)) {
            return std::nullopt;
        }
        return variables[ident];
    }
};

class Interpreter {
    private:
    NodeProg program;
    Position startingPos;
    SymbolTable globals;
    int currentInstruction = 0;
    int instruction_pos = 0;

    inline bool comparePos(Position pos1, Position pos2) {
        if (pos1.xPos == pos1.xPos && pos1.yPos == pos2.yPos && pos1.zPos == pos2.zPos) {
            return true;
        }
        return false;
    }

    public:
    Interpreter(NodeProg program, Position startingPos) : program(program), startingPos(startingPos) {}

    string visitNodeNumber(unique_ptr<NodeNumber> node) {
        if (node->value.type == toktype::int_lit) {
            return node->num_tok.value;
        }
    }

    string visitNodeVarAccess(unique_ptr<NodeVarAccess> node) {
        optional<string> value = globals.getValue(node->var_name_tok);
        if (value.has_value()) {
            return value.value();
        }
        else {
            throw std::runtime_error("Variable not found: " + node->var_name_tok.value);
        }
    }

    string visitNodeBinOp(unique_ptr<NodeBinOp> node) {
        string left = visitExpr(node->left);
        string right = visitExpr(node->right);
        if (node->op.type == toktype::plus) {
            return std::to_string(std::stoi(left) + std::stoi(right));
        } else if (node->op.type == toktype::minus) {
            return std::to_string(std::stoi(left) - std::stoi(right));
        } else if (node->op.type == toktype::mul) {
            return std::to_string(std::stoi(left) * std::stoi(right));
        } else if (node->op.type == toktype::div) {
            if (std::stoi(right) == 0) {
                throw std::runtime_error("Division by zero");
            }
            return std::to_string(std::stoi(left) / std::stoi(right));
        }
    }

    string visitNodeExec(unique_ptr<NodeExec> node) {
        visit([](auto&& node) -> string {
            using T = std::decay_t<decltype(node)>;
            if (std::is_same_v<T, unique_ptr<NodeVarAccess>>) {
                return globals.getValue(node->var_name_tok);
            }
            else if (std::is_same_v<T, unique_ptr<NodePosAccess>>) {
                return visitNodePosAccess
            }
        }, node->executed)
    }

    ParsePosition visitNodePosAccess(unique_ptr<NodePosAccess> node) {
        // bintype x = move(node->x);
        // bintype y = move(node->y);
        // bintype z = move(node->z);
        string x, y, z;
        if (std::holds_alternative<NodeNumber>(node->x))
            x = visitNodeNumber(node->x);
        else if (std::holds_alternative<NodeBinOp>(node->x))
            x = visitNodeBinOp(node->x);
        else if (std::holds_alternative<NodeVarAccess>(node->x))
            x = visitNodeVarAccess(node->x);

        if (std::holds_alternative<NodeNumber>(node->y))
            y = visitNodeNumber(node->y);
        else if (std::holds_alternative<NodeBinOp>(node->y))
            y = visitNodeBinOp(node->y);
        else if (std::holds_alternative<NodeVarAccess>(node->y))
            y = visitNodeVarAccess(node->y);

        if (std::holds_alternative<NodeNumber>(node->z))
            z = visitNodeNumber(node->z);
        else if (std::holds_alternative<NodeBinOp>(node->z))
            z = visitNodeBinOp(node->z);
        else if (std::holds_alternative<NodeVarAccess>(node->z))
            z = visitNodeVarAccess(node->z);

        return ParsePosition(x, y, z);
    }

    void execute(unique_ptr<NodeProg> program) {
        // first all the positions
        ParsePosition entry_pos = visitNodePosAccess(program->entry_pos);
        std::vector<ParsePosition> positions;
        for (auto&& stmt : porgram->stmts) {
            
            stmt->eval_pos = visitNodePosAccess(move(stmt));

            // for test
            std::cout << stmt->eval_pos.display() << std::endl;
        }
        // then execute the instructions
        

    }
};

int main() {
    // your test cases here
    std::string test = "ZetriScript [0:0:0]!\n
    \n
    [0:0:1]: func1 = allocSpace(l1); goto [0:0:3]!\n
    [0:0:0]: l1 = LINE([0:0:3],[0:0:1],0,5,1); goto [0:0:1]!\n
    [0:0:3]: goto func1!\n
    \n
    ZetriScript";
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

    unique_ptr<NodeProg> program = std::move(result->node);
    Interpreter interpreter(program, ParsePosition(0, 0, 0));
    interpreter.execute(program);

    return 0;
}