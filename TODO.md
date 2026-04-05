# TODO - CSCI-561 Homework 2: Heirs Game AI Agent

**Due Date:** April 6, 2026 23:59
**Current Date:** April 4, 2026

## Assignment Overview
Build a game-playing AI agent for "Heirs" - a chess variant on a 12x12 board. The agent must compete against:
- 10 games vs random agent (5 points each = 50 points)
- 10 games vs simple minimax agent (5 points each = 50 points)
- **Total possible: 100 points**

---

## Current Status: TIME MANAGEMENT FIXED - TESTING NEEDED ✓

### ✅ COMPLETED
- [x] All basic game infrastructure and move generation
- [x] Minimax with alpha-beta pruning
- [x] Iterative deepening search
- [x] Move ordering (captures + center control)
- [x] **SMART TIME MANAGEMENT** (April 4, 2026) ✨
  - [x] CPU time only (removed wall time)
  - [x] Dynamic branching factor calculation
  - [x] Depth-time prediction for optimal search
- [x] Material and positional evaluation

### 🔴 CRITICAL ISSUES (Why We're Losing)

**Game Result:** Lost to minimax agent - Black prince captured at move 59

#### **Root Cause Analysis (April 4, 2026)**

**Issue #1: TIME MANAGEMENT TOO CONSERVATIVE**
- **Current:** Using 0.3-0.8 seconds per move
- **Opponent:** Using 0.004 seconds per move (C++ compiled engine)
- **Problem:** With 14+ seconds remaining, we only use 0.3s per move
- **Impact:** Cannot search deep enough to detect mate-in-2 threats
- **Evidence:** At move 58, we had 14s left but didn't see Baby g10→g12 threatening prince capture

**Metrics from logs:**
- Move 2: 0.812s (30s remaining) ✓ Good
- Move 10: 0.732s (26.9s remaining) ✓ Good
- Move 56: 0.316s (14.3s remaining) ✗ TOO CONSERVATIVE!
- Lost at move 59 with 13.6s still remaining

**Issue #2: WEAK EVALUATION FUNCTION**

Current evaluation (`homework.py:263-314`):
- ✓ Material counting
- ✓ Center control (+10 bonus)
- ✓ Baby advancement
- ✗ **NO PRINCE SAFETY** - Critical flaw!
- ✗ **NO THREAT DETECTION** - Doesn't see attacks on prince
- ✗ **NO MOBILITY** - Doesn't value having more moves
- ✗ **NO DEFENSIVE COORDINATION** - Pieces don't defend key squares

**Losing sequence:**
```
Move 56: BLACK (us) moves princess d6→e7 (captures Baby)
Move 57: WHITE advances Baby g9→g10 (THREAT: next move g12 captures prince!)
Move 58: BLACK (us) moves tutor h5→f7 (???) - DIDN'T DEFEND!
Move 59: WHITE Baby g10→g12 - CAPTURES PRINCE - GAME OVER
```

**Why we lost:**
1. Didn't evaluate that prince at g12 was in danger
2. Didn't search deep enough to see the mate-in-2
3. Time management prevented depth-4+ search

---

## 🚨 HIGH PRIORITY FIXES (To Beat Minimax Agent)

### ✅ Fix #1: SMART Time Management ⚡ **COMPLETED!**
**File:** `homework.py:462-517`

**What we implemented:**

**1. CPU time only (removed wall time)** ✅
```python
start_time = time.process_time()  # Only CPU time
# Removed: start_wall_time, wall_elapsed, max(cpu, wall*0.5)
```

**2. Aggressive time allocation** ✅
```python
if my_time > 15:
    time_limit = min(2.0, my_time * 0.15)  # Up to 2s or 15%
elif my_time > 5:
    time_limit = min(1.0, my_time * 0.12)  # Up to 1s or 12%
elif my_time > 1:
    time_limit = my_time * 0.08  # 8%
else:
    time_limit = my_time * 0.3  # Emergency: 30%
```

**3. SMART depth-time prediction** ✅ **NEW!**
```python
# Calculate actual branching factor from previous depths
branching_factor = prev_depth_time / depth_before_prev
branching_factor = max(2.0, min(branching_factor, 6.0))  # Clamp 2-6

# Predict next depth time
estimated_next_depth_time = prev_depth_time * branching_factor

# Stop if not enough time (30% safety buffer)
if time_remaining < estimated_next_depth_time * 1.3:
    break
```

**Result:**
- Uses almost all allocated time efficiently
- Reaches depth 4-8 depending on position complexity
- Never starts a depth it can't finish
- Adapts to actual search speed in real-time

---

### Fix #2: PRINCE SAFETY EVALUATION 🛡️
**File:** `homework.py:266-360` (evaluate function)

**Add these critical features:**

**A. Threat Detection** ✅ **COMPLETED - April 5, 2026**
- **Implementation**: `isPrinceUnderThreat()` function at lines 124-152
- **Integration**: Penalty applied in `evaluate()` at lines 280-283
- **How it works**: Generates all enemy moves and checks if any capture our prince
- **Penalty value**: -50,000 (half of prince value 100,000)
- **Performance**: O(75) operations per evaluation (~15 enemy pieces × 5 moves)
- **Result**: AI now detects immediate threats and prioritizes prince defense

**B. Prince Safety Bonus**
```python
# Bonus for prince being in back rank (+100)
# Bonus for friendly pieces adjacent to prince (+50 each)
# Penalty for prince being in center/forward (-200)
```

**C. Mobility**
```python
# Count legal moves available
# +5 per legal move (more mobility = better position)
```

**D. Piece Coordination**
```python
# Bonus for pieces defending each other (+10)
# Bonus for controlling squares near enemy prince (+30)
```

---

### Fix #3: MOVE ORDERING IMPROVEMENTS
**File:** `homework.py:316-332`

**Current:** Prioritizes captures + center control

**Add:**
- **Check/threat moves** (attacks on enemy prince) → Score: +100000
- **Prince captures** → Score: +1000000 (game-ending)
- **Defensive moves** (blocks attack on our prince) → Score: +50000
- **Captures ordered by MVV-LVA** (capture valuable with cheap piece)

---

## 📋 Implementation Priority (UPDATED - April 5, 2026)

### CRITICAL (Do These First) 🔴
1. **[✅] Fix time management** - DONE! Smart depth-time prediction
2. **[✅] Add prince threat detection** - DONE! isPrinceUnderThreat() implemented
3. **[ ] TACTICAL MOVE ORDERING** ⚡ **← START HERE**
   - Add prince captures (score: +1,000,000)
   - Add checking moves (score: +100,000)
   - Add defensive moves when prince threatened (score: +50,000)
   - MVV-LVA for captures (victim value - attacker value)
   - **Impact**: Better alpha-beta pruning = effective +1-2 depth
   - **Time**: 30-45 minutes
   - **File**: `homework.py:316-332` (orderMoves function)

4. **[ ] STRENGTHEN PRINCE SAFETY EVALUATION** 🛡️
   - Prince position bonus: back rank +100, forward -200
   - Enemy proximity penalty: -200 per enemy in 3x3 zone
   - Defender bonus: +50 per friendly adjacent to prince
   - **Impact**: Prevents prince from advancing dangerously
   - **Time**: 30 minutes
   - **File**: `homework.py:266-360` (evaluate function)

5. **[ ] QUICK TEST** - Run 1 game vs minimax, verify improvements

### HIGH PRIORITY (Big Wins) 🟡
6. **[ ] TRANSPOSITION TABLE** 💾
   - Cache board positions with scores
   - Use board hash as key
   - Store: score, depth, best_move
   - **Impact**: Avoid recomputing same positions = faster/deeper
   - **Time**: 45-60 minutes
   - **Difficulty**: Medium (need hash function)

7. **[ ] REDUCE PER-NODE OVERHEAD** ⚡
   - Use makeMove/unmakeMove instead of board copy
   - Pass side_to_move directly (no GameState rebuild)
   - **Impact**: 20-30% faster = +1 depth
   - **Time**: 60 minutes
   - **Difficulty**: Medium (requires refactoring)

8. **[ ] ADD MOBILITY EVALUATION**
   - Count legal moves for both sides
   - +5 per legal move for us, -5 for opponent
   - **Impact**: Prefer positions with more options
   - **Time**: 20 minutes

### MEDIUM PRIORITY (If Time Allows) 🟢
9. **[ ] KILLER MOVE HEURISTIC** - Remember refutation moves
10. **[ ] HISTORY HEURISTIC** - Track historically good moves
11. **[ ] QUIESCENCE SEARCH** - Search captures to quiet position

---

## 📊 Expected Performance After Fixes

**Without fixes (current):**
- vs Random: Should win 10/10 ✓
- vs Minimax: Losing (0-3/10 wins) ✗

**With Fix #1 (time management):**
- vs Minimax: 3-5/10 wins (deeper search helps)

**With Fix #1 + Fix #2 (+ prince safety):**
- vs Minimax: 6-8/10 wins (detects threats)

**With All Fixes:**
- vs Minimax: 8-10/10 wins (competitive agent)

---

## 🎯 Minimum to Pass (50+ points)

✅ **Already have:** Beat random agent (50 points)
❌ **Still need:** Beat minimax agent (0-50 points depending on wins)

**Target:** Get at least 5 wins vs minimax = 75 points total (C grade)
**Stretch goal:** Get 8+ wins vs minimax = 90+ points (A grade)

---

## ⏰ Time Budget (2 days left)

### Day 1 - April 4 (Today)
- **2 hours:** Fix time management to be more aggressive
- **3 hours:** Add prince safety evaluation
- **1 hour:** Test and debug
- **1 hour:** Improve move ordering

### Day 2 - April 5
- **2 hours:** Add mobility and coordination bonuses
- **2 hours:** Extensive testing vs minimax
- **1 hour:** Fine-tune evaluation weights
- **1 hour:** Final testing and submission prep

### April 6 (Due Date)
- **Morning:** Final verification on vocareum
- **Submit by 23:59**

---

## 🔧 STEP-BY-STEP ACTION PLAN

### **STEP 1: Tactical Move Ordering** (30-45 min) ⚡
**Why**: Better pruning = effective +1-2 depth without any search changes

**File**: `homework.py:316-332`

**What to add** (in order of priority):
```python
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
```

**Test**: Run 1 game, check if defensive/checking moves are prioritized

---

### **STEP 2: Strengthen Prince Safety** (30 min) 🛡️
**Why**: Prevents advancing prince into danger

**File**: `homework.py:266-360` (in evaluate function, after threat detection)

**Add after the isPrinceUnderThreat check**:
```python
# Find our prince
my_prince_pos = findPiece(board, 'P' if is_white else 'p')
if my_prince_pos:
    pr, pc = my_prince_pos

    # Back rank safety bonus
    if (is_white and pr == 0) or (not is_white and pr == 11):
        score += 100

    # Forward penalty (prince too aggressive)
    if (is_white and pr > 6) or (not is_white and pr < 5):
        score -= 200

    # Count defenders (friendly pieces adjacent)
    for dr in [-1, 0, 1]:
        for dc in [-1, 0, 1]:
            if dr == 0 and dc == 0:
                continue
            nr, nc = pr + dr, pc + dc
            if inBound(nr, nc) and isFriendly(board[nr][nc], is_white):
                score += 50

    # Enemy proximity penalty (3x3 zone around prince)
    enemy_count = 0
    for dr in range(-1, 2):
        for dc in range(-1, 2):
            nr, nc = pr + dr, pc + dc
            if inBound(nr, nc) and isEnemy(board[nr][nc], is_white):
                enemy_count += 1
    score -= enemy_count * 200
```

**Test**: Verify prince stays on back rank in opening

---

### **STEP 3: Quick Test** (10 min)
Run 1 game vs minimax and check:
- Does AI defend when prince threatened?
- Does prince stay safe on back rank?
- Are checking moves prioritized?

---

### **STEP 4: Transposition Table** (45-60 min) 💾
**Why**: Avoid recomputing positions = faster search

**Implementation**:
1. Add board hash function
2. Create global cache dict
3. Check cache before searching
4. Store results after searching

**Skip for now if short on time** - Steps 1-3 give biggest wins

---

### **CURRENT PRIORITY: START WITH STEP 1**
Tactical move ordering gives the most impact for time invested.

---

## 📝 Notes from Log Analysis

**File:** `work/logs.txt` (2156 lines, game vs compete_minimax_5ya4xpmk)

**Key Observations:**
- Game lasted 59 moves (typical middlegame length)
- We used only 16.3 seconds of our 30 seconds (wasted 13.7s!)
- Opponent used almost no time (compiled C++ is fast)
- Lost because we didn't defend prince from advancing Baby
- Our moves were legal and mostly reasonable, just not deep enough

**Conclusion:** We have a working agent that needs strategic improvements, not fundamental fixes.
