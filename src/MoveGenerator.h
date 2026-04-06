#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "Board.h"
#include "Move.h"
#include <vector>

class MoveGenerator {
public:
    // Main move generation function
    static std::vector<Move> generateMoves(const Board& board, bool isWhite);

private:
    // Piece-specific move generators
    static void genBabyMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves);
    static void genPrinceMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves);
    static void genPrincessMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves);
    static void genPonyMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves);
    static void genGuardMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves);
    static void genTutorMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves);
    static void genScoutMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves);
    static void genSiblingMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves);

    // Helper functions
    static void rayMoves(const Board& board, int r, int c, bool isWhite,
                        const int directions[][2], int numDirections,
                        int maxSteps, std::vector<Move>& moves);

    static void addMove(const Board& board, int sr, int sc, int dr, int dc,
                       bool isWhite, std::vector<Move>& moves);
};

#endif // MOVEGENERATOR_H
