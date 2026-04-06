#include "Search.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include <ctime>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <limits>

Search::Search(const GameState& state)
    : state(state), startTime(0), timeLimit(0), timeout(false) {}

double Search::getCPUTime() const {
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

void Search::checkTimeout() {
    if (getCPUTime() - startTime >= timeLimit) {
        timeout = true;
    }
}

double Search::calculateTimeLimit(int piecesCount, double myTime) {
    // Estimate moves remaining based on piece count
    int estimatedMovesLeft;
    if (piecesCount >= 20) {
        estimatedMovesLeft = 40;  // Opening
    } else if (piecesCount >= 12) {
        estimatedMovesLeft = 30;  // Middlegame
    } else {
        estimatedMovesLeft = 20;  // Endgame
    }

    double limit;
    if (myTime > 15) {
        // Plenty of time: aggressive search
        double baseTime = myTime / estimatedMovesLeft;
        limit = std::min(2.0, baseTime * 1.2);
    } else if (myTime > 5) {
        // Moderate time: balanced approach
        double baseTime = myTime / estimatedMovesLeft;
        limit = std::min(1.0, baseTime * 0.9);
    } else if (myTime > 1) {
        // Low time: conservative
        double baseTime = myTime / (estimatedMovesLeft * 1.5);
        limit = std::min(0.3, baseTime);
    } else {
        // Critical: minimal but safe
        limit = myTime / 30;
    }

    // Safety cap: never exceed 8% of remaining time
    limit = std::min(limit, myTime * 0.08);
    // Absolute minimum for meaningful search
    limit = std::max(limit, 0.01);

    return limit;
}

double Search::moveScore(const Board& board, const Move& move, bool isWhite) {
    double score = 0;

    char captured = board.getPiece(move.dr, move.dc);

    // 1. Prince capture (game-ending)
    if (toupper(captured) == 'P') {
        return 1000000;
    }

    // 2. Checking moves (threatens enemy prince)
    Board newBoard = board;
    newBoard.applyMove(move);
    if (Evaluator::isPrinceUnderThreat(newBoard, !isWhite)) {
        score += 100000;
    }

    // 3. Defensive moves (our prince is threatened)
    if (Evaluator::isPrinceUnderThreat(board, isWhite)) {
        // Test if this move stops the threat
        if (!Evaluator::isPrinceUnderThreat(newBoard, isWhite)) {
            score += 50000;
        }
    }

    // 4. MVV-LVA captures
    if (captured != '.') {
        int victimValue = Evaluator::PIECE_VALUES[static_cast<int>(toupper(captured))];
        int attackerValue = Evaluator::PIECE_VALUES[static_cast<int>(toupper(board.getPiece(move.sr, move.sc)))];
        score += victimValue * 10 - attackerValue;
    }

    // 5. Center control
    if (move.dr >= 4 && move.dr <= 7 && move.dc >= 4 && move.dc <= 7) {
        score += 20;
    }

    return score;
}

std::vector<Move> Search::orderMoves(const Board& board, const std::vector<Move>& moves, bool isWhite) {
    std::vector<std::pair<double, Move>> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const Move& move : moves) {
        double score = moveScore(board, move, isWhite);
        scoredMoves.emplace_back(score, move);
    }

    // Sort in descending order (best moves first)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
             [](const auto& a, const auto& b) { return a.first > b.first; });

    std::vector<Move> orderedMoves;
    orderedMoves.reserve(moves.size());
    for (const auto& [score, move] : scoredMoves) {
        orderedMoves.push_back(move);
    }

    return orderedMoves;
}

std::pair<double, Move> Search::minimax(Board& board, int depth, double alpha, double beta,
                                         bool maximizing, bool isWhite) {
    // Timeout check
    checkTimeout();
    if (timeout) {
        return {0, Move()};
    }

    // Terminal or depth limit reached
    if (depth == 0 || board.isTerminal()) {
        return {Evaluator::evaluate(board, isWhite), Move()};
    }

    // Generate moves for the current player at this node
    // When maximizing=true, we're at a max node (root player's turn)
    // When maximizing=false, we're at a min node (opponent's turn)
    bool currentPlayerIsWhite = (maximizing == isWhite);
    std::vector<Move> moves = MoveGenerator::generateMoves(board, currentPlayerIsWhite);

    if (moves.empty()) {
        // No legal moves - treat as loss
        return {maximizing ? -1000000.0 : 1000000.0, Move()};
    }

    // Order moves for better pruning
    moves = orderMoves(board, moves, currentPlayerIsWhite);

    Move bestMove = moves[0];

    if (maximizing) {
        double maxEval = -std::numeric_limits<double>::infinity();

        for (const Move& move : moves) {
            char captured;
            board.makeMove(move, captured);
            auto [evalScore, _] = minimax(board, depth - 1, alpha, beta, false, isWhite);
            board.unmakeMove(move, captured);

            if (timeout) return {0, Move()};

            if (evalScore > maxEval) {
                maxEval = evalScore;
                bestMove = move;
            }

            alpha = std::max(alpha, evalScore);
            if (beta <= alpha) {
                break;  // Beta cutoff
            }
        }

        return {maxEval, bestMove};
    } else {
        double minEval = std::numeric_limits<double>::infinity();

        for (const Move& move : moves) {
            char captured;
            board.makeMove(move, captured);
            auto [evalScore, _] = minimax(board, depth - 1, alpha, beta, true, isWhite);
            board.unmakeMove(move, captured);

            if (timeout) return {0, Move()};

            if (evalScore < minEval) {
                minEval = evalScore;
                bestMove = move;
            }

            beta = std::min(beta, evalScore);
            if (beta <= alpha) {
                break;  // Alpha cutoff
            }
        }

        return {minEval, bestMove};
    }
}

Move Search::selectBestMove() {
    // Generate moves
    std::vector<Move> moves = MoveGenerator::generateMoves(state.getBoard(), state.isWhiteToMove());

    if (moves.empty()) {
        throw std::runtime_error("No legal moves available");
    }

    if (moves.size() == 1) {
        return moves[0];
    }

    // Emergency: if very low time, just return first ordered move
    if (state.getMyTime() < 0.1) {
        moves = orderMoves(state.getBoard(), moves, state.isWhiteToMove());
        return moves[0];
    }

    // Calculate time limit
    int piecesCount = state.getBoard().countPieces();
    timeLimit = calculateTimeLimit(piecesCount, state.getMyTime());

    // Iterative deepening
    startTime = getCPUTime();
    timeout = false;
    Move bestMove = moves[0];
    double prevDepthTime = 0;
    double depthBeforePrev = 0;

    for (int depth = 1; depth <= 20; depth++) {
        double depthStartTime = getCPUTime();
        double timeUsed = depthStartTime - startTime;
        double timeRemaining = timeLimit - timeUsed;

        if (timeRemaining <= 0) break;

        // Predict next depth time
        double estimatedNextDepthTime;
        if (depth > 2 && prevDepthTime > 0 && depthBeforePrev > 0) {
            double branchingFactor = prevDepthTime / depthBeforePrev;
            branchingFactor = std::max(2.0, std::min(branchingFactor, 6.0));
            estimatedNextDepthTime = prevDepthTime * branchingFactor;
        } else if (depth > 1 && prevDepthTime > 0) {
            estimatedNextDepthTime = prevDepthTime * 4.0;
        } else {
            estimatedNextDepthTime = 0.05;
        }

        // Stop if not enough time (30% safety buffer)
        if (depth > 1 && timeRemaining < estimatedNextDepthTime * 1.3) {
            break;
        }

        // Search at current depth
        Board boardCopy = state.getBoard();
        auto [_, move] = minimax(boardCopy, depth,
                                 -std::numeric_limits<double>::infinity(),
                                 std::numeric_limits<double>::infinity(),
                                 true, state.isWhiteToMove());

        if (timeout) break;

        double depthEndTime = getCPUTime();
        double currentDepthTime = depthEndTime - depthStartTime;

        if (move != Move()) {
            bestMove = move;
        }

        // Update time history
        depthBeforePrev = prevDepthTime;
        prevDepthTime = currentDepthTime;
    }

    return bestMove;
}
