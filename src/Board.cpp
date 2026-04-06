#include "Board.h"
#include "Utils.h"
#include <cstring>
#include <sstream>

Board::Board() {
    zobristHash = 0;

    // Initialize empty board
    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            grid[r][c] = '.';
        }
    }
}

Board::Board(const Board& other) {
    std::memcpy(grid, other.grid, sizeof(grid));
    zobristHash = other.zobristHash;
}

Board& Board::operator=(const Board& other) {
    if (this != &other) {
        std::memcpy(grid, other.grid, sizeof(grid));
        zobristHash = other.zobristHash;
    }
    return *this;
}

void Board::applyMove(const Move& move) {
    char piece = grid[move.sr][move.sc];
    char captured = grid[move.dr][move.dc];

    zobristHash ^= Utils::zobristPiece(move.sr, move.sc, piece);
    if (captured != '.') {
        zobristHash ^= Utils::zobristPiece(move.dr, move.dc, captured);
    }

    grid[move.sr][move.sc] = '.';
    grid[move.dr][move.dc] = piece;

    zobristHash ^= Utils::zobristPiece(move.dr, move.dc, piece);
}

void Board::makeMove(const Move& move, char& captured) {
    captured = grid[move.dr][move.dc];
    char piece = grid[move.sr][move.sc];

    zobristHash ^= Utils::zobristPiece(move.sr, move.sc, piece);
    if (captured != '.') {
        zobristHash ^= Utils::zobristPiece(move.dr, move.dc, captured);
    }

    grid[move.sr][move.sc] = '.';
    grid[move.dr][move.dc] = piece;

    zobristHash ^= Utils::zobristPiece(move.dr, move.dc, piece);
}

void Board::unmakeMove(const Move& move, char captured) {
    char piece = grid[move.dr][move.dc];

    zobristHash ^= Utils::zobristPiece(move.dr, move.dc, piece);
    if (captured != '.') {
        zobristHash ^= Utils::zobristPiece(move.dr, move.dc, captured);
    }

    grid[move.dr][move.dc] = captured;
    grid[move.sr][move.sc] = piece;

    zobristHash ^= Utils::zobristPiece(move.sr, move.sc, piece);
}

std::pair<int, int> Board::findPiece(char piece) const {
    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            if (grid[r][c] == piece) {
                return {r, c};
            }
        }
    }
    return {-1, -1};  // Not found
}

bool Board::isTerminal() const {
    auto [wr, wc] = findPiece('P');
    auto [br, bc] = findPiece('p');
    return (wr == -1) || (br == -1);
}

int Board::countPieces() const {
    int count = 0;
    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            if (grid[r][c] != '.') {
                count++;
            }
        }
    }
    return count;
}

std::string Board::toString() const {
    std::ostringstream oss;
    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            oss << grid[r][c];
        }
        oss << "\n";
    }
    return oss.str();
}
