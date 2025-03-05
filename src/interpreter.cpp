#include "parser.cpp"

using anynode = std::variant<NodeProg, NodeSegment, NodeExec, NodeLine, NodeAlloc, NodeVarAssign, NodeIdentifier, NodePosition, NodeNumber>;

class Interpreter {
    private:
    NodeProg program;
    Position startingPos;

    inline bool comparePos(Position pos1, Position pos2) {
        if (pos1.posX == pos1.posY && pos1.posY == pos2.posY && pos1.posZ == pos2.posZ) {
            return true;
        }
        return false;
    }

    public:
    Interpreter(NodeProg program_) : program(program_) {
        startingPos.posX = std::move(program.startingPosition.x.value);
        startingPos.posY = std::move(program.startingPosition.y.value);
        startingPos.posZ = std::move(program.startingPosition.z.value);
    }

    void visit(anynode node) {
        auto result = std::visit([](auto&& value) {
            if constexpr(std::is_same_v<std::decay_t<decltype(value)>, NodeProg>) {
                int i = 0;
                while (!comparePos(value.code[i].pos, startingPos)) {
                    i++;
                }
                return visit(value.code[i]);
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(value)>, NodeSegment>) {
                // continue that afterwards
            }
        }, node);
    }
}