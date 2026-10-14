#include "../src/core/Board.hpp"
#include "../src/core/Solver.hpp"
#include "../src/core/Generator.hpp"
#include "../src/core/HintEngine.hpp"
#include "../src/game/GameEngine.hpp"
#include "../src/game/ScoreManager.hpp"
#include "../src/game/Theme.hpp"
#include <iostream>
#include <cassert>

using namespace sudoku;

void test_score_calculation() {
    std::cout << "[Test] Dynamic score calculation... ";
    // Easy: base 1500. Time: 60s (-120). Mistakes: 0. Hints: 0. Bonus: +1000.
    int score1 = ScoreManager::calculate_score(Difficulty::EASY, 60, 0, 0);
    assert(score1 == 1500 - 120 + 1000); // 2380

    // Hard: base 6000. Time: 300s (-600). Mistakes: 2 (-600). Hints: 1 (-250). Bonus: 0.
    int score2 = ScoreManager::calculate_score(Difficulty::HARD, 300, 2, 1);
    assert(score2 == 6000 - 600 - 600 - 250); // 4550

    // Floor check
    int score_floor = ScoreManager::calculate_score(Difficulty::EASY, 10000, 3, 10);
    assert(score_floor == 100);

    std::cout << "PASSED\n";
}

void test_high_scores_persistence() {
    std::cout << "[Test] ScoreManager JSON load/save... ";
    auto& sm = ScoreManager::instance();

    ScoreEntry entry1{"Alice", Difficulty::HARD, 5200, 180, 1, 0, "2026-10-14 06:00"};
    ScoreEntry entry2{"Bob", Difficulty::EXPERT, 8900, 240, 0, 0, "2026-10-14 06:15"};
    ScoreEntry entry3{"Charlie", Difficulty::EASY, 2100, 90, 0, 1, "2026-10-14 06:30"};

    sm.add_score(entry1);
    sm.add_score(entry2);
    sm.add_score(entry3);
    assert(sm.save());

    assert(sm.load());
    auto top = sm.get_all_top_scores(10);
    assert(!top.empty());
    // Bob has highest score (8900)
    assert(top[0].score >= 8900);

    auto hard_top = sm.get_top_scores(Difficulty::HARD);
    assert(!hard_top.empty());

    std::cout << "PASSED\n";
}

void test_strikes_and_gameplay() {
    std::cout << "[Test] 3-Strikes gameplay rules... ";
    auto& engine = GameEngine::instance();
    engine.start_new_game(Difficulty::EASY);
    assert(engine.state() == GameState::PLAYING);
    assert(engine.strikes() == 0);

    // Find an empty cell
    int target_r = -1, target_c = -1;
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (!engine.board().is_given(r, c)) {
                target_r = r;
                target_c = c;
                break;
            }
        }
        if (target_r != -1) break;
    }

    assert(target_r != -1);
    engine.set_cursor(target_r, target_c);
    int correct_val = engine.board().get_solution(target_r, target_c);
    int wrong_val = (correct_val % 9) + 1;

    // Strike 1
    engine.enter_digit(wrong_val);
    assert(engine.strikes() == 1);
    assert(engine.state() == GameState::PLAYING);
    assert(engine.board().has_error(target_r, target_c));

    // Strike 2
    engine.enter_digit(wrong_val);
    assert(engine.strikes() == 2);
    assert(engine.state() == GameState::PLAYING);

    // Strike 3 -> Game Over
    engine.enter_digit(wrong_val);
    assert(engine.strikes() == 3);
    assert(engine.state() == GameState::GAME_OVER_STRIKES);

    // Restart game
    engine.restart_current_game();
    assert(engine.state() == GameState::PLAYING);
    assert(engine.strikes() == 0);

    // Now test pencil notes in note mode
    engine.toggle_note_mode();
    assert(engine.note_mode());
    engine.enter_digit(1);
    engine.enter_digit(4);
    assert(engine.board().get_cell(target_r, target_c).has_note(1));
    assert(engine.board().get_cell(target_r, target_c).has_note(4));
    assert(!engine.board().get_cell(target_r, target_c).has_note(7));
    assert(engine.strikes() == 0); // No strikes in note mode!

    // Test undo
    engine.undo();
    assert(!engine.board().get_cell(target_r, target_c).has_note(4));
    assert(engine.board().get_cell(target_r, target_c).has_note(1));

    // Test redo
    engine.redo();
    assert(engine.board().get_cell(target_r, target_c).has_note(4));

    // Switch back to normal mode
    engine.toggle_note_mode();
    assert(!engine.note_mode());

    // Enter correct digit
    engine.enter_digit(correct_val);
    assert(engine.board().get_value(target_r, target_c) == correct_val);
    assert(engine.board().get_cell(target_r, target_c).notes == 0); // Notes cleared
    assert(engine.strikes() == 0);

    // Test hint
    engine.request_hint();
    assert(engine.has_active_hint());
    assert(engine.hints_used() == 1);

    std::cout << "PASSED\n";
}

void test_themes() {
    std::cout << "[Test] Themes catalog... ";
    auto& tm = ThemeManager::instance();
    assert(tm.all_themes().size() == 6);

    for (int i = 0; i < 6; ++i) {
        tm.set_theme(static_cast<ThemeId>(i));
        assert(tm.current_theme_id() == static_cast<ThemeId>(i));
        assert(!tm.current_theme().name.empty());
    }
    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== Running Gameplay & State Machine Tests ===\n";
    test_score_calculation();
    test_high_scores_persistence();
    test_strikes_and_gameplay();
    test_themes();
    std::cout << "=== All Gameplay Tests Passed! ===\n";
    return 0;
}

