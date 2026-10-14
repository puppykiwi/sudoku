#pragma once

namespace sudoku {

class InputHandler {
public:
    static InputHandler& instance();

    // Process single frame / tick of input. Returns false if user requested exit.
    bool process_input();

private:
    InputHandler();

    void handle_mouse();
    void handle_main_menu(int ch);
    void handle_difficulty_select(int ch);
    void handle_playing(int ch);
    void handle_paused(int ch);
    void handle_victory(int ch);
    void handle_game_over_strikes(int ch);
    void handle_leaderboard(int ch);
    void handle_theme_select(int ch);
    void handle_help(int ch);

    bool should_exit_{false};
};

} // namespace sudoku

