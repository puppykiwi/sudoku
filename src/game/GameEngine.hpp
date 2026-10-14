#pragma once

#include "../core/Board.hpp"
#include "../core/Types.hpp"
#include "../core/HintEngine.hpp"
#include "ScoreManager.hpp"
#include <chrono>
#include <vector>
#include <string>

namespace sudoku {

class GameEngine {
public:
    static GameEngine& instance();

    void init();
    void update(); // Called on every tick/loop

    // Game Flow
    void start_new_game(Difficulty diff);
    void pause_game();
    void resume_game();
    void restart_current_game();
    void return_to_main_menu();

    // Input Actions
    void move_cursor(int d_row, int d_col);
    void set_cursor(int row, int col);
    void enter_digit(int digit);
    void clear_current_cell();
    void toggle_note_mode();
    void undo();
    void redo();
    void request_hint();
    void clear_active_hint();

    // Accessors
    GameState state() const { return state_; }
    void set_state(GameState s) { state_ = s; }

    const Board& board() const { return board_; }
    Board& board_mut() { return board_; }

    CellPos cursor() const { return cursor_; }
    bool note_mode() const { return note_mode_; }
    int strikes() const { return strikes_; }
    int max_strikes() const { return MAX_STRIKES; }
    int hints_used() const { return hints_used_; }
    Difficulty difficulty() const { return difficulty_; }
    int elapsed_seconds() const;
    int current_score() const;

    const HintResult& active_hint() const { return active_hint_; }
    bool has_active_hint() const { return active_hint_.found; }

    const std::string& status_message() const { return status_message_; }
    void set_status_message(const std::string& msg, int duration_ticks = 40);

    const std::string& player_name() const { return player_name_; }
    void set_player_name(const std::string& name) { player_name_ = name; }
    void append_player_name(char c);
    void pop_player_name();
    void submit_victory_score();

    // Menu Navigation
    int menu_selection() const { return menu_selection_; }
    void menu_up(int count);
    void menu_down(int count);
    void set_menu_selection(int sel) { menu_selection_ = sel; }

private:
    GameEngine();

    void record_move(const MoveRecord& move);
    void check_victory();

    static constexpr int MAX_STRIKES = 3;

    GameState state_{GameState::MAIN_MENU};
    Board board_;
    CellPos cursor_{0, 0};
    bool note_mode_{false};
    int strikes_{0};
    int hints_used_{0};
    Difficulty difficulty_{Difficulty::MEDIUM};

    // Timer tracking
    std::chrono::steady_clock::time_point timer_start_;
    int accumulated_seconds_{0};
    bool timer_running_{false};

    // History
    std::vector<MoveRecord> undo_stack_;
    std::vector<MoveRecord> redo_stack_;

    // Hint and notifications
    HintResult active_hint_;
    std::string status_message_;
    int status_ticks_{0};

    // Menu state
    int menu_selection_{0};

    // Player and score
    std::string player_name_{"Player"};
    int final_score_{0};
    bool score_submitted_{false};
};

} // namespace sudoku

