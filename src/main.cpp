#include "GameState.h"
#include "Search.h"
#include <iostream>
#include <exception>

int main() {
    try {
        // Read game state from input file
        GameState state = GameState::readFromFile("input.txt");

        // Search for best move
        Search search(state);
        Move bestMove = search.selectBestMove();

        // Write move to output file
        state.writeMove(bestMove, "output.txt");

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
