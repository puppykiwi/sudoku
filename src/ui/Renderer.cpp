#define _XOPEN_SOURCE_EXTENDED 1
#include "Renderer.hpp"
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace sudoku {

Renderer& Renderer::instance() {
    static Renderer renderer;
    return renderer;
}

Renderer::Renderer() {}

Renderer::~Renderer() {
    shutdown();
}

void Renderer::init() {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // Non-blocking input for responsive game loop
    curs_set(0);           // Hide hardware terminal cursor
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    ThemeManager::instance().init();
    update_dimensions();
}

void Renderer::shutdown() {
    curs_set(1);
    endwin();
}

void Renderer::update_dimensions() {
    getmaxyx(stdscr, term_height_, term_width_);
}

void Renderer::draw_dialog_box(int y, int x, int width, int height, const std::string& title) {
    attron(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));

    // Corners and horizontals
    mvaddstr(y, x, "┏");
    for (int i = 1; i < width - 1; ++i) addstr("━");
    addstr("┓");

    for (int row = 1; row < height - 1; ++row) {
        mvaddstr(y + row, x, "┃");
        for (int i = 1; i < width - 1; ++i) addch(' ');
        mvaddstr(y + row, x + width - 1, "┃");
    }

    mvaddstr(y + height - 1, x, "┗");
    for (int i = 1; i < width - 1; ++i) addstr("━");
    addstr("┛");

    if (!title.empty()) {
        int title_pos = x + (width - static_cast<int>(title.length()) - 4) / 2;
        if (title_pos > x + 1) {
            attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
            mvprintw(y, title_pos, "┤ %s ├", title.c_str());
            attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
        }
    }

    attroff(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
}

void Renderer::render() {
    update_dimensions();
    werase(stdscr);

    // Fill background
    bkgd(COLOR_PAIR(PAIR_DEFAULT));

    GameState state = GameEngine::instance().state();

    switch (state) {
        case GameState::MAIN_MENU:
            render_main_menu();
            break;
        case GameState::DIFFICULTY_SELECT:
            render_difficulty_select();
            break;
        case GameState::PLAYING:
            render_gameplay();
            break;
        case GameState::PAUSED:
            render_gameplay();
            render_paused();
            break;
        case GameState::GAME_OVER_VICTORY:
            render_gameplay();
            render_victory();
            break;
        case GameState::GAME_OVER_STRIKES:
            render_gameplay();
            render_game_over_strikes();
            break;
        case GameState::LEADERBOARD:
            render_leaderboard();
            break;
        case GameState::THEME_SELECT:
            render_theme_select();
            break;
        case GameState::HELP:
            render_help();
            break;
    }

    wrefresh(stdscr);
}

void Renderer::render_main_menu() {
    int center_x = term_width_ / 2;
    int start_y = std::max(1, (term_height_ - 22) / 2);

    // Title ASCII art
    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    const char* logo[] = {
        "  ███████╗██╗   ██╗██████╗  ██████╗ ██╗  ██╗██╗   ██╗",
        "  ██╔════╝██║   ██║██╔══██╗██╔═══██╗██║ ██╔╝██║   ██║",
        "  ███████╗██║   ██║██║  ██║██║   ██║█████╔╝ ██║   ██║",
        "  ╚════██║██║   ██║██║  ██║██║   ██║██╔═██╗ ██║   ██║",
        "  ███████║╚██████╔╝██████╔╝╚██████╔╝██║  ██╗╚██████╔╝",
        "  ╚══════╝ ╚═════╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ "
    };

    for (int i = 0; i < 6; ++i) {
        int x = center_x - 27;
        mvaddstr(start_y + i, std::max(0, x), logo[i]);
    }
    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string subtitle = "MODERN C++ EDITION  •  STANDARD RULES  •  3-STRIKES MODE";
    mvaddstr(start_y + 7, center_x - static_cast<int>(subtitle.length()) / 2, subtitle.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    // Menu options
    const std::vector<std::string> options = {
        "New Game",
        "High Scores & Leaderboards",
        "Color Themes",
        "How To Play / Rules",
        "Quit Game"
    };

    menu_top_y_ = start_y + 10;
    int sel = GameEngine::instance().menu_selection();

    for (size_t i = 0; i < options.size(); ++i) {
        int y = menu_top_y_ + static_cast<int>(i) * 2;
        int box_width = 36;
        int box_x = center_x - (box_width / 2);

        if (static_cast<int>(i) == sel) {
            attron(COLOR_PAIR(PAIR_MENU_SELECTED) | A_BOLD);
            mvprintw(y, box_x, "  ►  %-30s", options[i].c_str());
            attroff(COLOR_PAIR(PAIR_MENU_SELECTED) | A_BOLD);
        } else {
            attron(COLOR_PAIR(PAIR_MENU_NORMAL));
            mvprintw(y, box_x, "     %-30s", options[i].c_str());
            attroff(COLOR_PAIR(PAIR_MENU_NORMAL));
        }
    }

    // Footer info
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string footer = "Use [↑ / ↓ / W / S] or Mouse to Select  •  [Enter] Confirm  •  [Q] Quit";
    mvaddstr(term_height_ - 2, std::max(0, center_x - static_cast<int>(footer.length()) / 2), footer.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
}

void Renderer::render_difficulty_select() {
    int center_x = term_width_ / 2;
    int start_y = std::max(2, (term_height_ - 20) / 2);

    draw_dialog_box(start_y, center_x - 26, 52, 18, "SELECT DIFFICULTY");

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string sub = "Choose your challenge tier:";
    mvaddstr(start_y + 2, center_x - static_cast<int>(sub.length()) / 2, sub.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    struct DiffDesc {
        std::string name;
        std::string desc;
        int badge_pair;
    };

    DiffDesc diffs[] = {
        {"Easy", "40 clues  •  Singles only, quick & relaxing", PAIR_BADGE_EASY},
        {"Medium", "34 clues  •  Balanced logic, naked & hidden singles", PAIR_BADGE_MEDIUM},
        {"Hard", "29 clues  •  Pairs, triplets & deeper deductions", PAIR_BADGE_HARD},
        {"Expert", "24 clues  •  Master level, intricate branching", PAIR_BADGE_EXPERT}
    };

    menu_top_y_ = start_y + 4;
    int sel = GameEngine::instance().menu_selection();

    for (int i = 0; i < 4; ++i) {
        int y = menu_top_y_ + i * 3;
        int x = center_x - 22;

        if (i == sel) {
            attron(COLOR_PAIR(PAIR_MENU_SELECTED) | A_BOLD);
            mvprintw(y, x, " ► %-8s ", diffs[i].name.c_str());
            attroff(COLOR_PAIR(PAIR_MENU_SELECTED) | A_BOLD);

            attron(COLOR_PAIR(PAIR_DEFAULT) | A_BOLD);
            printw(" %s", diffs[i].desc.c_str());
            attroff(COLOR_PAIR(PAIR_DEFAULT) | A_BOLD);
        } else {
            attron(COLOR_PAIR(diffs[i].badge_pair) | A_BOLD);
            mvprintw(y, x, "   %-8s ", diffs[i].name.c_str());
            attroff(COLOR_PAIR(diffs[i].badge_pair) | A_BOLD);

            attron(COLOR_PAIR(PAIR_STATUS_LABEL));
            printw(" %s", diffs[i].desc.c_str());
            attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
        }
    }

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string footer = "[Enter] Start Game   •   [Esc / Backspace] Return to Menu";
    mvaddstr(start_y + 16, center_x - static_cast<int>(footer.length()) / 2, footer.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
}

void Renderer::render_gameplay() {
    int total_width = 37 + 4 + 38; // 79 columns
    int start_x = std::max(1, (term_width_ - total_width) / 2);
    int start_y = std::max(1, (term_height_ - 21) / 2);

    board_top_y_ = start_y;
    board_left_x_ = start_x;

    draw_board(start_y, start_x);
    draw_side_panel(start_y, start_x + 40);

    // Bottom status bar
    const std::string& msg = GameEngine::instance().status_message();
    if (!msg.empty()) {
        int msg_x = std::max(1, (term_width_ - static_cast<int>(msg.length())) / 2);
        attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
        mvprintw(start_y + 20, msg_x, " %s ", msg.c_str());
        attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    }
}

void Renderer::draw_board(int start_y, int start_x) {
    // Box-drawing characters
    // Top border
    attron(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
    mvaddstr(start_y, start_x, "╔═══════════╦═══════════╦═══════════╗");
    attroff(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));

    for (int r = 0; r < 9; ++r) {
        int cell_y = start_y + 1 + r * 2;

        // Render row cells
        for (int c = 0; c < 9; ++c) {
            int border_x = start_x + c * 4;

            // Draw left border of cell
            if (c % 3 == 0) {
                attron(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
                mvaddstr(cell_y, border_x, "║");
                attroff(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
            } else {
                attron(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
                mvaddstr(cell_y, border_x, "│");
                attroff(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
            }

            // Draw cell contents (3 characters wide)
            draw_cell(r, c, cell_y, border_x + 1);
        }
        // Rightmost outer border
        attron(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
        mvaddstr(cell_y, start_x + 36, "║");
        attroff(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));

        // Horizontal dividers
        if (r < 8) {
            int div_y = cell_y + 1;
            if ((r + 1) % 3 == 0) {
                attron(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
                mvaddstr(div_y, start_x, "╠═══════════╬═══════════╬═══════════╣");
                attroff(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
            } else {
                attron(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
                mvaddstr(div_y, start_x, "║───┼───┼───║───┼───┼───║───┼───┼───║");
                attroff(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
            }
        }
    }

    // Bottom border
    attron(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
    mvaddstr(start_y + 19, start_x, "╚═══════════╩═══════════╩═══════════╝");
    attroff(COLOR_PAIR(PAIR_BOARD_BORDER_OUTER));
}

void Renderer::draw_cell(int r, int c, int cell_y, int cell_x) {
    const Board& board = GameEngine::instance().board();
    const Cell& cell = board.get_cell(r, c);
    CellPos cursor = GameEngine::instance().cursor();
    int cursor_val = board.get_value(cursor.row, cursor.col);
    const auto& hint = GameEngine::instance().active_hint();

    bool is_cursor = (r == cursor.row && c == cursor.col);
    bool is_crosshair = (r == cursor.row || c == cursor.col || Board::get_box(r, c) == Board::get_box(cursor.row, cursor.col));
    bool is_same_digit = (cursor_val > 0 && cell.value == cursor_val);
    bool is_hint_target = (hint.found && hint.pos.row == r && hint.pos.col == c);

    int pair = PAIR_DEFAULT;
    int attr = A_NORMAL;

    if (cell.has_error) {
        pair = PAIR_ERROR;
        attr = A_BOLD;
    } else if (is_cursor) {
        pair = PAIR_CURSOR;
        attr = A_BOLD;
    } else if (is_hint_target) {
        pair = PAIR_HINT;
        attr = A_BOLD;
    } else if (is_same_digit) {
        pair = PAIR_HIGHLIGHT_SAME;
        attr = A_BOLD;
    } else if (is_crosshair) {
        pair = PAIR_CURSOR_CROSSHAIR;
    }

    attron(COLOR_PAIR(pair) | attr);

    if (cell.value > 0) {
        // Filled digit
        if (!is_cursor && !cell.has_error && !is_hint_target && !is_same_digit) {
            if (cell.is_given) {
                attron(COLOR_PAIR(PAIR_GIVEN_DIGIT) | A_BOLD);
            } else {
                attron(COLOR_PAIR(PAIR_USER_DIGIT) | A_BOLD);
            }
        }
        mvprintw(cell_y, cell_x, " %d ", cell.value);
    } else if (cell.notes != 0) {
        // Pencil marks
        if (!is_cursor && !is_hint_target) {
            attron(COLOR_PAIR(PAIR_NOTE_DIGIT));
        }
        int count = __builtin_popcount(cell.notes);
        if (count == 1) {
            int d = __builtin_ctz(cell.notes);
            mvprintw(cell_y, cell_x, " %d ", d);
        } else if (count == 2) {
            int d1 = 0, d2 = 0;
            for (int d = 1; d <= 9; ++d) {
                if ((cell.notes & (1 << d)) != 0) {
                    if (d1 == 0) d1 = d; else d2 = d;
                }
            }
            mvprintw(cell_y, cell_x, "%d %d", d1, d2);
        } else {
            // Display first and last candidate or count
            int d_first = __builtin_ctz(cell.notes);
            int d_last = 31 - __builtin_clz(cell.notes);
            mvprintw(cell_y, cell_x, "%d·%d", d_first, d_last);
        }
    } else {
        // Empty cell
        mvaddstr(cell_y, cell_x, "   ");
    }

    attroff(COLOR_PAIR(pair) | attr);
}

void Renderer::draw_side_panel(int start_y, int start_x) {
    auto& engine = GameEngine::instance();
    const Board& board = engine.board();
    auto& theme_mgr = ThemeManager::instance();

    draw_dialog_box(start_y, start_x, 38, 20, "GAME STATUS");

    // Header: Theme name
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(start_y + 2, start_x + 3, "Theme: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    printw("%-16s", theme_mgr.current_theme().name.c_str());
    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);

    // Difficulty badge
    int diff_pair = PAIR_BADGE_MEDIUM;
    if (engine.difficulty() == Difficulty::EASY) diff_pair = PAIR_BADGE_EASY;
    else if (engine.difficulty() == Difficulty::HARD) diff_pair = PAIR_BADGE_HARD;
    else if (engine.difficulty() == Difficulty::EXPERT) diff_pair = PAIR_BADGE_EXPERT;

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(start_y + 3, start_x + 3, "Difficulty: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(diff_pair) | A_BOLD);
    printw("[%s]", difficulty_to_string(engine.difficulty()).c_str());
    attroff(COLOR_PAIR(diff_pair) | A_BOLD);

    // Timer & Score
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(start_y + 4, start_x + 3, "Time: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);
    printw("%s", ScoreManager::format_time(engine.elapsed_seconds()).c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    printw("    Score: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);
    printw("%d", engine.current_score());
    attroff(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);

    // Strikes indicator (3 Strikes Mode)
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(start_y + 5, start_x + 3, "Strikes: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    for (int i = 0; i < engine.max_strikes(); ++i) {
        if (i < engine.strikes()) {
            attron(COLOR_PAIR(PAIR_STRIKE_ACTIVE) | A_BOLD);
            addstr(" ✖");
            attroff(COLOR_PAIR(PAIR_STRIKE_ACTIVE) | A_BOLD);
        } else {
            attron(COLOR_PAIR(PAIR_STRIKE_INACTIVE));
            addstr(" ○");
            attroff(COLOR_PAIR(PAIR_STRIKE_INACTIVE));
        }
    }
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    printw("  (%d / %d)", engine.strikes(), engine.max_strikes());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    // Entry Mode
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(start_y + 6, start_x + 3, "Mode: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    if (engine.note_mode()) {
        attron(COLOR_PAIR(PAIR_HINT) | A_BOLD);
        printw("[ ✎ PENCIL NOTES ] (Key: N)");
        attroff(COLOR_PAIR(PAIR_HINT) | A_BOLD);
    } else {
        attron(COLOR_PAIR(PAIR_USER_DIGIT) | A_BOLD);
        printw("[ 1-9 NORMAL ENTRY ] (Key: N)");
        attroff(COLOR_PAIR(PAIR_USER_DIGIT) | A_BOLD);
    }

    // Divider line
    attron(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
    mvaddstr(start_y + 7, start_x + 2, "──────────────────────────────────");
    attroff(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));

    // Selected Cell info and Candidate Notes Mini-Grid
    CellPos cursor = engine.cursor();
    const Cell& cur_cell = board.get_cell(cursor.row, cursor.col);

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(start_y + 8, start_x + 3, "Cell: R%d, C%d   Candidates: ", cursor.row + 1, cursor.col + 1);
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    // Mini 1-9 candidate row for selected cell
    for (int d = 1; d <= 9; ++d) {
        if (cur_cell.value == d) {
            attron(COLOR_PAIR(PAIR_GIVEN_DIGIT) | A_BOLD);
            printw("%d", d);
            attroff(COLOR_PAIR(PAIR_GIVEN_DIGIT) | A_BOLD);
        } else if (cur_cell.has_note(d)) {
            attron(COLOR_PAIR(PAIR_NOTE_DIGIT) | A_BOLD);
            printw("%d", d);
            attroff(COLOR_PAIR(PAIR_NOTE_DIGIT) | A_BOLD);
        } else {
            attron(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
            printw("·");
            attroff(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
        }
    }

    // Number Completion Keypad
    keypad_top_y_ = start_y + 10;
    keypad_left_x_ = start_x + 3;

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(keypad_top_y_, keypad_left_x_, "Digits Remaining (Click or 1-9):");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    for (int d = 1; d <= 9; ++d) {
        int row_offset = (d - 1) / 5;
        int col_offset = ((d - 1) % 5) * 6;
        int y = keypad_top_y_ + 1 + row_offset;
        int x = keypad_left_x_ + col_offset;

        int count = board.count_digit_occurrences(d);
        int remaining = 9 - count;

        if (remaining == 0) {
            attron(COLOR_PAIR(PAIR_KEYPAD_COMPLETED));
            mvprintw(y, x, "[%d:✓]", d);
            attroff(COLOR_PAIR(PAIR_KEYPAD_COMPLETED));
        } else {
            attron(COLOR_PAIR(PAIR_KEYPAD_DIGIT) | A_BOLD);
            mvprintw(y, x, "[%d:%d]", d, remaining);
            attroff(COLOR_PAIR(PAIR_KEYPAD_DIGIT) | A_BOLD);
        }
    }

    // Divider line
    attron(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
    mvaddstr(start_y + 14, start_x + 2, "──────────────────────────────────");
    attroff(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));

    // Hotkey Controls Guide
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(start_y + 15, start_x + 3, "[Arrows/WASD] Move   [1-9] Digit/Note");
    mvprintw(start_y + 16, start_x + 3, "[N] Toggle Notes     [Bksp/Del] Clear");
    mvprintw(start_y + 17, start_x + 3, "[H] Logical Hint     [U] Undo [R] Redo");
    mvprintw(start_y + 18, start_x + 3, "[T] Switch Theme     [M / Esc] Menu");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
}

void Renderer::render_paused() {
    int center_x = term_width_ / 2;
    int center_y = term_height_ / 2;

    draw_dialog_box(center_y - 4, center_x - 18, 36, 9, "GAME PAUSED");

    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    std::string p = "PAUSED";
    mvaddstr(center_y - 2, center_x - static_cast<int>(p.length()) / 2, p.c_str());
    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_DEFAULT));
    mvprintw(center_y, center_x - 14, "[P / Esc] Resume Game");
    mvprintw(center_y + 1, center_x - 14, "[R] Restart Current Puzzle");
    mvprintw(center_y + 2, center_x - 14, "[M] Return to Main Menu");
    attroff(COLOR_PAIR(PAIR_DEFAULT));
}

void Renderer::render_victory() {
    int center_x = term_width_ / 2;
    int center_y = term_height_ / 2;
    auto& engine = GameEngine::instance();

    draw_dialog_box(center_y - 8, center_x - 25, 50, 17, "★ PUZZLE COMPLETED! ★");

    attron(COLOR_PAIR(PAIR_VICTORY_BANNER) | A_BOLD);
    std::string banner = "🎉  VICTORY! PUZZLE SOLVED!  🎉";
    mvaddstr(center_y - 6, center_x - static_cast<int>(banner.length()) / 2, banner.c_str());
    attroff(COLOR_PAIR(PAIR_VICTORY_BANNER) | A_BOLD);

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(center_y - 4, center_x - 20, "Difficulty: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);
    printw("%s", difficulty_to_string(engine.difficulty()).c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(center_y - 3, center_x - 20, "Time Taken: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);
    printw("%s", ScoreManager::format_time(engine.elapsed_seconds()).c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(center_y - 2, center_x - 20, "Mistakes:   ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);
    printw("%d (Strike limit: 3)", engine.strikes());
    attroff(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(center_y - 1, center_x - 20, "Hints Used: ");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);
    printw("%d", engine.hints_used());
    attroff(COLOR_PAIR(PAIR_STATUS_VALUE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_VICTORY_BANNER) | A_BOLD);
    mvprintw(center_y + 1, center_x - 20, "FINAL SCORE: %d PTS", engine.current_score());
    attroff(COLOR_PAIR(PAIR_VICTORY_BANNER) | A_BOLD);

    // Player name input box
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    mvprintw(center_y + 3, center_x - 20, "Player Name: [");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
    attron(COLOR_PAIR(PAIR_CURSOR) | A_BOLD);
    printw(" %-12s ", engine.player_name().c_str());
    attroff(COLOR_PAIR(PAIR_CURSOR) | A_BOLD);
    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    printw("]");
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string sub = "[Type letters/digits to edit name]  •  [Enter] Submit";
    mvaddstr(center_y + 6, center_x - static_cast<int>(sub.length()) / 2, sub.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
}

void Renderer::render_game_over_strikes() {
    int center_x = term_width_ / 2;
    int center_y = term_height_ / 2;

    draw_dialog_box(center_y - 5, center_x - 20, 40, 11, "GAME OVER");

    attron(COLOR_PAIR(PAIR_ERROR) | A_BOLD);
    std::string go = "✖ ✖ ✖  THREE STRIKES REACHED  ✖ ✖ ✖";
    mvaddstr(center_y - 3, center_x - static_cast<int>(go.length()) / 2, go.c_str());
    attroff(COLOR_PAIR(PAIR_ERROR) | A_BOLD);

    attron(COLOR_PAIR(PAIR_DEFAULT));
    mvprintw(center_y - 1, center_x - 15, "You reached the 3-mistake limit.");
    mvprintw(center_y + 1, center_x - 15, "[R] Restart Same Puzzle");
    mvprintw(center_y + 2, center_x - 15, "[N] New Game");
    mvprintw(center_y + 3, center_x - 15, "[M] Return to Main Menu");
    attroff(COLOR_PAIR(PAIR_DEFAULT));
}

void Renderer::render_leaderboard() {
    int center_x = term_width_ / 2;
    int start_y = std::max(1, (term_height_ - 22) / 2);

    draw_dialog_box(start_y, center_x - 36, 72, 22, "HALL OF FAME - HIGH SCORES");

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string headers = "RANK   PLAYER           DIFFICULTY   TIME     STRIKES   SCORE   DATE";
    mvaddstr(start_y + 2, center_x - 33, headers.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    attron(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));
    mvaddstr(start_y + 3, center_x - 33, "──────────────────────────────────────────────────────────────────");
    attroff(COLOR_PAIR(PAIR_BOARD_BORDER_INNER));

    auto top = ScoreManager::instance().get_all_top_scores(12);

    if (top.empty()) {
        attron(COLOR_PAIR(PAIR_DEFAULT));
        std::string empty = "No high scores recorded yet! Complete a puzzle to enter the Hall of Fame.";
        mvaddstr(start_y + 7, center_x - static_cast<int>(empty.length()) / 2, empty.c_str());
        attroff(COLOR_PAIR(PAIR_DEFAULT));
    } else {
        for (size_t i = 0; i < top.size(); ++i) {
            const auto& s = top[i];
            int y = start_y + 4 + static_cast<int>(i);

            int pair = PAIR_DEFAULT;
            if (i == 0) pair = PAIR_VICTORY_BANNER;
            else if (i == 1) pair = PAIR_USER_DIGIT;
            else if (i == 2) pair = PAIR_GIVEN_DIGIT;

            attron(COLOR_PAIR(pair) | (i < 3 ? A_BOLD : A_NORMAL));
            mvprintw(y, center_x - 33, "#%-4d %-16s %-12s %-8s %-9d %-7d %s",
                     static_cast<int>(i + 1),
                     s.player_name.substr(0, 15).c_str(),
                     difficulty_to_string(s.difficulty).c_str(),
                     ScoreManager::format_time(s.time_seconds).c_str(),
                     s.mistakes,
                     s.score,
                     s.date_str.substr(0, 10).c_str());
            attroff(COLOR_PAIR(pair) | (i < 3 ? A_BOLD : A_NORMAL));
        }
    }

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string footer = "[Esc / Enter / M] Return to Main Menu";
    mvaddstr(start_y + 20, center_x - static_cast<int>(footer.length()) / 2, footer.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
}

void Renderer::render_theme_select() {
    int center_x = term_width_ / 2;
    int start_y = std::max(2, (term_height_ - 20) / 2);

    draw_dialog_box(start_y, center_x - 30, 60, 19, "MODERN THEME SELECTOR");

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string sub = "Cycle through themes to preview live in the terminal:";
    mvaddstr(start_y + 2, center_x - static_cast<int>(sub.length()) / 2, sub.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));

    const auto& themes = ThemeManager::instance().all_themes();
    menu_top_y_ = start_y + 4;
    int sel = GameEngine::instance().menu_selection();

    for (size_t i = 0; i < themes.size(); ++i) {
        int y = menu_top_y_ + static_cast<int>(i) * 2;
        int x = center_x - 26;

        if (static_cast<int>(i) == sel) {
            attron(COLOR_PAIR(PAIR_MENU_SELECTED) | A_BOLD);
            mvprintw(y, x, " ► %-18s ", themes[i].name.c_str());
            attroff(COLOR_PAIR(PAIR_MENU_SELECTED) | A_BOLD);

            attron(COLOR_PAIR(PAIR_DEFAULT) | A_BOLD);
            printw(" %s", themes[i].description.c_str());
            attroff(COLOR_PAIR(PAIR_DEFAULT) | A_BOLD);
        } else {
            attron(COLOR_PAIR(PAIR_MENU_NORMAL));
            mvprintw(y, x, "   %-18s ", themes[i].name.c_str());
            attroff(COLOR_PAIR(PAIR_MENU_NORMAL));

            attron(COLOR_PAIR(PAIR_STATUS_LABEL));
            printw(" %s", themes[i].description.c_str());
            attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
        }
    }

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string footer = "[↑ / ↓] Select Theme   •   [Enter / Esc] Apply & Back";
    mvaddstr(start_y + 17, center_x - static_cast<int>(footer.length()) / 2, footer.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
}

void Renderer::render_help() {
    int center_x = term_width_ / 2;
    int start_y = std::max(1, (term_height_ - 22) / 2);

    draw_dialog_box(start_y, center_x - 35, 70, 22, "HOW TO PLAY & SUDOKU RULES");

    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    mvaddstr(start_y + 2, center_x - 30, "STANDARD RULES:");
    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_DEFAULT));
    mvaddstr(start_y + 3, center_x - 30, "• Fill the 9x9 grid with numbers 1 through 9.");
    mvaddstr(start_y + 4, center_x - 30, "• Each row, column, and 3x3 block must contain every digit 1-9.");
    mvaddstr(start_y + 5, center_x - 30, "• Every generated puzzle has exactly ONE unique valid solution.");
    attroff(COLOR_PAIR(PAIR_DEFAULT));

    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    mvaddstr(start_y + 7, center_x - 30, "3-STRIKES MODE:");
    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_DEFAULT));
    mvaddstr(start_y + 8, center_x - 30, "• Entering an incorrect digit in Normal mode gives you a STRIKE.");
    mvaddstr(start_y + 9, center_x - 30, "• Reach 3 strikes and it's Game Over!");
    mvaddstr(start_y + 10, center_x - 30, "• Toggle Note mode [N] to safely pencil candidates with no strikes.");
    attroff(COLOR_PAIR(PAIR_DEFAULT));

    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    mvaddstr(start_y + 12, center_x - 30, "CONTROLS & SHORTCUTS:");
    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);

    attron(COLOR_PAIR(PAIR_DEFAULT));
    mvaddstr(start_y + 13, center_x - 30, "• Navigation   : Arrow keys, WASD, or Vim [H, J, K, L]");
    mvaddstr(start_y + 14, center_x - 30, "• Digits / Note: Number keys [1 - 9]");
    mvaddstr(start_y + 15, center_x - 30, "• Notes Mode   : [N] (Toggle pencil candidates)");
    mvaddstr(start_y + 16, center_x - 30, "• Undo / Redo  : [U] to undo, [R] to redo");
    mvaddstr(start_y + 17, center_x - 30, "• Hints        : [H] for smart step-by-step logical deduction");
    mvaddstr(start_y + 18, center_x - 30, "• Themes       : [T] to instantly cycle color theme");
    mvaddstr(start_y + 19, center_x - 30, "• Mouse        : Click to select cells, click keypad, click menus");
    attroff(COLOR_PAIR(PAIR_DEFAULT));

    attron(COLOR_PAIR(PAIR_STATUS_LABEL));
    std::string footer = "[Esc / Enter / M] Return to Main Menu";
    mvaddstr(start_y + 20, center_x - static_cast<int>(footer.length()) / 2, footer.c_str());
    attroff(COLOR_PAIR(PAIR_STATUS_LABEL));
}

bool Renderer::board_cell_from_screen(int screen_x, int screen_y, int& out_row, int& out_col) const {
    int rel_y = screen_y - board_top_y_ - 1;
    int rel_x = screen_x - board_left_x_ - 1;

    if (rel_y < 0 || rel_y >= 18 || rel_x < 0 || rel_x >= 36) return false;
    if (rel_y % 2 != 0) return false; // Clicked on divider line

    int row = rel_y / 2;
    int col = rel_x / 4;

    if (row >= 0 && row < 9 && col >= 0 && col < 9) {
        out_row = row;
        out_col = col;
        return true;
    }
    return false;
}

bool Renderer::keypad_digit_from_screen(int screen_x, int screen_y, int& out_digit) const {
    int rel_y = screen_y - keypad_top_y_ - 1;
    int rel_x = screen_x - keypad_left_x_;

    if (rel_y < 0 || rel_y > 1 || rel_x < 0 || rel_x >= 30) return false;

    int col_idx = rel_x / 6;
    if (col_idx < 0 || col_idx >= 5) return false;

    int digit = rel_y * 5 + col_idx + 1;
    if (digit >= 1 && digit <= 9) {
        out_digit = digit;
        return true;
    }
    return false;
}

bool Renderer::menu_item_from_screen(int screen_x, int screen_y, int item_count, int& out_item) const {
    (void)screen_x;
    int rel_y = screen_y - menu_top_y_;
    if (rel_y < 0) return false;

    int item = rel_y / 2;
    if (item >= 0 && item < item_count && rel_y % 2 == 0) {
        out_item = item;
        return true;
    }
    return false;
}

} // namespace sudoku
