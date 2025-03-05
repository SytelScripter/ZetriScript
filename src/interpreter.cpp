#include "parser.cpp"

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

    inline Position convertNode(NodePositionAccess node_pos) {
        Position result(node_pos.x.numTok.value, node_pos.y.numTok.value, node_pos.z.numTok.value);
        return result;
    }

    public:
    Interpreter(NodeProg program_) : program(program_) {
        startingPos.posX = std::move(program.startingPosition.x.numTok.value);
        startingPos.posY = std::move(program.startingPosition.y.numTok.value);
        startingPos.posZ = std::move(program.startingPosition.z.numTok.value);
    }

    NodeProg visitProg() {
        int i = 0;
        while (!comparePos(startingPos, convertNode(program.code[i].pos))) {
            i++;
        }

    }
}