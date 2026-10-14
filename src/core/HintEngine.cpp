#include "HintEngine.hpp"
#include <sstream>

namespace sudoku {

bool HintEngine::find_naked_single(const Board& board, HintResult& hint) {
    for (int r = 0; r < Board::SIZE; ++r) {
        for (int c = 0; c < Board::SIZE; ++c) {
            if (board.get_value(r, c) == 0) {
                uint16_t mask = board.get_candidate_mask(r, c);
                if (__builtin_popcount(mask) == 1) {
                    int digit = __builtin_ctz(mask);
                    hint.found = true;
                    hint.pos = {r, c};
                    hint.digit = digit;
                    hint.technique = "Naked Single";

                    std::ostringstream ss;
                    ss << "R" << (r + 1) << "C" << (c + 1) << " must be " << digit
                       << " (all other digits 1-9 are blocked by its row, col, or box).";
                    hint.explanation = ss.str();
                    return true;
                }
            }
        }
    }
    return false;
}

bool HintEngine::find_hidden_single(const Board& board, HintResult& hint) {
    // 1. Check Rows
    for (int r = 0; r < Board::SIZE; ++r) {
        for (int d = 1; d <= 9; ++d) {
            uint16_t bit = static_cast<uint16_t>(1 << d);
            if ((board.get_row_mask(r) & bit) != 0) continue;

            int match_count = 0;
            int match_col = -1;
            for (int c = 0; c < Board::SIZE; ++c) {
                if (board.get_value(r, c) == 0 && (board.get_candidate_mask(r, c) & bit) != 0) {
                    match_count++;
                    match_col = c;
                }
            }

            if (match_count == 1) {
                hint.found = true;
                hint.pos = {r, match_col};
                hint.digit = d;
                hint.technique = "Hidden Single (Row)";
                std::ostringstream ss;
                ss << "In Row " << (r + 1) << ", digit " << d << " can only fit in Column " << (match_col + 1) << ".";
                hint.explanation = ss.str();
                return true;
            }
        }
    }

    // 2. Check Columns
    for (int c = 0; c < Board::SIZE; ++c) {
        for (int d = 1; d <= 9; ++d) {
            uint16_t bit = static_cast<uint16_t>(1 << d);
            if ((board.get_col_mask(c) & bit) != 0) continue;

            int match_count = 0;
            int match_row = -1;
            for (int r = 0; r < Board::SIZE; ++r) {
                if (board.get_value(r, c) == 0 && (board.get_candidate_mask(r, c) & bit) != 0) {
                    match_count++;
                    match_row = r;
                }
            }

            if (match_count == 1) {
                hint.found = true;
                hint.pos = {match_row, c};
                hint.digit = d;
                hint.technique = "Hidden Single (Column)";
                std::ostringstream ss;
                ss << "In Column " << (c + 1) << ", digit " << d << " can only fit in Row " << (match_row + 1) << ".";
                hint.explanation = ss.str();
                return true;
            }
        }
    }

    // 3. Check 3x3 Boxes
    for (int b = 0; b < Board::SIZE; ++b) {
        for (int d = 1; d <= 9; ++d) {
            uint16_t bit = static_cast<uint16_t>(1 << d);
            if ((board.get_box_mask(b) & bit) != 0) continue;

            int match_count = 0;
            CellPos match_pos{0, 0};
            int start_r = (b / 3) * 3;
            int start_c = (b % 3) * 3;

            for (int r = start_r; r < start_r + 3; ++r) {
                for (int c = start_c; c < start_c + 3; ++c) {
                    if (board.get_value(r, c) == 0 && (board.get_candidate_mask(r, c) & bit) != 0) {
                        match_count++;
                        match_pos = {r, c};
                    }
                }
            }

            if (match_count == 1) {
                hint.found = true;
                hint.pos = match_pos;
                hint.digit = d;
                hint.technique = "Hidden Single (Box)";
                std::ostringstream ss;
                ss << "In Box " << (b + 1) << ", digit " << d << " can only fit at R"
                   << (match_pos.row + 1) << "C" << (match_pos.col + 1) << ".";
                hint.explanation = ss.str();
                return true;
            }
        }
    }

    return false;
}

bool HintEngine::find_solution_hint(const Board& board, HintResult& hint) {
    int best_r = -1, best_c = -1;
    int min_cands = 10;

    for (int r = 0; r < Board::SIZE; ++r) {
        for (int c = 0; c < Board::SIZE; ++c) {
            if (board.get_value(r, c) == 0) {
                int count = board.count_candidates(r, c);
                if (count > 0 && count < min_cands) {
                    min_cands = count;
                    best_r = r;
                    best_c = c;
                }
            }
        }
    }

    if (best_r != -1) {
        hint.found = true;
        hint.pos = {best_r, best_c};
        hint.digit = board.get_solution(best_r, best_c);
        hint.technique = "Deduction";
        std::ostringstream ss;
        ss << "At R" << (best_r + 1) << "C" << (best_c + 1)
           << ", logical analysis yields digit " << hint.digit << ".";
        hint.explanation = ss.str();
        return true;
    }

    return false;
}

HintResult HintEngine::get_hint(const Board& board) {
    HintResult hint;
    if (find_naked_single(board, hint)) return hint;
    if (find_hidden_single(board, hint)) return hint;
    if (find_solution_hint(board, hint)) return hint;
    return hint;
}

} // namespace sudoku

