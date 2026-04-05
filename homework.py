from __future__ import annotations

from dataclasses import dataclass
from typing import List, Optional, Tuple
import math
import random
import time

BOARD_SIZE = 12
COLS = ["a", "b", "c", "d", "e", "f", "g", "h", "j", "k", "m", "n"]
COLS_TO_IDX = {c: i for i, c in enumerate(COLS)}
IDX_TO_COLS = {i: c for i, c in enumerate(COLS)}

WHITE_PIECES = set("BPXYGTSN")
BLACK_PIECES = set("bpxygtsn")
ALL_PIECES = WHITE_PIECES | BLACK_PIECES

PIECE_VALUES = {
    'B': 100,
    'P': 100000,
    'X': 300,
    'Y': 250,
    'G': 220,
    'T': 220,
    'S': 280,
    'N': 240,
}

@dataclass(frozen=True)
class Move:
    sr: int
    sc: int
    dr: int
    dc: int

    def __str__(self) -> str:
        return f"{idx_to_coord(self.sr, self.sc)} {idx_to_coord(self.dr, self.dc)}"
    
@dataclass
class GameState:
    player: str
    my_time: float
    opp_time: float
    board: List[List[str]]

    def copy(self) -> "GameState":
        return GameState(
            player=self.player,
            my_time=self.my_time,
            opp_time=self.opp_time,
            board=[row[:] for row in self.board]
        )
    
    @property
    def me_is_white(self) -> bool:
        return self.player == "WHITE"
    
    def myPieces(self) -> set[str]:
        return WHITE_PIECES if self.me_is_white else BLACK_PIECES
    
    def oppPieces(self) -> set[str]:
        return BLACK_PIECES if self.me_is_white else WHITE_PIECES


#-----------------------
# Timeout
#-----------------------
class SearchTimeout(Exception):
    pass
    

def idx_to_coord(r: int, c: int) -> str:
    return f"{COLS[c]}{BOARD_SIZE - r}"

def coord_to_idx(coord: str) -> Tuple[int, int]:
    col = coord[0]
    row = int(coord[1:])
    return BOARD_SIZE - row, COLS_TO_IDX[col]

def readInput(path: str='input.txt') -> GameState:
    with open(path, 'r', encoding='utf-8') as f:
        lines = f.read().splitlines()
    
    player = lines[0].strip()
    my_time_s, opp_time_s = lines[1].split()
    board_lines = lines[2:14]

    board = [list(line) for line in board_lines]
    return GameState(
        player=player,
        my_time=float(my_time_s),
        opp_time=float(opp_time_s),
        board=board
    )

def writeOutput(move: Move, path: str='output.txt') -> None:
    with open(path, 'w', encoding='utf-8') as f:
        f.write(str(move) + '\n')

def inBound(r, c) -> bool:
    return 0 <= r < BOARD_SIZE and 0 <= c < BOARD_SIZE

def isWhite(ch: str) -> bool:
    return ch in WHITE_PIECES

def isBlack(ch: str) -> bool:
    return ch in BLACK_PIECES

def isFriendly(ch: str, me_is_white: bool) -> bool:
    if ch == '.':
        return False
    
    return isWhite(ch) if me_is_white else isBlack(ch)

def isEnemy(ch: str, me_is_white: bool) -> bool:
    if ch == '.':
        return False
    
    return isBlack(ch) if me_is_white else isWhite(ch)

def isTerminal(board) -> bool:
    return findPiece(board, 'P') is None or findPiece(board, 'p') is None

def findPiece(board: List[List[str]], piece: str) -> Optional[Tuple[int, int]]:
    for r in range(BOARD_SIZE):
        for c in range(BOARD_SIZE):
            if board[r][c] == piece:
                return r, c
    return None

def isPrinceUnderThreat(board: List[List[str]], is_white: bool) -> bool:
    """
    Check if the current player's prince is under immediate threat.
    """
    my_prince = 'P' if is_white else 'p'

    
    prince_pos = findPiece(board, my_prince)
    if prince_pos is None:
        return False 
    prince_r, prince_c = prince_pos

    # Create temporary GameState for opponent
    opponent_player = "BLACK" if is_white else "WHITE"
    temp_state = GameState(
        player=opponent_player,
        my_time=0.0,
        opp_time=0.0,
        board=board
    )

    # Simulate all enemy moves
    enemy_moves = genMoves(temp_state)

    for move in enemy_moves:
        if move.dr == prince_r and move.dc == prince_c:
            return True

    return False

def genMoves(state: GameState) -> List[Move]:
    """Generate all possible moves for the current player."""
    me_is_white = state.me_is_white
    moves: List[Move] = []

    for r in range(BOARD_SIZE):
        for c in range(BOARD_SIZE):
            piece = state.board[r][c]
            if piece == '.' or not isFriendly(piece, me_is_white):
                continue
            
            p = piece.upper()
            if p == 'B':
                moves.extend(genBabyMoves(state.board, r, c, me_is_white))
            elif p == 'P':
                moves.extend(genPrinceMoves(state.board, r, c, me_is_white))
            elif p == 'X':
                moves.extend(genPrincessMoves(state.board, r, c, me_is_white))
            elif p == 'Y':
                moves.extend(genPonyMoves(state.board, r, c, me_is_white))
            elif p == 'G':
                moves.extend(genGuardMoves(state.board, r, c, me_is_white))
            elif p == 'T':
                moves.extend(genTutorMoves(state.board, r, c, me_is_white))
            elif p == 'S':
                moves.extend(genScoutMoves(state.board, r, c, me_is_white))
            elif p == 'N':
                moves.extend(genSiblingMoves(state.board, r, c, me_is_white))
    return moves

def addMove(board: List[List[str]], sr: int, sc: int, dr: int, dc: int, me_is_white: bool, moves: List[Move]) -> None:
    if not inBound(dr, dc):
        return
    
    dest_piece = board[dr][dc]
    if isFriendly(dest_piece, me_is_white):
        return
    
    moves.append(Move(sr=sr, sc=sc, dr=dr, dc=dc))

def applyMove(board, move) -> List[List[str]]:
    new_board = [row[:] for row in board]
    piece = new_board[move.sr][move.sc]
    new_board[move.sr][move.sc] = '.'
    new_board[move.dr][move.dc] = piece

    return new_board

def rayMoves(board, r, c, me_is_white, directions, maxSteps):
    moves: List[Move] = []
    for dr, dc in directions:
        nr, nc = r, c
        for _ in range(maxSteps):
            nr += dr
            nc += dc
            if not inBound(nr, nc) or isFriendly(board[nr][nc], me_is_white):
                break
            
            moves.append(Move(r, c, nr, nc))

            if isEnemy(board[nr][nc], me_is_white):
                break
    return moves

def genBabyMoves(board: List[List[str]], r: int, c: int, me_is_white: bool) -> List[Move]:
    moves: List[Move] = []
    directions = -1 if me_is_white else 1
    
    # 1 step forward
    r1 = r + directions
    if inBound(r1, c) and not isFriendly(board[r1][c], me_is_white):
        moves.append(Move(r, c, r1, c))
    # 2 steps forward
    r2 = r + 2 * directions
    if inBound(r2, c) and board[r1][c] == '.' and not isFriendly(board[r2][c], me_is_white):
        moves.append(Move(r, c, r2, c))
    
    return moves

def genPrinceMoves(board: List[List[str]], r: int, c: int, me_is_white: bool) -> List[Move]:
    moves: List[Move] = []
    for dr in range(-1, 2):
        for dc in range(-1, 2):
            if dr == 0 and dc == 0:
                continue
            addMove(board, r, c, r + dr, c + dc, me_is_white, moves)
    return moves

def genPrincessMoves(board, r, c, me_is_white):
    directions = [(-1, -1), (-1, 1), (1, -1), (1, 1), (-1, 0), (1, 0), (0, -1), (0, 1)]
    return rayMoves(board, r, c, me_is_white, directions, 3)

def genGuardMoves(board, r, c, me_is_white):
    directions = [(-1, 0), (0, -1), (1, 0), (0, 1)]
    return rayMoves(board, r, c, me_is_white, directions, 2)

def genTutorMoves(board, r, c, me_is_white):
    directions = [(-1, -1), (-1, 1), (1, -1), (1, 1)]
    return rayMoves(board, r, c, me_is_white, directions, 2)

def genPonyMoves(board, r, c, me_is_white):
    directions = [(-1, -1), (-1, 1), (1, -1), (1, 1)]
    return rayMoves(board, r, c, me_is_white, directions, 1)

def genScoutMoves(board, r, c, me_is_white):
    moves = []
    directions = -1 if me_is_white else 1

    for step in range(1, 4):
        nr = r + step * directions
        if not inBound(nr, c):
            break
        
        for side in (-1, 1):
            nc = c + side
            if inBound(nr, nc) and not isFriendly(board[nr][nc], me_is_white):
                moves.append(Move(r, c, nr, nc))
    return moves

def genSiblingMoves(board, r, c, me_is_white):
    moves: List[Move] = []
    directions = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]

    for dr in range(-1, 2):
        for dc in range(-1, 2):
            if dr == 0 and dc == 0:
                continue

            nr, nc = r + dr, c + dc
            if inBound(nr, nc) and not isFriendly(board[nr][nc], me_is_white):
                # Check if there's a friendly piece adjacent to the destination
                for ddr, ddc in directions:
                    nnr, nnc = nr + ddr, nc + ddc
                    # Don't check the source square itself
                    if nnr == r and nnc == c:
                        continue
                    if inBound(nnr, nnc) and isFriendly(board[nnr][nnc], me_is_white):
                        moves.append(Move(r, c, nr, nc))
                        break

    return moves

def evaluate(board: List[List[str]], is_white: bool) -> float:
    if isTerminal(board):
        white_prince_exists = findPiece(board, 'P') is not None
        black_prince_exists = findPiece(board, 'p') is not None

        if white_prince_exists and not black_prince_exists:
            # White won
            return 1000000 if is_white else -1000000
        elif black_prince_exists and not white_prince_exists:
            # Black won
            return -1000000 if is_white else 1000000
        else:
            # Draw (shouldn't happen)
            return 0

    score = 0

    # Detect if our prince is under immediate attack
    # Apply huge penalty to discourage moves that leave prince exposed
    if isPrinceUnderThreat(board, is_white):
        score -= 50000

    # ========== PRINCE SAFETY EVALUATION ==========
    # Find our prince position
    my_prince_pos = findPiece(board, 'P' if is_white else 'p')
    if my_prince_pos:
        pr, pc = my_prince_pos

        # 1. BACK RANK SAFETY BONUS (+100)
        # Explanation: Prince is safer on the back rank (row 0 for white, row 11 for black)
        # This prevents the prince from advancing into danger
        # Row 0 = index 0, Row 11 = index 11
        if (is_white and pr == 0) or (not is_white and pr == 11):
            score += 100

        # 2. FORWARD POSITION PENALTY (-200)
        # Explanation: Prince in forward positions is exposed to attacks
        # For white: rows 7-11 (pr > 6) are forward (enemy territory)
        # For black: rows 0-6 (pr < 5) are forward (enemy territory)
        # This strongly discourages advancing the prince
        if (is_white and pr > 6) or (not is_white and pr < 5):
            score -= 200

        # 3. DEFENDER BONUS (+50 per friendly piece)
        # Explanation: Friendly pieces adjacent to prince provide protection
        # Check all 8 surrounding squares for friendly pieces
        # Each defender adds +50 to encourage defensive formation
        defender_count = 0
        for dr in [-1, 0, 1]:
            for dc in [-1, 0, 1]:
                if dr == 0 and dc == 0:
                    continue  # Skip the prince's own square
                nr, nc = pr + dr, pc + dc
                if inBound(nr, nc) and isFriendly(board[nr][nc], is_white):
                    defender_count += 1
        score += defender_count * 50

        # 4. ENEMY PROXIMITY PENALTY (-200 per enemy)
        # Explanation: Enemy pieces near the prince are dangerous
        # A 3x3 zone around the prince is the "danger zone"
        # Each enemy in this zone gets a -200 penalty
        # This makes the AI want to keep enemies away from the prince
        enemy_count = 0
        for dr in range(-1, 2):
            for dc in range(-1, 2):
                if dr == 0 and dc == 0:
                    continue  # Skip the prince's own square
                nr, nc = pr + dr, pc + dc
                if inBound(nr, nc) and isEnemy(board[nr][nc], is_white):
                    enemy_count += 1
        score -= enemy_count * 200

    # ========== MATERIAL AND POSITIONAL EVALUATION ==========
    for r in range(BOARD_SIZE):
        for c in range(BOARD_SIZE):
            piece = board[r][c]
            if piece == '.':
                continue

            piece_type = piece.upper()
            piece_value = PIECE_VALUES.get(piece_type, 0)

            # Positional bonuses
            positional_bonus = 0

            # Center control bonus (4x4 center)
            if 4 <= r <= 7 and 4 <= c <= 7:
                positional_bonus += 10

            # Encourage Baby advancement
            if piece_type == 'B':
                if isWhite(piece):
                    # White babies advance toward row 0
                    positional_bonus += (BOARD_SIZE - r) * 2
                else:
                    # Black babies advance toward row 11
                    positional_bonus += r * 2

            total_value = piece_value + positional_bonus

            # Add or subtract based on whose piece it is
            if isWhite(piece):
                score += total_value if is_white else -total_value
            else:
                score += -total_value if is_white else total_value

    return score

def orderMoves(board, moves, is_white):
    def moveScore(move):
        score = 0

        # 1. Prince capture (game-ending)
        captured = board[move.dr][move.dc]
        if captured.upper() == 'P':
            return 1000000

        # 2. Checking moves (threatens enemy prince)
        new_board = applyMove(board, move)
        enemy_prince_threatened = isPrinceUnderThreat(new_board, not is_white)
        if enemy_prince_threatened:
            score += 100000

        # 3. Defensive moves (our prince is threatened)
        our_prince_threatened = isPrinceUnderThreat(board, is_white)
        if our_prince_threatened:
            # Test if this move stops the threat
            if not isPrinceUnderThreat(new_board, is_white):
                score += 50000

        # 4. MVV-LVA captures (Most Valuable Victim - Least Valuable Attacker)
        if captured != '.':
            victim_value = PIECE_VALUES.get(captured.upper(), 0)
            attacker_value = PIECE_VALUES.get(board[move.sr][move.sc].upper(), 0)
            score += victim_value * 10 - attacker_value

        # 5. Center control (secondary)
        if 4 <= move.dr <= 7 and 4 <= move.dc <= 7:
            score += 20

        return score

    return sorted(moves, key=moveScore, reverse=True)

def minimax(board: List[List[str]], depth: int, alpha: float, beta: float,
            is_white: bool, maximizing: bool, start_time: float, time_limit: float) -> Tuple[float, Optional[Move]]:

    # Check for timeout at the start of each call
    if time.process_time() - start_time >= time_limit:
        raise SearchTimeout()

    # Terminal state or depth limit
    if depth == 0 or isTerminal(board):
        return evaluate(board, is_white), None

    # Generate moves for current player based on the board state
    temp_state = GameState(
        player="WHITE" if maximizing == is_white else "BLACK",
        my_time=0,
        opp_time=0,
        board=board
    )
    moves = genMoves(temp_state)

    if not moves:
        # No legal moves - treat as loss
        return -1000000 if maximizing else 1000000, None

    # Order moves for better pruning
    moves = orderMoves(board, moves, temp_state.me_is_white)

    best_move = None

    if maximizing:
        max_eval = -math.inf
        for move in moves:

            if time.process_time() - start_time >= time_limit:
                raise SearchTimeout()
            
            new_board = applyMove(board, move)
            eval_score, _ = minimax(new_board, depth - 1, alpha, beta, is_white, False, start_time, time_limit)

            if eval_score > max_eval:
                max_eval = eval_score
                best_move = move

            alpha = max(alpha, eval_score)
            if beta <= alpha:
                break  

        return max_eval, best_move
    else:
        min_eval = math.inf
        for move in moves:

            if time.process_time() - start_time >= time_limit:
                raise SearchTimeout()
            
            new_board = applyMove(board, move)
            eval_score, _ = minimax(new_board, depth - 1, alpha, beta, is_white, True, start_time, time_limit)

            if eval_score < min_eval:
                min_eval = eval_score
                best_move = move

            beta = min(beta, eval_score)
            if beta <= alpha:
                break  

        return min_eval, best_move

def estimatePiecesOnBoard(board: List[List[str]]) -> int:
    count = 0
    for r in range(BOARD_SIZE):
        for c in range(BOARD_SIZE):
            if board[r][c] != '.':
                count += 1
    return count

def selectMove(state: GameState) -> Move:
    """
    Select the best move via minimax.
    """
    # for early exit if only one possible move
    moves = genMoves(state)

    if not moves:
        raise ValueError("No legal moves available")

    if len(moves) == 1:
        return moves[0]

    # time remaining
    my_time = state.my_time

    # Emergency: if very low time, just return first move
    if my_time < 0.1:
        moves = orderMoves(state.board, moves, state.me_is_white)
        return moves[0]

    # Estimate game phase based on pieces remaining
    pieces_count = estimatePiecesOnBoard(state.board)

    # Opening: 24 pieces (full board), Middlegame: 12-23, Endgame: <12
    if pieces_count >= 20:
        estimated_moves_left = 40
    elif pieces_count >= 12:
        estimated_moves_left = 30
    else:
        estimated_moves_left = 20

    if my_time > 15:
        # Plenty of time: use more aggressive search
        base_time_per_move = my_time / estimated_moves_left
        time_limit = min(2.0, base_time_per_move * 1.2)
    elif my_time > 5:
        # Moderate time: balanced approach
        base_time_per_move = my_time / estimated_moves_left
        time_limit = min(1.0, base_time_per_move * 0.9)
    elif my_time > 1:
        # Low time: conservative but still searching
        base_time_per_move = my_time / (estimated_moves_left * 1.5)
        time_limit = min(0.3, base_time_per_move)
    else:
        # Critical: minimal but safe
        time_limit = my_time / 30

    # Safety cap: never exceed 8% of remaining time
    time_limit = min(time_limit, my_time * 0.08)

    # Absolute minimum for meaningful search
    time_limit = max(time_limit, 0.01)

    # Use CPU time only (what the game engine measures)
    start_time = time.process_time()
    best_move = moves[0]
    prev_depth_time = 0
    depth_before_prev = 0

    # Iterative deepening with smart depth-time prediction
    for depth in range(1, 20):
        depth_start_time = time.process_time()

        # Calculate time used so far and remaining
        time_used = depth_start_time - start_time
        time_remaining = time_limit - time_used

        # Predict how long the next depth will take based on actual branching factor
        if depth > 2 and prev_depth_time > 0 and depth_before_prev > 0:
            # Calculate actual branching factor: time(depth N) / time(depth N-1)
            branching_factor = prev_depth_time / depth_before_prev
            # Clamp to reasonable range (2.0 to 6.0 for chess variants)
            branching_factor = max(2.0, min(branching_factor, 6.0))
            estimated_next_depth_time = prev_depth_time * branching_factor
        elif depth > 1 and prev_depth_time > 0:
            # Only have one depth time: use conservative estimate
            estimated_next_depth_time = prev_depth_time * 4.0
        else:
            # First depth: small conservative estimate
            estimated_next_depth_time = 0.05

        # Stop if we don't have enough time for next depth (with 30% safety buffer)
        if depth > 1 and time_remaining < estimated_next_depth_time * 1.3:
            break

        try:
            _, move = minimax(
                state.board,
                depth,
                -math.inf,
                math.inf,
                state.me_is_white,
                True,
                start_time,
                time_limit * 0.95  # Hard time limit at 90%
            )

            # Record this depth's completion time
            depth_end_time = time.process_time()
            current_depth_time = depth_end_time - depth_start_time

            if move is not None:
                best_move = move

            # Shift time history for next iteration
            depth_before_prev = prev_depth_time
            prev_depth_time = current_depth_time

        except SearchTimeout:
            break
        except Exception:
            break

    return best_move

def main():
    state = readInput('input.txt')
    move = selectMove(state)
    writeOutput(move, 'output.txt')

if __name__ == "__main__":
    main()
