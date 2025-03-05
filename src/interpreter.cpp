#include "parser.cpp"
#include <string>

using finalOutput = std::variant<int, float>;

using possibleReturnValues = std::optional<std::vector<Position>>;

class SymbolTable {
    public:
    std::unordered_map<Token, possibleReturnValues> variables;

    void setValue(Token ident, possibleReturnValues value) {
        variables[ident] = value;
    }

    possibleReturnValues getValue(Token ident) {
        if (!variables.find(ident)) {
            return std::nullopt;
        }
        return variables[ident];
    }
}

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

    inline Position convertPositionNode(NodePositionAccess node_pos) {
        Position result = Position(node_pos.x.numTok.value, node_pos.y.numTok.value, node_pos.z.numTok.value);
        return result;
    }

    inline bool hasNoDecimal(float num) {
        return num == static_cast<int>(num);
    }

    public:
    Interpreter(NodeProg program_) : program(program_) {
        startingPos.posX = std::move(program.startingPosition.x.numTok.value);
        startingPos.posY = std::move(program.startingPosition.y.numTok.value);
        startingPos.posZ = std::move(program.startingPosition.z.numTok.value);
    }

    void visitGoto(NodeGoto goto_) {
        Position nextPosition = convertPositionNode(goto_.nextPos);
        bool wentToBeginning = false;
        while (!comparePos(nextPosition, convertPositionNode(program.code[i].pos))) {
            if (currentInstruction == 0) {
                wentToBeginning = true;
            }
            if (!wentToBeginning) {
                currentInstruction--;
            } else {
                currentInstruction++;
            }
            if (currentInstruction >= program.code.size() || currentInstruction < 0) {
                std::cout << "ERROR: NO SUCH INSTRUCTION: [" << nextPosition.x << ":" + nextPosition.y << ":" << nextPosition.z << "]\n";
                return;
            }
        }
        visitSegment(program.code[i]);
    }

    possibleReturnValues visitLine(NodeLine node) {
        possibleReturnValues result;

        int start = std::stoi(node.start);
        int end = std::stoi(node.end);
        int step = std::stoi(node.step);
        Position currentPos = convertPositionNode(node.pos1);
        Position vectorPos = convertPositionNode(node.pos2);
        int startX = std::stoi(currentPos.xPos);
        int startY = std::stoi(currentPos.yPos);
        int startZ = std::stoi(currentPos.zPos);
        int vecX = std::stoi(vectorPos.xPos);
        int vecY = std::stoi(vectorPos.yPos);
        int vecZ = std::stoi(vectorPos.zPos);
        int x = startX;
        int y = startY;
        int z = startZ;
        for (int t = start; t < end; t += step) {
            Position temp = Position(std::to_string(x), std::to_string(y), std::to_string(z));
            result.value().push_back(Position(x, y, z));
            x = startX + t * vecX;
            y = startY + t * vecY;
            z = startZ + t * vecZ;
        }
        return result;
    }

    possibleReturnValues visitVarAccess(NodeVarAccess node) {
        possibleReturnValues result = globals.getValue();
        if (!result.has_value()) {
            std::cout << "ERROR: UNDEFINED VARIABLE" << ident.value << "\n";
            return std::nullopt;
        }
        return result;
    }

    possibleReturnValues visitAlloc(NodeAlloc node) {
        return visitVarAccess(node.allocated);
    }

    void visitVarAssign(NodeVarAssign var_assign) {
        possibleReturnValues result;
        if (std::holds_alternative<NodeLine>(var_assign.value))
            result = visitLine(std::get<NodeLine>(var_assign.value));
        else if (std::holds_alternative<NodeVarAccess>(var_assign.value))
            result = visitVarAccess(std::get<NodeVarAccess>(var_assign.value));
        else if (std::holds_alternative<NodeAlloc>(var_assign.value))
            result = visitAlloc(std::get<NodeAlloc>(var_assign.value));
        globals.setValue(var_assign.ident.identName, result);
    }

    void visitSegment(NodeSegment segment) {
        std::vector<finalOutput> segment_result;
        instruction_pos = 0;
        while (instruction_pos < segment.content.size()) {
            auto instruction = segment.content[instruction_pos];
            if (std::holds_alternative<NodeAlloc>(instruction)) {
                visitAlloc(std::get<NodeAlloc>(instruction));
                // segment_result.push_back(visitAlloc(std::get<NodeAlloc>(instruction)));
                instruction_pos++;
            }
            else if (std::holds_alternative<NodeVarAssign>(instruction)) {
                visitVarAssign(std::get<NodeVarAssign>(instruction));
                // segment_result.push_back(visitVarAssign(std::get<NodeVarAssign>(instruction)));
                instruction_pos++;
            }
            else if (std::holds_alternative<NodeLine>(instruction)) {
                visitLine(std::get<NodeLine>(instruction))
                // segment_result.push_back(visitLine(std::get<NodeLine>(instruction)));
                instruction_pos++;
            }
            else if (std::holds_alternative<NodeVarAccess>(instruction)) {
                visitIdentifier(std::get<NodeVarAccess>(instruction))
                // segment_result.push_back(visitIdentifier(std::get<NodeVarAccess>(instruction)));
                instruction_pos++;
            }
            else if (std::holds_alternative<NodeNumber>(instruction)) {
                visitNumber(std::get<NodeNumber>(instruction))
                // segment_result.push_back(visitNumber(std::get<NodeNumber>(instruction)));
                instruction_pos++;
            }
            else if (std::holds_alternative<NodeGoto>(instruction)) {
                visitGoto(std::get<NodeGoto>(instruction))
                // segment_result.push_back(visitGoto(std::get<NodeGoto>(instruction)));
                instruction_pos++;
            }
        }
    }

    void visitProg() {
        while (!comparePos(startingPos, convertPositionNode(program.code[i].pos))) {
            currentInstruction++;
            if (currentInstruction >= program.code.size()) {
                std::cout << "ERROR: NO SUCH INSTRUCTION: [" << startingPos.x << ":" + startingPos.y << ":" << startingPos.z << "]\n";
                return;
            }
        }
        visitSegment(program.code[i]);
    }
}