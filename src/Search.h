#ifndef SEARCH_H
#define SEARCH_H

#include "GameState.h"
#include "Move.h"
#include "Evaluator.h"
#include <vector>
#include <utility>

class Search {
public:
    Search(const GameState& state);
    Move selectBestMove();

private:
    GameState state;
    double startTime;
    double timeLimit;
    bool timeout;

    // Minimax with alpha-beta pruning
    std::pair<double, Move> minimax(Board& board, int depth, double alpha, double beta,
                                     bool maximizing, bool isWhite);

    // Move ordering for better pruning
    std::vector<Move> orderMoves(const Board& board, const std::vector<Move>& moves, bool isWhite);
    double moveScore(const Board& board, const Move& move, bool isWhite);

    // Time management
    double getCPUTime() const;
    void checkTimeout();
    double calculateTimeLimit(int piecesCount, double myTime);
};

#endif // SEARCH_H
