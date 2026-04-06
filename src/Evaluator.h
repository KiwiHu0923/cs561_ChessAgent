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
    static int countEnemyPrinceEscapeSquares(const Board& board, bool princeIsWhite);

    // Piece values
    static const std::array<int, 128> PIECE_VALUES;

private:
    // Sub-evaluation functions
    static double evaluatePrinceSafety(const Board& board, bool isWhite);
    static double evaluatePrincePressure(const Board& board, bool isWhite);
    static double evaluateMaterial(const Board& board, bool isWhite);
    static double evaluatePosition(const Board& board, bool isWhite);
    static int countAttackersNearEnemyPrince(const Board& board, bool attackerIsWhite, int pr, int pc);
    static int pressurePieceWeight(char piece);
};

#endif // EVALUATOR_H
