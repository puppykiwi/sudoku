#pragma once

#include "Types.hpp"
#include <array>
#include <string>
#include <vector>
#include <cstdint>

namespace sudoku {

struct Cell {
    int value{0};           // 0 = empty, 1..9 = filled
    int solution{0};        // Correct solution digit (1..9)
    bool is_given{false};   // True if part of original puzzle clues
    uint16_t notes{0};      // Bitmask for pencil notes (bits 1..9)
    bool has_error{false};  // Visual error indicator (3 strikes)

    bool has_note(int digit) const {
        if (digit < 1 || digit > 9) return false;
        return (notes & (1 << digit)) != 0;
    }

    void toggle_note(int digit) {
        if (digit >= 1 && digit <= 9) {
            notes ^= (1 << digit);
        }
    }

    void remove_note(int digit) {
        if (digit >= 1 && digit <= 9) {
            notes &= ~(1 << digit);
        }
    }

    void clear_notes() {
        notes = 0;
    }
};

class Board {
public:
    static constexpr int SIZE = 9;
    static constexpr int BOX_SIZE = 3;
    static constexpr int TOTAL_CELLS = 81;

    Board();

    void reset();

    // Cell access
    const Cell& get_cell(int r, int c) const { return grid_[r][c]; }
    Cell& get_cell_mut(int r, int c) { return grid_[r][c]; }

    int get_value(int r, int c) const { return grid_[r][c].value; }
    void set_value(int r, int c, int val);
    void clear_value(int r, int c);

    int get_solution(int r, int c) const { return grid_[r][c].solution; }
    void set_solution(int r, int c, int sol) { grid_[r][c].solution = sol; }

    bool is_given(int r, int c) const { return grid_[r][c].is_given; }
    void set_given(int r, int c, bool given) { grid_[r][c].is_given = given; }

    uint16_t get_notes(int r, int c) const { return grid_[r][c].notes; }
    void set_notes(int r, int c, uint16_t notes) { grid_[r][c].notes = notes; }
    void toggle_note(int r, int c, int digit);
    void remove_note(int r, int c, int digit);
    void clear_notes(int r, int c);

    bool has_error(int r, int c) const { return grid_[r][c].has_error; }
    void set_error(int r, int c, bool err) { grid_[r][c].has_error = err; }
    void clear_all_errors();

    // Constraint calculations
    static int get_box(int r, int c) { return (r / 3) * 3 + (c / 3); }
    bool is_valid_placement(int r, int c, int val) const;
    uint16_t get_candidate_mask(int r, int c) const;
    int count_candidates(int r, int c) const;
    std::vector<int> get_candidate_list(int r, int c) const;

    // Board analysis
    bool is_solved() const;
    bool is_full() const;
    int count_clues() const;
    int count_filled() const;
    int count_digit_occurrences(int digit) const;
    bool is_digit_completed(int digit) const { return count_digit_occurrences(digit) == 9; }

    // Peer cells (row, col, 3x3 box peers)
    std::vector<CellPos> get_peer_cells(int r, int c) const;
    // Remove note from all peers when a digit is confirmed
    std::vector<std::pair<CellPos, uint16_t>> remove_note_from_peers(int r, int c, int digit);

    // Conflict detection
    std::vector<CellPos> find_conflicts() const;

    // String import/export
    std::string to_string() const;
    bool load_from_string(const std::string& str);

    // Fast copy for solver
    void copy_from(const Board& other);

    // Direct mask access for solver performance
    uint16_t get_row_mask(int r) const { return row_masks_[r]; }
    uint16_t get_col_mask(int c) const { return col_masks_[c]; }
    uint16_t get_box_mask(int b) const { return box_masks_[b]; }

private:
    void recompute_masks();

    std::array<std::array<Cell, SIZE>, SIZE> grid_;
    std::array<uint16_t, SIZE> row_masks_{};
    std::array<uint16_t, SIZE> col_masks_{};
    std::array<uint16_t, SIZE> box_masks_{};
};

} // namespace sudoku
