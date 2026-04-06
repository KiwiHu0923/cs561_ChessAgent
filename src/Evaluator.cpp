#include "Evaluator.h"
#include "MoveGenerator.h"
#include "Utils.h"
#include <cmath>

// Initialize piece values
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
    if (board.isTerminal()) {
        auto [wr, wc] = board.findPiece('P');
        auto [br, bc] = board.findPiece('p');

        if (wr != -1 && br == -1) {
            return isWhite ? 1000000 : -1000000;
        } else if (br != -1 && wr == -1) {
            return isWhite ? -1000000 : 1000000;
        }
        return 0; 
    }

    double score = 0;

    // Prince threat detection
    if (isPrinceUnderThreat(board, isWhite)) {
        score -= 50000;
    }

    // Prince safety evaluation
    score += evaluatePrinceSafety(board, isWhite);
    score += evaluatePrincePressure(board, isWhite);

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

int Evaluator::pressurePieceWeight(char piece) {
    switch (toupper(piece)) {
        case 'X': return 16; // Princess
        case 'S': return 14; // Scout
        case 'Y': return 10; // Pony
        case 'N': return 10; // Sibling
        case 'G': return 9;  // Guard
        case 'T': return 9;  // Tutor
        case 'B': return 3;  // Baby
        case 'P': return 0;  // Prince itself not counted as attacker pressure
        default: return 0;
    }
}

int Evaluator::countAttackersNearEnemyPrince(const Board& board, bool attackerIsWhite, int oppoPrinceR, int oppoPrinceC) {
    int count = 0;

    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            char piece = board.getPiece(r, c);
            if (piece == '.' || !Utils::isFriendly(piece, !attackerIsWhite)) continue;

            int dist = std::abs(r - oppoPrinceR) + std::abs(c - oppoPrinceC);

            if (dist <= 2) 
                count ++;
        }
    }

    return count;
}

// Count how many escape squares the enemy prince has (not occupied by friendly pieces and not attacked)
int Evaluator::countEnemyPrinceEscapeSquares(const Board& board, bool princeIsWhite) {
    char prince = princeIsWhite ? 'P' : 'p';
    auto [pr, pc] = board.findPiece(prince);
    if (pr == -1) return 0;  // Prince not found

    bool attacker = !princeIsWhite;
    std::vector<Move> attackerMoves = MoveGenerator::generateMoves(board, attacker);

    bool attacked[12][12] = {};
    for (const Move& move : attackerMoves) {
        attacked[move.dr][move.dc] = true;
    }

    int escapeSquares = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;

            int nr = pr + dr, nc = pc + dc;
            if (!Utils::inBound(nr, nc)) continue;

            char dest = board.getPiece(nr, nc);
            if (dest == '.' || Utils::isFriendly(dest, princeIsWhite)) continue;
            if (attacked[nr][nc]) continue;
        }
        escapeSquares++;
    }

    return escapeSquares;
}

// Evaluate pressure on the enemy prince based on proximity of friendly pieces, distance-based pressure, and escape square reduction
double Evaluator::evaluatePrincePressure(const Board&board, bool isWhite) {
    double score = 0;
    char enemyPrince = isWhite ? 'p' : 'P';
    auto [epr, epc] = board.findPiece(enemyPrince);
    if (epr == -1) return 0;  // enemy prince not found

    // 1. Immediate direct threat bonus (+6000)
    if (isPrinceUnderThreat(board, !isWhite)) {
        score += 6000;
    }

    // 2. Friendly attackers near enemy prince
    int nearbyAttackers = countAttackersNearEnemyPrince(board, isWhite, epr, epc);
    score += nearbyAttackers * 40;

    // 3. Distance-based pressure from attacking pieces
    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            char piece = board.getPiece(r, c);
            if (piece == '.' || !Utils::isFriendly(piece, isWhite)) continue;

            int weight = pressurePieceWeight(piece);
            if (weight == 0) continue;

            int dist = std::abs(r - epr) + std::abs(c - epc);
            if (dist <= 4) {
                score += (5 - dist) * weight;
            }
        }
    }


    // Fewer escape squares for Enemy Prince means higher pressure
    int escapeSquares = countEnemyPrinceEscapeSquares(board, !isWhite);
    score += (8 - escapeSquares) * 120;

    if (escapeSquares <= 2) {
        score += 250;  
    }

    return score;
}

// Material evaluation based on piece values
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

// Positional evaluation based on piece placement and strategic factors
// - Center control: Pieces in the 4x4 center get a bonus
// - Baby advancement: Babies get a bonus for advancing toward the enemy side
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
