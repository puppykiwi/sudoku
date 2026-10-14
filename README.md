# Modern C++ Sudoku

A high-performance, feature-packed Sudoku game written in modern **C++17** with a terminal user interface (TUI) powered by `ncursesw`.

All core algorithms—including **puzzle generation, backtracking solver with constraint propagation, uniqueness verification, and logical deduction hints**—are **built completely from scratch with zero algorithmic libraries**.

---

## Features

- **Standard 9x9 Sudoku Rules**: 9 rows, 9 columns, and 9 $3\times3$ non-overlapping subgrids.
- **Custom Backtracking Solver**:
  - Implements the **Minimum Remaining Values (MRV)** heuristic to branch on cells with the fewest candidate digits first.
  - Bitwise constraint propagation using 16-bit masks for $O(1)$ candidate lookups.
  - Multi-solution counter for fast uniqueness verification ($\le 1\text{ ms}$).
- **Unique Puzzle Generator**:
  - Seeds random valid full grids using randomized backtracking.
  - Digs out clues while maintaining 180° rotational symmetry and mathematically guaranteeing **exactly one unique solution**.
  - 4 tuned difficulty tiers:
    - **Easy**: 40 clues (singles-only solving flow).
    - **Medium**: 34 clues (naked & hidden singles).
    - **Hard**: 28 clues (requires pairs, intersections, and deeper deductions).
    - **Expert**: 24 clues (advanced branching and master-level logic).
- **3-Strikes Mode**:
  - Direct error alerting: entering an incorrect digit in Normal mode incurs a Strike.
  - Reaching 3 strikes results in Game Over.
  - Safe **Pencil Notes** mode (`N` key) lets you pencil candidate digits (1–9) without penalty.
- **Smart Logical Hint Engine**:
  - Detects **Naked Singles** and **Hidden Singles** (by row, column, and $3\times3$ box).
  - Explains the step-by-step logic behind each deduction.
- **Dynamic Leaderboard & High Scores**:
  - Persistent storage in JSON (`~/.config/sudoku/highscores.json` with local fallback).
  - Dynamic score formula factoring base difficulty points, elapsed completion time, mistakes, and hint penalties, plus bonuses for flawless runs.
  - Hall of Fame screen with top ranks per difficulty.
- **Full Undo / Redo & Move History**:
  - Reverts moves, cell erasures, and candidate notes, restoring peer cell notes cleanly.
- **6 Modern Color Themes**:
  - **Tokyo Night**: Cyber-navy indigo with vivid cyan, lime, and purple accents.
  - **Dracula**: Classic dark slate with hot pink, cyan, and mint green highlights.
  - **Nord Arctic**: Glacial bluish gray with frost cyan and snow white clarity.
  - **Cyberpunk Neon**: High-contrast obsidian black with electric yellow and magenta.
  - **Monokai Pro**: Warm charcoal with bright lime, orange, and sky blue.
  - **Paper Minimal**: Daylight warm parchment with dark charcoal typography.
- **Dual Keyboard & Mouse Support**:
  - Navigate with Arrow keys, WASD, or Vim keys (`H, J, K, L`).
  - Full mouse support: click on any cell, menu item, or side panel number keypad to select and play!

---

## Quick Start

### Build

```bash
make -j$(nproc)
```

### Run Game

```bash
./sudoku
```

### Run Test Suite

```bash
make test
```

---

## Controls

| Action | Key / Input |
|---|---|
| **Move Cursor** | Arrow Keys / `W`, `A`, `S`, `D` / Mouse Click |
| **Enter Digit** | `1` – `9` / Click Keypad on Side Panel |
| **Toggle Pencil Notes Mode** | `N` |
| **Clear Cell / Note** | `Backspace` / `Delete` / `C` / `X` / `0` |
| **Undo Move** | `U` |
| **Redo Move** | `R` |
| **Smart Logical Hint** | `H` / `?` |
| **Cycle Theme** | `T` |
| **Pause Game / Menu** | `P` / `M` / `Esc` |
| **Quit Game** | `Q` |

---

## Scoring Formula

$$\text{Final Score} = \max\left(100, \text{BasePoints} - (\text{TimeSeconds} \times 2) - (\text{Strikes} \times 300) - (\text{Hints} \times 250) + \text{Bonuses}\right)$$

- **Base Points**:
  - Easy: 1,500 pts
  - Medium: 3,500 pts
  - Hard: 6,000 pts
  - Expert: 10,000 pts
- **Bonuses**:
  - +500 pts for 0 Strikes (flawless accuracy)
  - +500 pts for 0 Hints used (pure independent solve)

---

## Project Structure

```
/opt/sudoku/
├── Makefile                 # Build system with test pipeline
├── README.md                # Documentation and controls guide
├── src/
│   ├── main.cpp             # Main application lifecycle and loop
│   ├── core/
│   │   ├── Types.hpp        # Data models, difficulty, move structures
│   │   ├── Board.hpp/.cpp   # 9x9 board with 16-bit constraint masks
│   │   ├── Solver.hpp/.cpp  # MRV backtracking solver & uniqueness tester
│   │   ├── Generator.hpp/.cpp # Unique puzzle generator with 180° symmetry
│   │   └── HintEngine.hpp/.cpp # Naked/Hidden single deduction engine
│   ├── game/
│   │   ├── GameEngine.hpp/.cpp # Game loop, state machine, 3-strikes rules
│   │   ├── ScoreManager.hpp/.cpp # High scores calculation & JSON storage
│   │   └── Theme.hpp/.cpp   # 6 Modern theme definitions & color pair management
│   └── ui/
│       ├── Renderer.hpp/.cpp # Double/light Unicode box rendering & scenes
│       └── Input.hpp/.cpp   # Keyboard & mouse event handler
└── tests/
    ├── test_solver.cpp      # Unit tests for solver, generator & uniqueness
    └── test_gameplay.cpp    # Unit tests for 3-strikes, scoring & undo/redo
```

