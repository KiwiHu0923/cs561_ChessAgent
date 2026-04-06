#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Board.h"
#include "Move.h"
#include <string>

class GameState {
private:
    Board board;
    bool whiteToMove;
    double myTime;
    double oppTime;

public:
    // Constructor
    GameState(const Board& board, bool whiteToMove, double myTime, double oppTime);

    // Accessors
    const Board& getBoard() const { return board; }
    Board& getBoardMutable() { return board; }
    bool isWhiteToMove() const { return whiteToMove; }
    double getMyTime() const { return myTime; }
    double getOppTime() const { return oppTime; }

    // I/O operations
    static GameState readFromFile(const std::string& filename);
    void writeMove(const Move& move, const std::string& filename) const;
};

#endif // GAMESTATE_H
