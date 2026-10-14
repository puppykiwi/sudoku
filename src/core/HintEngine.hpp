#pragma once

#include "Board.hpp"
#include "Types.hpp"
#include <string>

namespace sudoku {

struct HintResult {
    bool found{false};
    CellPos pos{0, 0};
    int digit{0};
    std::string technique;
    std::string explanation;
};

class HintEngine {
public:
    // Finds the next logical hint for the player
    static HintResult get_hint(const Board& board);

private:
    static bool find_naked_single(const Board& board, HintResult& hint);
    static bool find_hidden_single(const Board& board, HintResult& hint);
    static bool find_solution_hint(const Board& board, HintResult& hint);
};

} // namespace sudoku

