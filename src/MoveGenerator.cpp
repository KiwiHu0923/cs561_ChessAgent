#include "MoveGenerator.h"
#include "Utils.h"

std::vector<Move> MoveGenerator::generateMoves(const Board& board, bool isWhite) {
    std::vector<Move> moves;
    moves.reserve(60);  // Typical max moves, avoid reallocation

    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            char piece = board.getPiece(r, c);
            if (piece == '.' || !Utils::isFriendly(piece, isWhite)) {
                continue;
            }

            char pieceType = toupper(piece);
            switch (pieceType) {
                case 'B': genBabyMoves(board, r, c, isWhite, moves); break;
                case 'P': genPrinceMoves(board, r, c, isWhite, moves); break;
                case 'X': genPrincessMoves(board, r, c, isWhite, moves); break;
                case 'Y': genPonyMoves(board, r, c, isWhite, moves); break;
                case 'G': genGuardMoves(board, r, c, isWhite, moves); break;
                case 'T': genTutorMoves(board, r, c, isWhite, moves); break;
                case 'S': genScoutMoves(board, r, c, isWhite, moves); break;
                case 'N': genSiblingMoves(board, r, c, isWhite, moves); break;
            }
        }
    }

    return moves;
}

void MoveGenerator::addMove(const Board& board, int sr, int sc, int dr, int dc,
                            bool isWhite, std::vector<Move>& moves) {
    if (!Utils::inBound(dr, dc)) return;

    char destPiece = board.getPiece(dr, dc);
    if (Utils::isFriendly(destPiece, isWhite)) return;

    moves.emplace_back(sr, sc, dr, dc);
}

void MoveGenerator::rayMoves(const Board& board, int r, int c, bool isWhite,
                             const int directions[][2], int numDirections,
                             int maxSteps, std::vector<Move>& moves) {
    for (int d = 0; d < numDirections; d++) {
        int dr = directions[d][0];
        int dc = directions[d][1];
        int nr = r, nc = c;

        for (int step = 0; step < maxSteps; step++) {
            nr += dr;
            nc += dc;

            if (!Utils::inBound(nr, nc) || Utils::isFriendly(board.getPiece(nr, nc), isWhite)) {
                break;
            }

            moves.emplace_back(r, c, nr, nc);

            if (Utils::isEnemy(board.getPiece(nr, nc), isWhite)) {
                break;
            }
        }
    }
}

void MoveGenerator::genBabyMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves) {
    int direction = isWhite ? -1 : 1;

    // 1 step forward
    int r1 = r + direction;
    if (Utils::inBound(r1, c) && !Utils::isFriendly(board.getPiece(r1, c), isWhite)) {
        moves.emplace_back(r, c, r1, c);
    }

    // 2 steps forward (must be clear in between)
    int r2 = r + 2 * direction;
    if (Utils::inBound(r2, c) && board.getPiece(r1, c) == '.' &&
        !Utils::isFriendly(board.getPiece(r2, c), isWhite)) {
        moves.emplace_back(r, c, r2, c);
    }
}

void MoveGenerator::genPrinceMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves) {
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            addMove(board, r, c, r + dr, c + dc, isWhite, moves);
        }
    }
}

void MoveGenerator::genPrincessMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves) {
    static const int directions[8][2] = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1},  // Diagonals
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}     // Orthogonals
    };
    rayMoves(board, r, c, isWhite, directions, 8, 3, moves);
}

void MoveGenerator::genGuardMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves) {
    static const int directions[4][2] = {
        {-1, 0}, {0, -1}, {1, 0}, {0, 1}  // Orthogonals only
    };
    rayMoves(board, r, c, isWhite, directions, 4, 2, moves);
}

void MoveGenerator::genTutorMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves) {
    static const int directions[4][2] = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonals only
    };
    rayMoves(board, r, c, isWhite, directions, 4, 2, moves);
}

void MoveGenerator::genPonyMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves) {
    static const int directions[4][2] = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonals only
    };
    rayMoves(board, r, c, isWhite, directions, 4, 1, moves);
}

void MoveGenerator::genScoutMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves) {
    int direction = isWhite ? -1 : 1;

    for (int step = 1; step <= 3; step++) {
        int nr = r + step * direction;
        if (!Utils::inBound(nr, c)) break;

        // Left and right from the forward position
        for (int side = -1; side <= 1; side += 2) {
            int nc = c + side;
            if (Utils::inBound(nr, nc) && !Utils::isFriendly(board.getPiece(nr, nc), isWhite)) {
                moves.emplace_back(r, c, nr, nc);
            }
        }
    }
}

void MoveGenerator::genSiblingMoves(const Board& board, int r, int c, bool isWhite, std::vector<Move>& moves) {
    static const int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };

    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;

            int nr = r + dr;
            int nc = c + dc;

            if (!Utils::inBound(nr, nc) || Utils::isFriendly(board.getPiece(nr, nc), isWhite)) {
                continue;
            }

            // Check if there's a friendly piece adjacent to the destination
            bool hasAdjacentFriendly = false;
            for (int i = 0; i < 8; i++) {
                int nnr = nr + directions[i][0];
                int nnc = nc + directions[i][1];

                // Don't check the source square itself
                if (nnr == r && nnc == c) continue;

                if (Utils::inBound(nnr, nnc) && Utils::isFriendly(board.getPiece(nnr, nnc), isWhite)) {
                    hasAdjacentFriendly = true;
                    break;
                }
            }

            if (hasAdjacentFriendly) {
                moves.emplace_back(r, c, nr, nc);
            }
        }
    }
}
