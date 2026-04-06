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

    namespace {
        uint64_t splitmix64(uint64_t x) {
            x += 0x9e3779b97f4a7c15ULL;
            x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
            x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
            return x ^ (x >> 31);
        }

        constexpr uint64_t ZOBRIST_SEED = 0x561561561ULL;

        const std::array<std::array<std::array<uint64_t, 128>, BOARD_SIZE>, BOARD_SIZE> ZOBRIST_PIECES = [] {
            std::array<std::array<std::array<uint64_t, 128>, BOARD_SIZE>, BOARD_SIZE> table{};
            for (int r = 0; r < BOARD_SIZE; ++r) {
                for (int c = 0; c < BOARD_SIZE; ++c) {
                    for (int piece = 0; piece < 128; ++piece) {
                        table[r][c][piece] = splitmix64(
                            ZOBRIST_SEED ^ static_cast<uint64_t>(r * BOARD_SIZE * 128 + c * 128 + piece)
                        );
                    }
                }
            }
            return table;
        }();

        const uint64_t ZOBRIST_SIDE = splitmix64(ZOBRIST_SEED ^ 0xabcdef1234567890ULL);
    }

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

    uint64_t zobristPiece(int r, int c, char piece) {
        if (piece == '.') {
            return 0;
        }
        return ZOBRIST_PIECES[r][c][static_cast<unsigned char>(piece)];
    }

    uint64_t zobristSideToMove() {
        return ZOBRIST_SIDE;
    }
}
