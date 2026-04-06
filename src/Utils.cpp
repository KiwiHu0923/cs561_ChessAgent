#include "Utils.h"
#include <cctype>
#include <array>

namespace Utils {
    // Column names: a-n excluding 'i' and 'l'
    const char COLS[12] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'j', 'k', 'm', 'n'};

    // Fast lookup table for column character to index
    const std::array<int, 128> COL_MAP = [] {
        std::array<int, 128> m{};
        m['a'] = 0;  m['b'] = 1;  m['c'] = 2;  m['d'] = 3;
        m['e'] = 4;  m['f'] = 5;  m['g'] = 6;  m['h'] = 7;
        m['j'] = 8;  m['k'] = 9;  m['m'] = 10; m['n'] = 11;
        return m;
    }();

    std::string idxToCoord(int r, int c) {
        std::string result;
        result += COLS[c];
        result += std::to_string(BOARD_SIZE - r);
        return result;
    }

    std::pair<int, int> coordToIdx(const std::string& coord) {
        char col = coord[0];
        int row = std::stoi(coord.substr(1));
        return {BOARD_SIZE - row, COL_MAP[static_cast<int>(col)]};
    }
}
