#pragma once

#include "Board.hpp"
#include <random>

namespace sudoku {

class Solver {
public:
    // Solve the given board (in-place). Returns true if a solution exists.
    static bool solve(Board& board);

    // Solve using randomized candidate ordering (used for generating random seed boards)
    static bool solve_random(Board& board, std::mt19937& rng);

    // Count solutions up to max_count (default 2).
    // Returns: 0 (unsolvable), 1 (unique solution), >= 2 (multiple solutions)
    static int count_solutions(const Board& board, int max_count = 2);

    // Check if the board has a unique solution
    static bool has_unique_solution(const Board& board) {
        return count_solutions(board, 2) == 1;
    }

private:
    // Core backtracking using MRV (Minimum Remaining Values)
    static bool backtrack_mrv(Board& board);
    static bool backtrack_mrv_random(Board& board, std::mt19937& rng);
    static void count_backtrack_mrv(Board& board, int& count, int max_count);

    // Find cell with minimum remaining values (MRV)
    // Returns false if all filled (solved) or true with (best_r, best_c)
    static bool find_mrv_cell(const Board& board, int& best_r, int& best_c, int& min_candidates);
};

} // namespace sudoku

