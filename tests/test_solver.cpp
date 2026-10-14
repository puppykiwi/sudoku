#include "../src/core/Board.hpp"
#include "../src/core/Solver.hpp"
#include "../src/core/Generator.hpp"
#include "../src/core/HintEngine.hpp"
#include <iostream>
#include <cassert>
#include <chrono>

using namespace sudoku;

void test_board_basics() {
    std::cout << "[Test] Board basics and constraints... ";
    Board board;
    assert(board.count_filled() == 0);

    board.set_value(0, 0, 5);
    assert(board.get_value(0, 0) == 5);
    assert(!board.is_valid_placement(0, 1, 5)); // Same row
    assert(!board.is_valid_placement(1, 0, 5)); // Same col
    assert(!board.is_valid_placement(1, 1, 5)); // Same 3x3 box
    assert(board.is_valid_placement(3, 3, 5));  // Different row, col, box

    board.clear_value(0, 0);
    assert(board.get_value(0, 0) == 0);
    assert(board.is_valid_placement(0, 1, 5));

    std::cout << "PASSED\n";
}

void test_solver() {
    std::cout << "[Test] Solver with known puzzle... ";
    // Classic easy puzzle:
    // 5 3 . . 7 . . . .
    // 6 . . 1 9 5 . . .
    // . 9 8 . . . . 6 .
    // 8 . . . 6 . . . 3
    // 4 . . 8 . 3 . . 1
    // 7 . . . 2 . . . 6
    // . 6 . . . . 2 8 .
    // . . . 4 1 9 . . 5
    // . . . . 8 . . 7 9
    std::string puzzle = 
        "53..7...." 
        "6..195..." 
        ".98....6." 
        "8...6...3" 
        "4..8.3..1" 
        "7...2...6" 
        ".6....28." 
        "...419..5" 
        "....8..79";

    Board board;
    board.load_from_string(puzzle);
    assert(board.count_clues() == 30);

    // Test unique solution count
    int sol_count = Solver::count_solutions(board, 2);
    assert(sol_count == 1);

    // Test solving
    bool solved = Solver::solve(board);
    assert(solved);
    assert(board.is_solved());
    assert(board.is_full());
    assert(board.get_value(0, 2) == 4);

    std::cout << "PASSED\n";
}

void test_generator() {
    std::cout << "[Test] Generator for multiple difficulties... \n";

    Difficulty diffs[] = {Difficulty::EASY, Difficulty::MEDIUM, Difficulty::HARD, Difficulty::EXPERT};
    for (auto diff : diffs) {
        auto t1 = std::chrono::high_resolution_clock::now();
        Board board = Generator::generate(diff, 12345);
        auto t2 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

        int clues = board.count_clues();
        int solutions = Solver::count_solutions(board, 2);

        std::cout << "  - " << difficulty_to_string(diff)
                  << ": Clues = " << clues
                  << ", Uniqueness = " << (solutions == 1 ? "EXACTLY 1" : "FAIL")
                  << ", Time = " << ms << " ms\n";

        assert(solutions == 1);
        assert(clues > 17 && clues < 60);
    }
    std::cout << "[Test] Generator: ALL PASSED\n";
}

void test_hints() {
    std::cout << "[Test] Hint engine... ";
    std::string puzzle = 
        "53..7...." 
        "6..195..." 
        ".98....6." 
        "8...6...3" 
        "4..8.3..1" 
        "7...2...6" 
        ".6....28." 
        "...419..5" 
        "....8..79";

    Board board;
    board.load_from_string(puzzle);
    auto hint = HintEngine::get_hint(board);
    assert(hint.found);
    assert(hint.digit >= 1 && hint.digit <= 9);
    std::cout << "Hint found: " << hint.technique << " -> " << hint.explanation << " PASSED\n";
}

int main() {
    std::cout << "=== Running Sudoku Core Tests ===\n";
    test_board_basics();
    test_solver();
    test_hints();
    test_generator();
    std::cout << "=== All Core Tests Passed Successfully! ===\n";
    return 0;
}

