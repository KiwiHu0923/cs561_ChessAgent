#include "Move.h"
#include "Utils.h"

std::string Move::toString() const {
    return Utils::idxToCoord(sr, sc) + " " + Utils::idxToCoord(dr, dc);
}
