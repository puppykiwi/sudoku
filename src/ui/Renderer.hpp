#pragma once

#include "../game/GameEngine.hpp"
#include "../game/Theme.hpp"
#include <string>

namespace sudoku {

class Renderer {
public:
    static Renderer& instance();

    void init();
    void shutdown();
    void render();

    // Screen geometry
    int term_width() const { return term_width_; }
    int term_height() const { return term_height_; }

    // Hit testing for mouse clicks
    bool board_cell_from_screen(int screen_x, int screen_y, int& out_row, int& out_col) const;
    bool keypad_digit_from_screen(int screen_x, int screen_y, int& out_digit) const;
    bool menu_item_from_screen(int screen_x, int screen_y, int item_count, int& out_item) const;

private:
    Renderer();
    ~Renderer();

    void update_dimensions();

    // Scene renderers
    void render_main_menu();
    void render_difficulty_select();
    void render_gameplay();
    void render_paused();
    void render_victory();
    void render_game_over_strikes();
    void render_leaderboard();
    void render_theme_select();
    void render_help();

    // Gameplay sub-renderers
    void draw_board(int start_y, int start_x);
    void draw_side_panel(int start_y, int start_x);
    void draw_cell(int r, int c, int start_y, int start_x);
    void draw_dialog_box(int y, int x, int width, int height, const std::string& title);

    int term_width_{80};
    int term_height_{24};

    // Stored layout positions for mouse hit testing
    int board_top_y_{2};
    int board_left_x_{3};
    int keypad_top_y_{0};
    int keypad_left_x_{0};
    int menu_top_y_{0};
};

} // namespace sudoku

