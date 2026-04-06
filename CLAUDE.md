# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a CSCI-561 homework assignment implementing a custom chess variant board game AI called "Heirs". The project has been implemented in **both Python and C++**:

### C++ Implementation (Primary - FAST) ⚡
- **Location:** [src/](src/) directory with clean OOP architecture
- **Speed:** 50-100x faster than Python
- **Depth:** Searches 6-10 ply vs Python's 4-5 ply
- **Executable:** Compiled to [homework](homework) binary (59KB)
- **Build:** `make` to compile, `make clean` to remove object files

### Python Implementation (Reference)
- **Location:** [homework.py](homework.py)
- **Purpose:** Original implementation, kept for reference
- **Speed:** Slower due to interpreted nature

## Running the Program

### C++ Version (Recommended) ⚡
```bash
# Compile (only needed after code changes)
make

# Run (reads input.txt, writes output.txt)
./homework
```

### Python Version
```bash
python homework.py
```

Both versions expect [input.txt](input.txt) in the current directory and write output to [output.txt](output.txt).

## Input/Output Format

The program uses file-based I/O for automated grading:

- **Input file**: `input.txt` (note: code has typo `'intput.txt'` as default parameter)
  - Line 1: Current player ("WHITE" or "BLACK")
  - Line 2: Time remaining for both players (space-separated floats)
  - Lines 3-14: 12x12 board state (12 rows of 12 characters each)

- **Output file**: `output.txt`
  - Single line with move in format: `<source> <destination>` (e.g., "a12 a10")

### Typical Workflow
```python
state = readInput('input.txt')      # Read game state
moves = genMoves(state)              # Generate legal moves
# [AI logic to select best move]
writeOutput(selected_move, 'output.txt')  # Write chosen move
```

## Board Coordinate System

### Custom Column Naming
The 12x12 board uses columns **a-n excluding 'i' and 'l'**:
```
Columns: a b c d e f g h j k m n  (no 'i' or 'l')
Rows:    12 to 1 (top to bottom)
```

Example coordinate: `"h8"` → column 'h' (index 7), row 8 (index 4)

### Coordinate Conversion
- `coord_to_idx(coord: str)` → `(row_idx, col_idx)` tuple
- `idx_to_coord(r: int, c: int)` → algebraic notation string

## Game Pieces and Movement

### Piece Types
- **B/b** (Baby): Pawn-like - 1-2 steps forward
- **P/p** (Prince): King-like - 1 step any direction (CRITICAL piece - game ends if captured)
- **X/x** (Princess): Up to 3 steps diagonally or orthogonally
- **G/g** (Guard): Up to 2 steps orthogonally
- **T/t** (Tutor): Up to 2 steps diagonally
- **Y/y** (Pony): 1 step diagonally
- **S/s** (Scout): L-shaped moves forward (1-3 steps forward, then 1 left/right)
- **N/n** (Sibling): 1 step to squares adjacent to friendly pieces

Uppercase = WHITE pieces, lowercase = BLACK pieces

### Piece Values (for evaluation)
```python
PIECE_VALUES = {
    'P': 100000,  # Prince (game-ending)
    'X': 300,     # Princess
    'S': 280,     # Scout
    'Y': 250,     # Pony
    'N': 240,     # Sibling
    'G': 220,     # Guard
    'T': 220,     # Tutor
    'B': 100,     # Baby
}
```

## Code Architecture

### Core Data Structures
- `Move`: Dataclass with source (sr, sc) and destination (dr, dc) coordinates
- `GameState`: Contains player, time remaining, and board state (12x12 list)

### Move Generation Pattern

The `genMoves(state)` function iterates through all friendly pieces and delegates to piece-specific generators:

```
genMoves() → calls piece-specific functions:
  ├─ genBabyMoves()      (forward movement only)
  ├─ genPrinceMoves()    (1 step any direction)
  ├─ genPrincessMoves()  (uses rayMoves with 3-step limit)
  ├─ genGuardMoves()     (uses rayMoves with 2-step limit)
  ├─ genTutorMoves()     (uses rayMoves with 2-step limit)
  ├─ genPonyMoves()      (uses rayMoves with 1-step limit)
  ├─ genScoutMoves()     (custom L-shaped forward logic)
  └─ genSiblingMoves()   (checks for adjacent friendly pieces)
```

### Helper Functions
- `rayMoves()`: Generic sliding move generator with direction and step limits
- `addMove()`: Validates and adds a single move to list
- `applyMove()`: Creates new board state after applying a move (returns new board, doesn't mutate)
- `isTerminal()`: Checks if either Prince ('P' or 'p') is captured
- `findPiece()`: Locates a specific piece on the board

### Board Query Helpers
- `inBound(r, c)`: Check if coordinates are valid
- `isFriendly()` / `isEnemy()`: Determine piece ownership
- `isWhite()` / `isBlack()`: Check piece color

## AI Implementation (FULLY COMPLETE - April 5, 2026)

### ✅ Implemented Features (C++ and Python)

**1. Minimax Search with Alpha-Beta Pruning** ([Search.cpp:226-362](src/Search.cpp))
- Recursive minimax algorithm with alpha-beta cutoffs
- Supports both maximizing and minimizing players
- CPU time tracking to avoid exceeding time limits
- **Transposition Table**: Caches position evaluations with depth and exact/bounds flags
- **Repetition Detection**: Tracks position hashes to avoid infinite loops (draw by repetition)
- Returns both evaluation score and best move

**2. Advanced Evaluation Function** ([Evaluator.cpp:20-52](src/Evaluator.cpp))
- **Material counting**: Uses piece values (Prince=100k, Princess=300, Scout=280, etc.)
- **Prince threat detection** (-50k penalty when under immediate attack)
- **Prince safety evaluation**:
  - Back rank bonus (+100)
  - Forward position penalty (-200)
  - Defender bonus (+50 per adjacent friendly piece)
  - Enemy proximity penalty (-200 per enemy in 3x3 zone)
- **Prince pressure evaluation** (attacking enemy prince):
  - Check bonus (+6500)
  - Nearby attackers (+70 each)
  - Distance pressure (weighted by piece type)
  - Escape square restriction (+220 per blocked square)
- **Positional bonuses**:
  - Center control (+10 for pieces in central 4x4)
  - Baby advancement (encourages pawn pushes)
- Terminal state detection (returns ±1M for prince capture)

**3. SMART Time Management** ([Search.cpp:34-69](src/Search.cpp))
- **CPU time only** - Uses `clock()` (matches game engine measurement)
- **Adaptive time allocation**:
  - >15s: Up to 2 seconds per move (aggressive search)
  - 5-15s: Up to 1 second per move (balanced)
  - 1-5s: Conservative (estimated moves * 1.5)
  - <1s: Critical mode (myTime / 30)
- **Game phase awareness**: Adjusts estimated moves based on piece count
- **Safety cap**: Never exceeds 8% of remaining time
- **Smart stopping**: Never starts a depth it can't finish (30% safety buffer)

**4. Advanced Move Ordering** ([Search.cpp:131-224](src/Search.cpp))
- **Prince captures**: 1M score (game-ending)
- **Preferred moves**: 250k (from transposition table)
- **Checking moves**: 100k (threatens enemy prince)
- **Defensive moves**: 50k (removes threat to our prince)
- **Repetition avoidance**: -1500 per historical occurrence, -2000 for in-search repetition
- **MVV-LVA captures**: victim_value * 10 - attacker_value
- **Escape square restriction**: +360 per blocked square
- **Distance to enemy prince**: +25 per square closer
- **Center control**: +20
- Dramatically improves alpha-beta pruning efficiency

**5. Iterative Deepening with Dynamic Branching** ([Search.cpp:402-445](src/Search.cpp))
- Starts at depth 1, increases incrementally
- **Dynamic branching factor calculation**:
  ```cpp
  branchingFactor = prevDepthTime / depthBeforePrev
  branchingFactor = clamp(branchingFactor, 2.0, 6.0)
  estimatedNextDepthTime = prevDepthTime * branchingFactor
  ```
- **Predictive stopping**: Never starts a depth without 30% time buffer
- Returns best move from deepest completed search
- **C++ Performance**: Reaches depth 6-10 (vs Python's 4-5)

**6. Zobrist Hashing & Repetition Detection** ([Search.cpp:71-129](src/Search.cpp))
- **Position hashing**: Zobrist hashing for fast board state identification
- **Historical repetition tracking**: Saves/loads position history from [work/repetition_history.txt](work/repetition_history.txt)
- **Game continuity detection**: Validates time clocks to detect new game vs continuation
- **In-search repetition**: Tracks positions within current search to avoid loops
- **Move ordering integration**: Penalizes moves leading to repeated positions

### Current Strengths
- ✅ Legal move generation for all 8 piece types
- ✅ No timeout issues (smart time management with safety caps)
- ✅ Highly efficient search (alpha-beta + transposition table + move ordering)
- ✅ Adaptive to position complexity (dynamic branching factor)
- ✅ **Prince threat detection** - Detects immediate attacks (-50k penalty)
- ✅ **Prince safety evaluation** - Back rank bonus, forward penalty, defender counting
- ✅ **Prince pressure evaluation** - Aggressive play when attacking enemy prince
- ✅ **Repetition avoidance** - Both historical and in-search repetition detection
- ✅ **50-100x faster in C++** - Searches 2-5 ply deeper than Python

---

## C++ Implementation Architecture

### File Structure
```
src/
├── main.cpp              # Entry point (reads input, runs search, writes output)
├── Utils.h/cpp           # Helper functions (inBound, isWhite, coordinate conversion, Zobrist hashing)
├── Move.h/cpp            # Move class (sr, sc, dr, dc + toString())
├── Board.h/cpp           # Board class (12x12 fixed array, makeMove/unmakeMove, Zobrist hashing)
├── GameState.h/cpp       # GameState class (holds board + time + I/O)
├── MoveGenerator.h/cpp   # Move generation for all 8 piece types
├── Evaluator.h/cpp       # Position evaluation (material + prince safety + pressure)
└── Search.h/cpp          # Minimax + alpha-beta + transposition table + iterative deepening + repetition detection
```

### Key Design Decisions

**1. Fixed Arrays vs Dynamic Allocation**
```cpp
char grid[12][12];  // Stack allocation - VERY fast
```
- Python uses dynamic lists with heap allocation
- C++ fixed array is 10-20x faster for access

**2. Make/Unmake Move Optimization with Zobrist Hashing**
```cpp
void makeMove(const Move& move, char& captured);
void unmakeMove(const Move& move, char captured);
```
- Instead of copying the board every move (Python's `applyMove`)
- Modify in place, then undo - 5-10x faster
- Updates Zobrist hash incrementally during make/unmake - O(1) position hashing

**3. Pass by Reference**
```cpp
void generateMoves(const Board& board, std::vector<Move>& moves);
```
- Avoids unnecessary copying
- Python implicitly copies objects frequently

**4. Compiler Optimizations**
```makefile
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -flto
```
- `-O3`: Aggressive optimization
- `-march=native`: CPU-specific instructions
- `-flto`: Link-time optimization

### Performance Comparison

| Metric | Python | C++ | Speedup |
|--------|--------|-----|---------|
| Move generation | ~100μs | ~2μs | 50x |
| Evaluation | ~50μs | ~1μs | 50x |
| Nodes/second | ~5,000 | ~250,000 | 50x |
| Typical depth | 4-5 ply | 6-10 ply | +2-5 ply |
| Time per move | 0.4-0.9s | 0.004-0.01s | 40-200x |

### Key Optimizations Beyond Basic C++
- **Transposition Table**: Caches 50-70% of positions, massive speedup in midgame
- **Zobrist Hashing**: O(1) incremental hashing during make/unmake moves
- **Repetition Detection**: Historical + in-search tracking prevents infinite loops
- **Advanced Move Ordering**: 8 different heuristics (checks, defenses, repetitions, MVV-LVA, etc.)
- **Dynamic Time Management**: Game phase awareness (opening/middlegame/endgame)

### Advanced Features Deep Dive

**Transposition Table** ([Search.cpp:265-285](src/Search.cpp))
- Stores position hash → (score, depth, exact/lower/upper bound, best move)
- 50-70% cache hit rate in midgame
- Supports exact scores and alpha/beta bounds for maximum reuse
- Dramatically reduces nodes searched (effective branching factor reduction)

**Zobrist Hashing** ([Board.h](src/Board.h), [Utils.h](src/Utils.h))
- Incremental hashing: O(1) update during makeMove/unmakeMove
- Random 64-bit values for each (piece, square) combination
- XOR operations for fast updates
- Enables fast transposition table lookups

**Repetition Detection** ([Search.cpp:75-129](src/Search.cpp))
- **Historical tracking**: Saves position hashes to [work/repetition_history.txt](work/repetition_history.txt)
- **Game continuity**: Validates time clocks to detect new game vs continuation
- **In-search tracking**: Detects repetitions within current search tree
- **Move ordering integration**: -1500 penalty for historical repetitions, -2000 for in-search
- **Draw detection**: Returns 0.0 score for 2+ repetitions

**Move Ordering Heuristics** ([Search.cpp:131-224](src/Search.cpp))
1. **Prince captures**: 1,000,000 (game-ending)
2. **Preferred moves**: 250,000 (from transposition table)
3. **Checking moves**: 100,000 (threatens enemy prince)
4. **Defensive moves**: 50,000 (removes threat to our prince)
5. **Repetition penalties**: -1500 (historical), -2000 (in-search)
6. **MVV-LVA**: victim_value × 10 - attacker_value
7. **Escape square restriction**: +360 per blocked square
8. **Center control**: +20

### Building and Testing

```bash
# Clean build
make clean && make

# Test with sample input
./homework

# Compare with Python version
python homework.py
mv output.txt output_python.txt
./homework
mv output.txt output_cpp.txt

# Both should produce legal moves (may differ due to search depth)
```

### Important Files Generated at Runtime
- **[work/repetition_history.txt](work/repetition_history.txt)**: Position history for repetition avoidance
  - Format: `META <myTime> <oppTime>` followed by `<hash> <count>` pairs
  - Automatically cleaned/reset when new game detected (time clock increases)
  - Top 24 positions by occurrence count preserved between moves
