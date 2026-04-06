#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <utility>
#include <array>
#include <cstdint>

namespace Utils {
    // Board constants
    constexpr int BOARD_SIZE = 12;
    extern const char COLS[12];
    extern const std::array<int, 128> COL_MAP;  // Fast character-to-index mapping

    // Board boundary checking
    inline bool inBound(int r, int c) {
        return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
    }

    // Piece type checking
    inline bool isWhite(char piece) {
        return piece >= 'A' && piece <= 'Z';
    }

    inline bool isBlack(char piece) {
        return piece >= 'a' && piece <= 'z';
    }

    inline bool isFriendly(char piece, bool isWhite) {
        if (piece == '.') return false;
        return isWhite ? Utils::isWhite(piece) : Utils::isBlack(piece);
    }

    inline bool isEnemy(char piece, bool isWhite) {
        if (piece == '.') return false;
        return isWhite ? Utils::isBlack(piece) : Utils::isWhite(piece);
    }

    // Coordinate conversion
    std::string idxToCoord(int r, int c);
    std::pair<int, int> coordToIdx(const std::string& coord);

    // Zobrist hashing helpers
    uint64_t zobristPiece(int r, int c, char piece);
    uint64_t zobristSideToMove();
}

#endif // UTILS_H
