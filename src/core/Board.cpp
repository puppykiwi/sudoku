#include "Board.hpp"
#include <algorithm>
#include <sstream>

namespace sudoku {

Board::Board() {
    reset();
}

void Board::reset() {
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            grid_[r][c] = Cell{};
        }
        row_masks_[r] = 0;
        col_masks_[r] = 0;
        box_masks_[r] = 0;
    }
}

void Board::recompute_masks() {
    row_masks_.fill(0);
    col_masks_.fill(0);
    box_masks_.fill(0);

    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            int val = grid_[r][c].value;
            if (val >= 1 && val <= 9) {
                uint16_t bit = static_cast<uint16_t>(1 << val);
                row_masks_[r] |= bit;
                col_masks_[c] |= bit;
                box_masks_[get_box(r, c)] |= bit;
            }
        }
    }
}

void Board::set_value(int r, int c, int val) {
    if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return;
    int old_val = grid_[r][c].value;
    if (old_val == val) return;

    if (old_val >= 1 && old_val <= 9) {
        uint16_t old_bit = static_cast<uint16_t>(1 << old_val);
        row_masks_[r] &= ~old_bit;
        col_masks_[c] &= ~old_bit;
        box_masks_[get_box(r, c)] &= ~old_bit;
    }

    grid_[r][c].value = val;

    if (val >= 1 && val <= 9) {
        uint16_t new_bit = static_cast<uint16_t>(1 << val);
        row_masks_[r] |= new_bit;
        col_masks_[c] |= new_bit;
        box_masks_[get_box(r, c)] |= new_bit;
    }
}

void Board::clear_value(int r, int c) {
    set_value(r, c, 0);
}

void Board::toggle_note(int r, int c, int digit) {
    if (r >= 0 && r < SIZE && c >= 0 && c < SIZE) {
        grid_[r][c].toggle_note(digit);
    }
}

void Board::remove_note(int r, int c, int digit) {
    if (r >= 0 && r < SIZE && c >= 0 && c < SIZE) {
        grid_[r][c].remove_note(digit);
    }
}

void Board::clear_notes(int r, int c) {
    if (r >= 0 && r < SIZE && c >= 0 && c < SIZE) {
        grid_[r][c].clear_notes();
    }
}

void Board::clear_all_errors() {
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            grid_[r][c].has_error = false;
        }
    }
}

bool Board::is_valid_placement(int r, int c, int val) const {
    if (val < 1 || val > 9) return false;
    uint16_t bit = static_cast<uint16_t>(1 << val);
    
    // Ignore current cell's existing value if testing replacement
    int current = grid_[r][c].value;
    if (current == val) return true;

    if ((row_masks_[r] & bit) != 0) return false;
    if ((col_masks_[c] & bit) != 0) return false;
    if ((box_masks_[get_box(r, c)] & bit) != 0) return false;

    return true;
}

uint16_t Board::get_candidate_mask(int r, int c) const {
    if (grid_[r][c].value != 0) return 0;
    uint16_t used = row_masks_[r] | col_masks_[c] | box_masks_[get_box(r, c)];
    return (~used) & 0x03FE; // Only digits 1..9 (bits 1 through 9)
}

int Board::count_candidates(int r, int c) const {
    uint16_t mask = get_candidate_mask(r, c);
    return __builtin_popcount(mask);
}

std::vector<int> Board::get_candidate_list(int r, int c) const {
    std::vector<int> result;
    uint16_t mask = get_candidate_mask(r, c);
    for (int d = 1; d <= 9; ++d) {
        if ((mask & (1 << d)) != 0) {
            result.push_back(d);
        }
    }
    return result;
}

bool Board::is_solved() const {
    for (int r = 0; r < SIZE; ++r) {
        if (row_masks_[r] != 0x03FE) return false;
        if (col_masks_[r] != 0x03FE) return false;
        if (box_masks_[r] != 0x03FE) return false;
    }
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (grid_[r][c].value < 1 || grid_[r][c].value > 9) return false;
        }
    }
    return true;
}

bool Board::is_full() const {
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (grid_[r][c].value == 0) return false;
        }
    }
    return true;
}

int Board::count_clues() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (grid_[r][c].is_given && grid_[r][c].value != 0) {
                count++;
            }
        }
    }
    return count;
}

int Board::count_filled() const {
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (grid_[r][c].value != 0) count++;
        }
    }
    return count;
}

int Board::count_digit_occurrences(int digit) const {
    if (digit < 1 || digit > 9) return 0;
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (grid_[r][c].value == digit) count++;
        }
    }
    return count;
}

std::vector<CellPos> Board::get_peer_cells(int r, int c) const {
    std::vector<CellPos> peers;
    peers.reserve(20);

    // Row peers
    for (int cc = 0; cc < SIZE; ++cc) {
        if (cc != c) peers.push_back({r, cc});
    }
    // Column peers
    for (int rr = 0; rr < SIZE; ++rr) {
        if (rr != r) peers.push_back({rr, c});
    }
    // Box peers (avoiding row/col duplicates)
    int start_r = (r / 3) * 3;
    int start_c = (c / 3) * 3;
    for (int rr = start_r; rr < start_r + 3; ++rr) {
        if (rr == r) continue;
        for (int cc = start_c; cc < start_c + 3; ++cc) {
            if (cc == c) continue;
            peers.push_back({rr, cc});
        }
    }
    return peers;
}

std::vector<std::pair<CellPos, uint16_t>> Board::remove_note_from_peers(int r, int c, int digit) {
    std::vector<std::pair<CellPos, uint16_t>> affected;
    if (digit < 1 || digit > 9) return affected;

    auto peers = get_peer_cells(r, c);
    for (const auto& peer : peers) {
        Cell& cell = grid_[peer.row][peer.col];
        if (cell.value == 0 && cell.has_note(digit)) {
            affected.push_back({peer, cell.notes});
            cell.remove_note(digit);
        }
    }
    return affected;
}

std::vector<CellPos> Board::find_conflicts() const {
    std::vector<CellPos> conflicts;

    // Check rows
    for (int r = 0; r < SIZE; ++r) {
        for (int c1 = 0; c1 < SIZE; ++c1) {
            int v1 = grid_[r][c1].value;
            if (v1 == 0) continue;
            for (int c2 = c1 + 1; c2 < SIZE; ++c2) {
                if (grid_[r][c2].value == v1) {
                    conflicts.push_back({r, c1});
                    conflicts.push_back({r, c2});
                }
            }
        }
    }

    // Check cols
    for (int c = 0; c < SIZE; ++c) {
        for (int r1 = 0; r1 < SIZE; ++r1) {
            int v1 = grid_[r1][c].value;
            if (v1 == 0) continue;
            for (int r2 = r1 + 1; r2 < SIZE; ++r2) {
                if (grid_[r2][c].value == v1) {
                    conflicts.push_back({r1, c});
                    conflicts.push_back({r2, c});
                }
            }
        }
    }

    // Check boxes
    for (int b = 0; b < SIZE; ++b) {
        int start_r = (b / 3) * 3;
        int start_c = (b % 3) * 3;
        std::vector<std::pair<int, CellPos>> cells;
        for (int r = start_r; r < start_r + 3; ++r) {
            for (int c = start_c; c < start_c + 3; ++c) {
                int val = grid_[r][c].value;
                if (val != 0) {
                    cells.push_back({val, {r, c}});
                }
            }
        }
        for (size_t i = 0; i < cells.size(); ++i) {
            for (size_t j = i + 1; j < cells.size(); ++j) {
                if (cells[i].first == cells[j].first) {
                    conflicts.push_back(cells[i].second);
                    conflicts.push_back(cells[j].second);
                }
            }
        }
    }

    // Remove duplicates
    std::sort(conflicts.begin(), conflicts.end(), [](const CellPos& a, const CellPos& b) {
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });
    conflicts.erase(std::unique(conflicts.begin(), conflicts.end()), conflicts.end());

    return conflicts;
}

std::string Board::to_string() const {
    std::string s;
    s.reserve(TOTAL_CELLS);
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            int val = grid_[r][c].value;
            s.push_back(val == 0 ? '.' : static_cast<char>('0' + val));
        }
    }
    return s;
}

bool Board::load_from_string(const std::string& str) {
    if (str.length() < TOTAL_CELLS) return false;
    reset();
    for (int i = 0; i < TOTAL_CELLS; ++i) {
        int r = i / SIZE;
        int c = i % SIZE;
        char ch = str[i];
        if (ch >= '1' && ch <= '9') {
            int val = ch - '0';
            set_value(r, c, val);
            set_given(r, c, true);
        } else {
            set_value(r, c, 0);
            set_given(r, c, false);
        }
    }
    return true;
}

void Board::copy_from(const Board& other) {
    grid_ = other.grid_;
    row_masks_ = other.row_masks_;
    col_masks_ = other.col_masks_;
    box_masks_ = other.box_masks_;
}

} // namespace sudoku

