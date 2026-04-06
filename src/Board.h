#ifndef BOARD_H
#define BOARD_H

#include "Move.h"
#include "Utils.h"
#include <utility>
#include <string>
#include <cstdint>

class Board {
private:
    char grid[12][12];  // Fixed-size array for performance
    uint64_t zobristHash;

public:
    // Constructors
    Board();
    Board(const Board& other);  // Copy constructor

    // Assignment operator
    Board& operator=(const Board& other);

    // Access methods
    inline char getPiece(int r, int c) const {
        return grid[r][c];
    }

    inline void setPiece(int r, int c, char piece) {
        if (grid[r][c] != '.') {
            zobristHash ^= Utils::zobristPiece(r, c, grid[r][c]);
        }
        grid[r][c] = piece;
        if (piece != '.') {
            zobristHash ^= Utils::zobristPiece(r, c, piece);
        }
    }

    inline uint64_t getHash() const {
        return zobristHash;
    }

    // Move operations
    void applyMove(const Move& move);
    void makeMove(const Move& move, char& captured);   // For make/unmake optimization
    void unmakeMove(const Move& move, char captured);

    // Board queries
    std::pair<int, int> findPiece(char piece) const;
    bool isTerminal() const;
    int countPieces() const;

    // Debug/Display
    std::string toString() const;
};

#endif // BOARD_H
