#define _XOPEN_SOURCE_EXTENDED 1
#include "Input.hpp"
#include "Renderer.hpp"
#include "../game/GameEngine.hpp"
#include "../game/Theme.hpp"
#include <ncursesw/ncurses.h>

namespace sudoku {

InputHandler& InputHandler::instance() {
    static InputHandler handler;
    return handler;
}

InputHandler::InputHandler() {}

bool InputHandler::process_input() {
    int ch = getch();
    if (ch == ERR) {
        return !should_exit_;
    }

    if (ch == KEY_MOUSE) {
        handle_mouse();
        return !should_exit_;
    }

    GameState state = GameEngine::instance().state();
    switch (state) {
        case GameState::MAIN_MENU:
            handle_main_menu(ch);
            break;
        case GameState::DIFFICULTY_SELECT:
            handle_difficulty_select(ch);
            break;
        case GameState::PLAYING:
            handle_playing(ch);
            break;
        case GameState::PAUSED:
            handle_paused(ch);
            break;
        case GameState::GAME_OVER_VICTORY:
            handle_victory(ch);
            break;
        case GameState::GAME_OVER_STRIKES:
            handle_game_over_strikes(ch);
            break;
        case GameState::LEADERBOARD:
            handle_leaderboard(ch);
            break;
        case GameState::THEME_SELECT:
            handle_theme_select(ch);
            break;
        case GameState::HELP:
            handle_help(ch);
            break;
    }

    return !should_exit_;
}

void InputHandler::handle_mouse() {
    MEVENT event;
    if (getmouse(&event) == OK) {
        if (event.bstate & (BUTTON1_CLICKED | BUTTON1_PRESSED | BUTTON1_RELEASED)) {
            GameState state = GameEngine::instance().state();

            if (state == GameState::PLAYING) {
                int r = 0, c = 0, digit = 0;
                if (Renderer::instance().board_cell_from_screen(event.x, event.y, r, c)) {
                    GameEngine::instance().set_cursor(r, c);
                } else if (Renderer::instance().keypad_digit_from_screen(event.x, event.y, digit)) {
                    GameEngine::instance().enter_digit(digit);
                }
            } else if (state == GameState::MAIN_MENU) {
                int item = 0;
                if (Renderer::instance().menu_item_from_screen(event.x, event.y, 5, item)) {
                    GameEngine::instance().set_menu_selection(item);
                    handle_main_menu(10); // Trigger Enter
                }
            } else if (state == GameState::DIFFICULTY_SELECT) {
                int item = 0;
                if (Renderer::instance().menu_item_from_screen(event.x, event.y, 4, item)) {
                    GameEngine::instance().set_menu_selection(item);
                    handle_difficulty_select(10);
                }
            } else if (state == GameState::THEME_SELECT) {
                int item = 0;
                if (Renderer::instance().menu_item_from_screen(event.x, event.y, static_cast<int>(ThemeId::COUNT), item)) {
                    GameEngine::instance().set_menu_selection(item);
                    ThemeManager::instance().set_theme(static_cast<ThemeId>(item));
                }
            }
        }
    }
}

void InputHandler::handle_main_menu(int ch) {
    auto& engine = GameEngine::instance();
    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
        case 'k':
        case 'K':
            engine.menu_up(5);
            break;

        case KEY_DOWN:
        case 's':
        case 'S':
        case 'j':
        case 'J':
            engine.menu_down(5);
            break;

        case 10: // Enter
        case ' ':
            switch (engine.menu_selection()) {
                case 0: // New Game
                    engine.set_menu_selection(1); // Default to Medium
                    engine.set_state(GameState::DIFFICULTY_SELECT);
                    break;
                case 1: // Leaderboard
                    engine.set_state(GameState::LEADERBOARD);
                    break;
                case 2: // Themes
                    engine.set_menu_selection(static_cast<int>(ThemeManager::instance().current_theme_id()));
                    engine.set_state(GameState::THEME_SELECT);
                    break;
                case 3: // Help
                    engine.set_state(GameState::HELP);
                    break;
                case 4: // Quit
                    should_exit_ = true;
                    break;
            }
            break;

        case 'q':
        case 'Q':
        case 27: // Esc
            should_exit_ = true;
            break;
    }
}

void InputHandler::handle_difficulty_select(int ch) {
    auto& engine = GameEngine::instance();
    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
        case 'k':
        case 'K':
            engine.menu_up(4);
            break;

        case KEY_DOWN:
        case 's':
        case 'S':
        case 'j':
        case 'J':
            engine.menu_down(4);
            break;

        case 10: // Enter
        case ' ': {
            Difficulty diff = static_cast<Difficulty>(engine.menu_selection());
            engine.start_new_game(diff);
            break;
        }

        case 27: // Esc
        case KEY_BACKSPACE:
        case 127:
        case 'q':
        case 'Q':
            engine.return_to_main_menu();
            break;
    }
}

void InputHandler::handle_playing(int ch) {
    auto& engine = GameEngine::instance();

    switch (ch) {
        // Navigation
        case KEY_UP:
        case 'w':
        case 'W':
        case 'k':
        case 'K':
            engine.move_cursor(-1, 0);
            break;

        case KEY_DOWN:
        case 's':
        case 'S':
        case 'j':
        case 'J':
            engine.move_cursor(1, 0);
            break;

        case KEY_LEFT:
        case 'a':
        case 'A':
            engine.move_cursor(0, -1);
            break;

        case KEY_RIGHT:
        case 'd':
        case 'D':
        case 'l':
        case 'L':
            engine.move_cursor(0, 1);
            break;

        // Digits
        case '1': case '2': case '3':
        case '4': case '5': case '6':
        case '7': case '8': case '9':
            engine.enter_digit(ch - '0');
            break;

        // Note mode toggle
        case 'n':
        case 'N':
            engine.toggle_note_mode();
            break;

        // Clear / Erase cell
        case KEY_BACKSPACE:
        case 127:
        case KEY_DC:
        case 'c':
        case 'C':
        case 'x':
        case 'X':
        case '0':
            engine.clear_current_cell();
            break;

        // Undo & Redo
        case 'u':
        case 'U':
            engine.undo();
            break;

        case 'r':
        case 'R':
            engine.redo();
            break;

        // Hint
        case 'h':
        case 'H':
        case '?':
            engine.request_hint();
            break;

        // Themes switch
        case 't':
        case 'T':
            ThemeManager::instance().next_theme();
            engine.set_status_message("Switched theme to: " + ThemeManager::instance().current_theme().name, 35);
            break;

        // Pause / Menu
        case 'p':
        case 'P':
            engine.pause_game();
            break;

        case 27: // Esc
        case 'm':
        case 'M':
            engine.pause_game();
            break;
    }
}

void InputHandler::handle_paused(int ch) {
    auto& engine = GameEngine::instance();
    switch (ch) {
        case 'p':
        case 'P':
        case 27: // Esc
        case 10: // Enter
            engine.resume_game();
            break;

        case 'r':
        case 'R':
            engine.restart_current_game();
            break;

        case 'm':
        case 'M':
        case 'q':
        case 'Q':
            engine.return_to_main_menu();
            break;
    }
}

void InputHandler::handle_victory(int ch) {
    auto& engine = GameEngine::instance();
    if (ch == 10 || ch == KEY_ENTER) {
        engine.submit_victory_score();
        engine.set_state(GameState::LEADERBOARD);
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
        engine.pop_player_name();
    } else if (ch >= 32 && ch <= 126) {
        engine.append_player_name(static_cast<char>(ch));
    } else if (ch == 27) {
        engine.submit_victory_score();
        engine.return_to_main_menu();
    }
}

void InputHandler::handle_game_over_strikes(int ch) {
    auto& engine = GameEngine::instance();
    switch (ch) {
        case 'r':
        case 'R':
            engine.restart_current_game();
            break;

        case 'n':
        case 'N':
            engine.set_state(GameState::DIFFICULTY_SELECT);
            break;

        case 'm':
        case 'M':
        case 27: // Esc
        case 'q':
        case 'Q':
            engine.return_to_main_menu();
            break;
    }
}

void InputHandler::handle_leaderboard(int ch) {
    auto& engine = GameEngine::instance();
    switch (ch) {
        case 27: // Esc
        case 10: // Enter
        case ' ':
        case 'm':
        case 'M':
        case 'q':
        case 'Q':
            engine.return_to_main_menu();
            break;
    }
}

void InputHandler::handle_theme_select(int ch) {
    auto& engine = GameEngine::instance();
    int count = static_cast<int>(ThemeId::COUNT);

    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
        case 'k':
        case 'K':
            engine.menu_up(count);
            ThemeManager::instance().set_theme(static_cast<ThemeId>(engine.menu_selection()));
            break;

        case KEY_DOWN:
        case 's':
        case 'S':
        case 'j':
        case 'J':
            engine.menu_down(count);
            ThemeManager::instance().set_theme(static_cast<ThemeId>(engine.menu_selection()));
            break;

        case 10: // Enter
        case ' ':
        case 27: // Esc
            engine.return_to_main_menu();
            break;
    }
}

void InputHandler::handle_help(int ch) {
    auto& engine = GameEngine::instance();
    switch (ch) {
        case 27: // Esc
        case 10: // Enter
        case ' ':
        case 'm':
        case 'M':
        case 'q':
        case 'Q':
            engine.return_to_main_menu();
            break;
    }
}

} // namespace sudoku
