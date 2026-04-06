#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "Board.h"
#include <array>

class Evaluator {
public:
    // Main evaluation function
    static double evaluate(const Board& board, bool isWhite);

    // Prince threat detection (public for use in Search)
    static bool isPrinceUnderThreat(const Board& board, bool isWhite);

    // Piece values
    static const std::array<int, 128> PIECE_VALUES;

private:
    // Sub-evaluation functions
    static double evaluatePrinceSafety(const Board& board, bool isWhite);
    static double evaluateMaterial(const Board& board, bool isWhite);
    static double evaluatePosition(const Board& board, bool isWhite);
};

#endif // EVALUATOR_H
