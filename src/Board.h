#ifndef BOARD_H
#define BOARD_H

#include "Move.h"
#include <utility>
#include <string>

class Board {
private:
    char grid[12][12];  // Fixed-size array for performance

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
        grid[r][c] = piece;
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
