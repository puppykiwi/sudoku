#pragma once

#include "Board.hpp"
#include "Types.hpp"
#include <random>

namespace sudoku {

class Generator {
public:
    // Generate a valid, uniquely solvable puzzle for the given difficulty
    static Board generate(Difficulty difficulty, unsigned int seed = 0);

private:
    static void generate_full_board(Board& board, std::mt19937& rng);
    static void dig_clues(Board& board, Difficulty difficulty, std::mt19937& rng);
};

} // namespace sudoku

