#include "Evaluator.h"
#include "MoveGenerator.h"
#include "Utils.h"
#include <cmath>

// Initialize piece values using array indexing by ASCII value
const std::array<int, 128> Evaluator::PIECE_VALUES = [] {
    std::array<int, 128> v{};
    v['B'] = 100;  v['b'] = 100;
    v['P'] = 100000; v['p'] = 100000;
    v['X'] = 300;   v['x'] = 300;
    v['Y'] = 250;   v['y'] = 250;
    v['G'] = 220;   v['g'] = 220;
    v['T'] = 220;   v['t'] = 220;
    v['S'] = 280;   v['s'] = 280;
    v['N'] = 240;   v['n'] = 240;
    return v;
}();

double Evaluator::evaluate(const Board& board, bool isWhite) {
    // Check terminal state
    if (board.isTerminal()) {
        auto [wr, wc] = board.findPiece('P');
        auto [br, bc] = board.findPiece('p');

        if (wr != -1 && br == -1) {
            // White won
            return isWhite ? 1000000 : -1000000;
        } else if (br != -1 && wr == -1) {
            // Black won
            return isWhite ? -1000000 : 1000000;
        }
        return 0;  // Draw (shouldn't happen)
    }

    double score = 0;

    // Prince threat detection
    if (isPrinceUnderThreat(board, isWhite)) {
        score -= 50000;
    }

    // Prince safety evaluation
    score += evaluatePrinceSafety(board, isWhite);

    // Material and positional evaluation
    score += evaluateMaterial(board, isWhite);
    score += evaluatePosition(board, isWhite);

    return score;
}

bool Evaluator::isPrinceUnderThreat(const Board& board, bool isWhite) {
    char myPrince = isWhite ? 'P' : 'p';
    auto [pr, pc] = board.findPiece(myPrince);

    if (pr == -1) return false;  // Prince not found

    // Generate all enemy moves
    std::vector<Move> enemyMoves = MoveGenerator::generateMoves(board, !isWhite);

    // Check if any enemy move targets our prince
    for (const Move& move : enemyMoves) {
        if (move.dr == pr && move.dc == pc) {
            return true;
        }
    }

    return false;
}

double Evaluator::evaluatePrinceSafety(const Board& board, bool isWhite) {
    double score = 0;
    char myPrince = isWhite ? 'P' : 'p';
    auto [pr, pc] = board.findPiece(myPrince);

    if (pr == -1) return 0;  // Prince not found

    // 1. Back rank safety bonus (+100)
    if ((isWhite && pr == 0) || (!isWhite && pr == 11)) {
        score += 100;
    }

    // 2. Forward position penalty (-200)
    if ((isWhite && pr > 6) || (!isWhite && pr < 5)) {
        score -= 200;
    }

    // 3. Defender bonus (+50 per friendly piece adjacent)
    int defenderCount = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = pr + dr, nc = pc + dc;
            if (Utils::inBound(nr, nc) && Utils::isFriendly(board.getPiece(nr, nc), isWhite)) {
                defenderCount++;
            }
        }
    }
    score += defenderCount * 50;

    // 4. Enemy proximity penalty (-200 per enemy in 3x3 zone)
    int enemyCount = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = pr + dr, nc = pc + dc;
            if (Utils::inBound(nr, nc) && Utils::isEnemy(board.getPiece(nr, nc), isWhite)) {
                enemyCount++;
            }
        }
    }
    score -= enemyCount * 200;

    return score;
}

double Evaluator::evaluateMaterial(const Board& board, bool isWhite) {
    double score = 0;

    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            char piece = board.getPiece(r, c);
            if (piece == '.') continue;

            char pieceType = toupper(piece);
            int pieceValue = PIECE_VALUES[static_cast<int>(pieceType)];

            if (Utils::isWhite(piece)) {
                score += isWhite ? pieceValue : -pieceValue;
            } else {
                score += isWhite ? -pieceValue : pieceValue;
            }
        }
    }

    return score;
}

double Evaluator::evaluatePosition(const Board& board, bool isWhite) {
    double score = 0;

    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            char piece = board.getPiece(r, c);
            if (piece == '.') continue;

            double positionalBonus = 0;

            // Center control bonus (4x4 center)
            if (r >= 4 && r <= 7 && c >= 4 && c <= 7) {
                positionalBonus += 10;
            }

            // Baby advancement
            char pieceType = toupper(piece);
            if (pieceType == 'B') {
                if (Utils::isWhite(piece)) {
                    // White babies advance toward row 0
                    positionalBonus += (12 - r) * 2;
                } else {
                    // Black babies advance toward row 11
                    positionalBonus += r * 2;
                }
            }

            if (Utils::isWhite(piece)) {
                score += isWhite ? positionalBonus : -positionalBonus;
            } else {
                score += isWhite ? -positionalBonus : positionalBonus;
            }
        }
    }

    return score;
}
