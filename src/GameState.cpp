#include "GameState.h"
#include "Utils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

GameState::GameState(const Board& board, bool whiteToMove, double myTime, double oppTime)
    : board(board), whiteToMove(whiteToMove), myTime(myTime), oppTime(oppTime) {}

GameState GameState::readFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open input file: " + filename);
    }

    std::string line;

    // Line 1: Player ("WHITE" or "BLACK")
    std::getline(file, line);
    bool whiteToMove = (line == "WHITE");

    // Line 2: Time remaining (my_time opp_time)
    std::getline(file, line);
    std::istringstream iss(line);
    double myTime, oppTime;
    iss >> myTime >> oppTime;

    // Lines 3-14: Board state (12 rows of 12 characters)
    Board board;
    for (int r = 0; r < 12; r++) {
        std::getline(file, line);
        if (line.length() < 12) {
            throw std::runtime_error("Invalid board line at row " + std::to_string(r));
        }
        for (int c = 0; c < 12; c++) {
            board.setPiece(r, c, line[c]);
        }
    }

    file.close();
    return GameState(board, whiteToMove, myTime, oppTime);
}

void GameState::writeMove(const Move& move, const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open output file: " + filename);
    }

    file << move.toString() << "\n";
    file.close();
}
