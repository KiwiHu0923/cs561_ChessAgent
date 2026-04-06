#include "Search.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include "Utils.h"
#include <ctime>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <fstream>
#include <cctype>
#include <cerrno>
#include <sys/stat.h>

Search::Search(const GameState& state)
    : state(state),
      startTime(0),
      timeLimit(0),
      timeout(false),
      repetitionHistoryPath("work/repetition_history.txt") {
    loadRepetitionHistory();
}

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

uint64_t Search::positionKey(const Board& board, bool whiteToMove) const {
    return board.getHash() ^ (whiteToMove ? Utils::zobristSideToMove() : 0ULL);
}

void Search::loadRepetitionHistory() {
    std::ifstream file(repetitionHistoryPath);
    if (!file.is_open()) {
        return;
    }

    std::string tag;
    double savedMyTime = -1.0;
    double savedOppTime = -1.0;
    if (!(file >> tag >> savedMyTime >> savedOppTime) || tag != "META") {
        return;
    }

    // Both clocks should decrease during a game. If they jump upward, treat it as a new game.
    if (state.getMyTime() > savedMyTime + 0.25 || state.getOppTime() > savedOppTime + 0.25) {
        return;
    }

    uint64_t hash = 0;
    int count = 0;
    while (file >> hash >> count) {
        historicalResultCounts[hash] = count;
    }
}

void Search::saveRepetitionHistory(uint64_t resultKey) {
    historicalResultCounts[resultKey]++;

    std::vector<std::pair<uint64_t, int>> entries;
    entries.reserve(historicalResultCounts.size());
    for (const auto& entry : historicalResultCounts) {
        entries.push_back(entry);
    }

    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (entries.size() > 24) {
        entries.resize(24);
    }

    if (mkdir("work", 0755) != 0 && errno != EEXIST) {
        return;
    }

    std::ofstream file(repetitionHistoryPath, std::ios::trunc);
    if (!file.is_open()) {
        return;
    }

    file << "META " << state.getMyTime() << " " << state.getOppTime() << "\n";
    for (const auto& [hash, count] : entries) {
        file << hash << " " << count << "\n";
    }
}

double Search::moveScore(Board& board, const Move& move, bool isWhite, const Move& preferredMove) {
    double score = 0;

    char movingPiece = board.getPiece(move.sr, move.sc);
    char captured = board.getPiece(move.dr, move.dc);
    int attackerValue = Evaluator::PIECE_VALUES[static_cast<int>(toupper(movingPiece))];
    bool wasThreatened = Evaluator::isPrinceUnderThreat(board, isWhite);

    // 1. Prince capture (game-ending)
    if (toupper(captured) == 'P') {
        return 1000000;
    }

    if (move == preferredMove) {
        score += 250000;
    }

    char enemyPrince = isWhite ? 'p' : 'P';
    auto [oldPr, oldPc] = board.findPiece(enemyPrince);
    int oldDist = 100;
    if (oldPr != -1) {
        oldDist = std::abs(oldPr - move.sr) + std::abs(oldPc - move.sc);
    }

    // 2. Checking moves (threatens enemy prince)
    char undoneCapture;
    board.makeMove(move, undoneCapture);
    if (Evaluator::isPrinceUnderThreat(board, !isWhite)) {
        score += 100000;
    }

    // 3. Defensive moves (our prince is threatened)
    if (wasThreatened && !Evaluator::isPrinceUnderThreat(board, isWhite)) {
        score += 50000;
    }

    auto [newPr, newPc] = board.findPiece(enemyPrince);
    if (newPr != -1) {
        int newDist = std::abs(newPr - move.dr) + std::abs(newPc - move.dc);
        if (newDist < oldDist) {
            score += (oldDist - newDist) * 25;
        }
    }

    int escapeSquares = Evaluator::countEnemyPrinceEscapeSquares(board, !isWhite);
    score += (8 - escapeSquares) * 360;

    uint64_t nextKey = positionKey(board, !isWhite);
    auto historyIt = historicalResultCounts.find(nextKey);
    if (historyIt != historicalResultCounts.end()) {
        score -= 1500.0 * historyIt->second;
    }

    if (currentLineCounts.find(nextKey) != currentLineCounts.end()) {
        score -= 2000.0;
    }

    // 4. MVV-LVA captures
    if (captured != '.') {
        int victimValue = Evaluator::PIECE_VALUES[static_cast<int>(toupper(captured))];
        score += victimValue * 10 - attackerValue;
    }

    // 5. Center control
    if (move.dr >= 4 && move.dr <= 7 && move.dc >= 4 && move.dc <= 7) {
        score += 20;
    }

    board.unmakeMove(move, undoneCapture);
    return score;
}

std::vector<Move> Search::orderMoves(Board& board, const std::vector<Move>& moves, bool isWhite,
                                     const Move& preferredMove) {
    std::vector<std::pair<double, Move>> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const Move& move : moves) {
        double score = moveScore(board, move, isWhite, preferredMove);
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

    bool currentPlayerIsWhite = (maximizing == isWhite);
    uint64_t key = positionKey(board, currentPlayerIsWhite);

    int& seenCount = currentLineCounts[key];
    seenCount++;
    struct RepetitionGuard {
        std::unordered_map<uint64_t, int>& counts;
        uint64_t key;
        ~RepetitionGuard() {
            auto it = counts.find(key);
            if (it == counts.end()) {
                return;
            }
            if (--it->second == 0) {
                counts.erase(it);
            }
        }
    } guard{currentLineCounts, key};

    if (seenCount >= 2) {
        return {0.0, Move()};
    }

    // Terminal or depth limit reached
    if (depth == 0 || board.isTerminal()) {
        return {Evaluator::evaluate(board, isWhite), Move()};
    }

    double originalAlpha = alpha;
    double originalBeta = beta;

    auto ttIt = transpositionTable.find(key);
    Move preferredMove;
    if (ttIt != transpositionTable.end()) {
        const TTEntry& entry = ttIt->second;
        preferredMove = entry.bestMove;

        if (entry.depth >= depth) {
            if (entry.flag == TT_EXACT) {
                return {entry.score, entry.bestMove};
            }
            if (entry.flag == TT_LOWER) {
                alpha = std::max(alpha, entry.score);
            } else if (entry.flag == TT_UPPER) {
                beta = std::min(beta, entry.score);
            }

            if (alpha >= beta) {
                return {entry.score, entry.bestMove};
            }
        }
    }

    std::vector<Move> moves = MoveGenerator::generateMoves(board, currentPlayerIsWhite);

    if (moves.empty()) {
        // No legal moves - treat as loss
        double terminalScore = maximizing ? -1000000.0 : 1000000.0;
        transpositionTable[key] = {terminalScore, depth, TT_EXACT, Move()};
        return {terminalScore, Move()};
    }

    // Order moves for better pruning
    moves = orderMoves(board, moves, currentPlayerIsWhite, preferredMove);

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

        uint8_t flag = TT_EXACT;
        if (maxEval <= originalAlpha) {
            flag = TT_UPPER;
        } else if (maxEval >= originalBeta) {
            flag = TT_LOWER;
        }
        transpositionTable[key] = {maxEval, depth, flag, bestMove};
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

        uint8_t flag = TT_EXACT;
        if (minEval <= originalAlpha) {
            flag = TT_UPPER;
        } else if (minEval >= originalBeta) {
            flag = TT_LOWER;
        }
        transpositionTable[key] = {minEval, depth, flag, bestMove};
        return {minEval, bestMove};
    }
}

Move Search::selectBestMove() {
    // Generate moves
    Board board = state.getBoard();
    std::vector<Move> moves = MoveGenerator::generateMoves(board, state.isWhiteToMove());

    if (moves.empty()) {
        throw std::runtime_error("No legal moves available");
    }

    if (moves.size() == 1) {
        char captured;
        board.makeMove(moves[0], captured);
        saveRepetitionHistory(positionKey(board, !state.isWhiteToMove()));
        return moves[0];
    }

    // Emergency: if very low time, just return first ordered move
    if (state.getMyTime() < 0.1) {
        moves = orderMoves(board, moves, state.isWhiteToMove());
        char captured;
        board.makeMove(moves[0], captured);
        saveRepetitionHistory(positionKey(board, !state.isWhiteToMove()));
        return moves[0];
    }

    // Calculate time limit
    int piecesCount = board.countPieces();
    timeLimit = calculateTimeLimit(piecesCount, state.getMyTime());

    // Iterative deepening
    startTime = getCPUTime();
    timeout = false;
    transpositionTable.clear();
    currentLineCounts.clear();
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
        Board boardCopy = board;
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

    char captured;
    board.makeMove(bestMove, captured);
    saveRepetitionHistory(positionKey(board, !state.isWhiteToMove()));

    return bestMove;
}
