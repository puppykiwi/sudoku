#include "Solver.hpp"
#include <algorithm>
#include <vector>

namespace sudoku {

bool Solver::find_mrv_cell(const Board& board, int& best_r, int& best_c, int& min_candidates) {
    best_r = -1;
    best_c = -1;
    min_candidates = 10;

    for (int r = 0; r < Board::SIZE; ++r) {
        for (int c = 0; c < Board::SIZE; ++c) {
            if (board.get_value(r, c) == 0) {
                int count = board.count_candidates(r, c);
                if (count == 0) {
                    // Contradiction: empty cell with no possible candidate
                    min_candidates = 0;
                    best_r = r;
                    best_c = c;
                    return true;
                }
                if (count < min_candidates) {
                    min_candidates = count;
                    best_r = r;
                    best_c = c;
                    if (min_candidates == 1) {
                        // Optimal: naked single
                        return true;
                    }
                }
            }
        }
    }

    return (best_r != -1);
}

bool Solver::backtrack_mrv(Board& board) {
    int r = -1, c = -1, count = 10;
    if (!find_mrv_cell(board, r, c, count)) {
        // No empty cells remaining -> board is completely and validly solved
        return true;
    }

    if (count == 0) {
        // Dead end encountered
        return false;
    }

    uint16_t cand_mask = board.get_candidate_mask(r, c);
    for (int d = 1; d <= 9; ++d) {
        if ((cand_mask & (1 << d)) != 0) {
            board.set_value(r, c, d);
            if (backtrack_mrv(board)) {
                return true;
            }
            board.clear_value(r, c);
        }
    }

    return false;
}

bool Solver::backtrack_mrv_random(Board& board, std::mt19937& rng) {
    int r = -1, c = -1, count = 10;
    if (!find_mrv_cell(board, r, c, count)) {
        return true;
    }

    if (count == 0) {
        return false;
    }

    auto candidates = board.get_candidate_list(r, c);
    std::shuffle(candidates.begin(), candidates.end(), rng);

    for (int d : candidates) {
        board.set_value(r, c, d);
        if (backtrack_mrv_random(board, rng)) {
            return true;
        }
        board.clear_value(r, c);
    }

    return false;
}

void Solver::count_backtrack_mrv(Board& board, int& count, int max_count) {
    if (count >= max_count) {
        return;
    }

    int r = -1, c = -1, cands = 10;
    if (!find_mrv_cell(board, r, c, cands)) {
        // Found a valid solution
        count++;
        return;
    }

    if (cands == 0) {
        // Dead end
        return;
    }

    uint16_t cand_mask = board.get_candidate_mask(r, c);
    for (int d = 1; d <= 9; ++d) {
        if ((cand_mask & (1 << d)) != 0) {
            board.set_value(r, c, d);
            count_backtrack_mrv(board, count, max_count);
            board.clear_value(r, c);

            if (count >= max_count) {
                return;
            }
        }
    }
}

bool Solver::solve(Board& board) {
    return backtrack_mrv(board);
}

bool Solver::solve_random(Board& board, std::mt19937& rng) {
    return backtrack_mrv_random(board, rng);
}

int Solver::count_solutions(const Board& board, int max_count) {
    Board copy;
    copy.copy_from(board);
    int count = 0;
    count_backtrack_mrv(copy, count, max_count);
    return count;
}

} // namespace sudoku

