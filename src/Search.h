#ifndef SEARCH_H
#define SEARCH_H

#include "GameState.h"
#include "Move.h"
#include "Evaluator.h"
#include <vector>
#include <utility>
#include <string>
#include <unordered_map>
#include <cstdint>

class Search {
public:
    Search(const GameState& state);
    Move selectBestMove();

private:
    struct TTEntry {
        double score;
        int depth;
        uint8_t flag;
        Move bestMove;
    };

    static constexpr uint8_t TT_EXACT = 0;
    static constexpr uint8_t TT_LOWER = 1;
    static constexpr uint8_t TT_UPPER = 2;

    GameState state;
    double startTime;
    double timeLimit;
    bool timeout;
    std::unordered_map<uint64_t, TTEntry> transpositionTable;
    std::unordered_map<uint64_t, int> currentLineCounts;
    std::unordered_map<uint64_t, int> historicalResultCounts;
    std::string repetitionHistoryPath;

    // Minimax with alpha-beta pruning
    std::pair<double, Move> minimax(Board& board, int depth, double alpha, double beta,
                                     bool maximizing, bool isWhite);

    // Move ordering for better pruning
    std::vector<Move> orderMoves(Board& board, const std::vector<Move>& moves, bool isWhite,
                                 const Move& preferredMove = Move());
    double moveScore(Board& board, const Move& move, bool isWhite, const Move& preferredMove);

    // Time management
    double getCPUTime() const;
    void checkTimeout();
    double calculateTimeLimit(int piecesCount, double myTime);

    // Position identity and repetition handling
    uint64_t positionKey(const Board& board, bool whiteToMove) const;
    void loadRepetitionHistory();
    void saveRepetitionHistory(uint64_t resultKey);
};

#endif // SEARCH_H
