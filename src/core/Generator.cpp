#include "Generator.hpp"
#include "Solver.hpp"
#include <algorithm>
#include <vector>

namespace sudoku {

struct CellPair {
    CellPos first;
    CellPos second;
};

void Generator::generate_full_board(Board& board, std::mt19937& rng) {
    board.reset();
    // Fill first 3x3 block randomly with 1..9 to introduce good entropy quickly
    std::vector<int> first_box_digits = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::shuffle(first_box_digits.begin(), first_box_digits.end(), rng);
    int idx = 0;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            board.set_value(r, c, first_box_digits[idx++]);
        }
    }

    // Solve remainder of the board with randomized solver
    Solver::solve_random(board, rng);

    // Record solution value in each cell
    for (int r = 0; r < Board::SIZE; ++r) {
        for (int c = 0; c < Board::SIZE; ++c) {
            board.set_solution(r, c, board.get_value(r, c));
        }
    }
}

void Generator::dig_clues(Board& board, Difficulty difficulty, std::mt19937& rng) {
    int target_clues = difficulty_target_clues(difficulty);

    // Build symmetric pairs (180-degree rotational symmetry)
    std::vector<CellPair> pairs;
    for (int r = 0; r < Board::SIZE; ++r) {
        for (int c = 0; c < Board::SIZE; ++c) {
            int opp_r = 8 - r;
            int opp_c = 8 - c;
            if (r < opp_r || (r == opp_r && c <= opp_c)) {
                pairs.push_back({{r, c}, {opp_r, opp_c}});
            }
        }
    }

    std::shuffle(pairs.begin(), pairs.end(), rng);

    int current_clues = Board::TOTAL_CELLS;

    // Phase 1: Symmetric removal
    for (const auto& pair : pairs) {
        if (current_clues <= target_clues) break;

        int val1 = board.get_value(pair.first.row, pair.first.col);
        int val2 = board.get_value(pair.second.row, pair.second.col);

        board.clear_value(pair.first.row, pair.first.col);
        if (pair.first != pair.second) {
            board.clear_value(pair.second.row, pair.second.col);
        }

        // Test uniqueness
        if (Solver::count_solutions(board, 2) == 1) {
            current_clues -= (pair.first == pair.second ? 1 : 2);
        } else {
            // Restore values
            board.set_value(pair.first.row, pair.first.col, val1);
            if (pair.first != pair.second) {
                board.set_value(pair.second.row, pair.second.col, val2);
            }
        }
    }

    // Phase 2: Asymmetric refinement if needed for Hard/Expert
    if (current_clues > target_clues) {
        std::vector<CellPos> remaining_cells;
        for (int r = 0; r < Board::SIZE; ++r) {
            for (int c = 0; c < Board::SIZE; ++c) {
                if (board.get_value(r, c) != 0) {
                    remaining_cells.push_back({r, c});
                }
            }
        }
        std::shuffle(remaining_cells.begin(), remaining_cells.end(), rng);

        for (const auto& pos : remaining_cells) {
            if (current_clues <= target_clues) break;

            int old_val = board.get_value(pos.row, pos.col);
            board.clear_value(pos.row, pos.col);

            if (Solver::count_solutions(board, 2) == 1) {
                current_clues--;
            } else {
                board.set_value(pos.row, pos.col, old_val);
            }
        }
    }

    // Mark remaining filled cells as given
    for (int r = 0; r < Board::SIZE; ++r) {
        for (int c = 0; c < Board::SIZE; ++c) {
            if (board.get_value(r, c) != 0) {
                board.set_given(r, c, true);
            } else {
                board.set_given(r, c, false);
            }
        }
    }
}

Board Generator::generate(Difficulty difficulty, unsigned int seed) {
    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
    Board board;
    generate_full_board(board, rng);
    dig_clues(board, difficulty, rng);
    return board;
}

} // namespace sudoku

