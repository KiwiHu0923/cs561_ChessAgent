# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a CSCI-561 homework assignment implementing a custom chess variant board game AI. The codebase focuses on move generation and game state management for an automated game-playing agent.

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

## AI Implementation (COMPLETED - April 4, 2026)

### ✅ Implemented Features

**1. Minimax Search with Alpha-Beta Pruning** (`homework.py:334-391`)
- Recursive minimax algorithm with alpha-beta cutoffs
- Supports both maximizing and minimizing players
- CPU time tracking to avoid exceeding time limits
- Returns both evaluation score and best move

**2. Evaluation Function** (`homework.py:263-314`)
- Material counting using PIECE_VALUES
- Positional bonuses:
  - Center control (+10 for pieces in central 4x4)
  - Baby advancement (encourages pawn pushes)
- Terminal state detection (returns ±1000000 for prince capture)

**3. SMART Time Management** (`homework.py:402-517`) ✨
- **CPU time only** - Uses `time.process_time()` (matches game engine measurement)
- **Adaptive time allocation**:
  - >15s: Up to 2 seconds per move (aggressive search)
  - 5-15s: Up to 1 second per move (balanced)
  - 1-5s: 8% of remaining time (conservative)
  - <1s: 30% emergency mode
- **Dynamic branching factor calculation**:
  ```python
  branching_factor = prev_depth_time / depth_before_prev
  estimated_next_depth_time = prev_depth_time * branching_factor
  ```
- **Smart stopping**: Never starts a depth it can't finish (30% safety buffer)
- **Result**: Reaches depth 4-8 depending on position complexity

**4. Move Ordering** (`homework.py:316-332`)
- Prioritizes captures (10x piece value)
- Rewards center control (+20)
- Improves alpha-beta pruning efficiency

**5. Iterative Deepening** (`homework.py:467-505`)
- Starts at depth 1, increases incrementally
- Uses depth-time prediction to stop before timeout
- Returns best move from deepest completed search

**6. Prince Threat Detection** (`homework.py:124-152`) ✨ **NEW - April 5, 2026**
- **New function**: `isPrinceUnderThreat(board, is_white)`
  - Generates all legal enemy moves
  - Checks if any move targets our prince position
  - Returns True if threat detected, False if safe
- **Integration**: Called in `evaluate()` before material calculation (lines 280-283)
- **Penalty**: -50,000 points when prince is under immediate threat
- **Algorithm**:
  1. Find our prince position using `findPiece()`
  2. Create opponent GameState
  3. Generate all opponent moves via `genMoves()`
  4. Check if any move.dr/dc equals prince position
  5. Return True if threat found
- **Why this works**:
  - Reuses existing `genMoves()` - no code duplication
  - Guaranteed accuracy (same logic as AI's move generation)
  - Detects all immediate threats (one-move captures)
- **Performance impact**: ~3× per evaluation, but better pruning compensates
- **Fixes critical bug**: AI now defends prince when threatened (prevents Move 58 blunder)

### Current Strengths
- ✅ Legal move generation for all piece types
- ✅ No timeout issues (smart time management)
- ✅ Efficient search (alpha-beta + move ordering)
- ✅ Adaptive to position complexity (branching factor)
- ✅ **Prince threat detection** - Detects immediate attacks on our prince (-50k penalty)

### Known Weaknesses (From Game Analysis)
- ❌ **No mobility evaluation** - Doesn't value having more legal moves
- ❌ **No defensive coordination** - Pieces don't defend key squares
- ❌ **No prince position safety** - Doesn't prefer back rank over forward positions
- ❌ **No enemy proximity penalties** - Doesn't penalize enemies near prince zone

### Next Improvements Needed (Prince Safety Features)
1. **Prince position safety** - Bonuses for back rank (+100), penalties for forward positions (-200)
2. **Enemy proximity penalties** - Penalty for enemy pieces near prince zone 3x3 (-200 per piece)
3. **Defender bonuses** - Reward friendly pieces adjacent to prince (+50 each)
4. **Threat detection in move ordering** - Prioritize defensive moves in orderMoves()
5. **Mobility evaluation** - +5 points per legal move available
6. **Piece coordination** - Bonuses for defending key squares
