#ifndef MOVE_H
#define MOVE_H

#include <string>

class Move {
public:
    int sr, sc, dr, dc;  // source row/col, destination row/col

    // Constructor
    Move() : sr(0), sc(0), dr(0), dc(0) {}
    Move(int sr, int sc, int dr, int dc) : sr(sr), sc(sc), dr(dr), dc(dc) {}

    // Convert move to string format "a12 a10"
    std::string toString() const;

    // Equality operator
    bool operator==(const Move& other) const {
        return sr == other.sr && sc == other.sc && dr == other.dr && dc == other.dc;
    }

    bool operator!=(const Move& other) const {
        return !(*this == other);
    }
};

#endif // MOVE_H
