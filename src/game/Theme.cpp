#define _XOPEN_SOURCE_EXTENDED 1
#include "Theme.hpp"
#include <ncursesw/ncurses.h>

namespace sudoku {

ThemeManager& ThemeManager::instance() {
    static ThemeManager mgr;
    return mgr;
}

ThemeManager::ThemeManager() {
    setup_themes();
}

void ThemeManager::setup_themes() {
    themes_ = {
        {
            ThemeId::TOKYO_NIGHT,
            "Tokyo Night",
            "A clean, dark cyber-indigo theme with vivid cyan and purple accents",
            false,
            0x1a1b26, 0xc0caf5, // bg, fg
            0x7aa2f7, 0x3d59a1, 0x24283b, // border outer, box, inner
            0x7dcfff, 0x9ece6a, 0x565f89, // given, user, note
            0x3d59a1, 0xffffff, // cursor bg, fg
            0x1f2335,           // crosshair
            0x292e42, 0xff9e64, // highlight same bg, fg
            0xdb4b4b, 0xffffff, // error
            0xbb9af7, 0x1a1b26, // hint
            0xbb9af7,           // accent
            0xf7768e, 0x414868  // strike active, inactive
        },
        {
            ThemeId::DRACULA,
            "Dracula",
            "Classic gothic dark theme with hot pink and mint green contrasts",
            false,
            0x282a36, 0xf8f8f2,
            0xbd93f9, 0x6272a4, 0x44475a,
            0x8be9fd, 0x50fa7b, 0x6272a4,
            0x44475a, 0xffffff,
            0x343746,
            0x44475a, 0xffb86c,
            0xff5555, 0xffffff,
            0xff79c6, 0x282a36,
            0xff79c6,
            0xff5555, 0x6272a4
        },
        {
            ThemeId::NORD,
            "Nord Arctic",
            "Cool arctic ice slate with frost cyan and snow white clarity",
            false,
            0x2e3440, 0xeceff4,
            0x88c0d0, 0x81a1c1, 0x3b4252,
            0x88c0d0, 0xa3be8c, 0x4c566a,
            0x434c5e, 0xffffff,
            0x3b4252,
            0x4c566a, 0xebcb8b,
            0xbf616a, 0xffffff,
            0xb48ead, 0x2e3440,
            0x88c0d0,
            0xbf616a, 0x4c566a
        },
        {
            ThemeId::CYBERPUNK,
            "Cyberpunk Neon",
            "High-contrast obsidian black with electric yellow and neon magenta",
            false,
            0x0d0e15, 0xfcee0a,
            0x00f0ff, 0xff007f, 0x25283d,
            0x00f0ff, 0x00ff66, 0x5b6282,
            0xff007f, 0xffffff,
            0x1a1d2e,
            0x301934, 0xfcee0a,
            0xff003c, 0xffffff,
            0xfcee0a, 0x000000,
            0x00f0ff,
            0xff003c, 0x40445a
        },
        {
            ThemeId::MONOKAI,
            "Monokai Pro",
            "Beloved coding palette with warm charcoal, neon lime, and sunset orange",
            false,
            0x272822, 0xf8f8f2,
            0x66d9ef, 0xf92672, 0x3e3d32,
            0x66d9ef, 0xa6e22e, 0x75715e,
            0x49483e, 0xffffff,
            0x343329,
            0x49483e, 0xfd971f,
            0xf92672, 0xffffff,
            0xe6db74, 0x272822,
            0xa6e22e,
            0xf92672, 0x75715e
        },
        {
            ThemeId::PAPER_LIGHT,
            "Paper Minimal",
            "Warm daylight parchment with crisp typography and gentle tones",
            true,
            0xfaf8f5, 0x2c3e50,
            0x34495e, 0x7f8c8d, 0xd5dbdb,
            0x16a085, 0x2980b9, 0x95a5a6,
            0xd5dbdb, 0x000000,
            0xeaecee,
            0xf9e79f, 0x7d6608,
            0xe74c3c, 0xffffff,
            0xf39c12, 0xffffff,
            0x2980b9,
            0xc0392b, 0xbdc3c7
        }
    };
}

void ThemeManager::init() {
    if (!has_colors()) return;
    start_color();
    use_default_colors();
    can_change_colors_ = can_change_color();
    colors_initialized_ = true;
    apply_current_theme();
}

void ThemeManager::set_theme(ThemeId id) {
    if (static_cast<int>(id) >= 0 && static_cast<int>(id) < static_cast<int>(ThemeId::COUNT)) {
        current_theme_id_ = id;
        apply_current_theme();
    }
}

void ThemeManager::next_theme() {
    int next = (static_cast<int>(current_theme_id_) + 1) % static_cast<int>(ThemeId::COUNT);
    set_theme(static_cast<ThemeId>(next));
}

void ThemeManager::prev_theme() {
    int prev = (static_cast<int>(current_theme_id_) - 1 + static_cast<int>(ThemeId::COUNT)) % static_cast<int>(ThemeId::COUNT);
    set_theme(static_cast<ThemeId>(prev));
}

void ThemeManager::apply_current_theme() {
    if (!colors_initialized_) return;
    init_theme_colors(current_theme());
}

void ThemeManager::init_theme_colors(const ThemeDefinition& t) {
    if (can_change_colors_ && COLORS >= 64) {
        // Redefine custom color indices (30 to 49)
        enum CustomColor {
            CC_BG = 30, CC_FG, CC_BORDER_OUTER, CC_BORDER_BOX, CC_BORDER_INNER,
            CC_GIVEN, CC_USER, CC_NOTE, CC_CURSOR_BG, CC_CURSOR_FG,
            CC_CROSSHAIR_BG, CC_HIGHLIGHT_BG, CC_HIGHLIGHT_FG, CC_ERROR_BG,
            CC_ERROR_FG, CC_HINT_BG, CC_HINT_FG, CC_ACCENT, CC_STRIKE_ACT, CC_STRIKE_INACT
        };

        auto set_c = [](int idx, uint32_t hex) {
            auto rgb = RgbColor::from_hex(hex);
            init_color(idx, rgb.r, rgb.g, rgb.b);
        };

        set_c(CC_BG, t.bg);
        set_c(CC_FG, t.fg);
        set_c(CC_BORDER_OUTER, t.border_outer);
        set_c(CC_BORDER_BOX, t.border_box);
        set_c(CC_BORDER_INNER, t.border_inner);
        set_c(CC_GIVEN, t.given_digit);
        set_c(CC_USER, t.user_digit);
        set_c(CC_NOTE, t.note_digit);
        set_c(CC_CURSOR_BG, t.cursor_bg);
        set_c(CC_CURSOR_FG, t.cursor_fg);
        set_c(CC_CROSSHAIR_BG, t.crosshair_bg);
        set_c(CC_HIGHLIGHT_BG, t.highlight_same_bg);
        set_c(CC_HIGHLIGHT_FG, t.highlight_same_fg);
        set_c(CC_ERROR_BG, t.error_bg);
        set_c(CC_ERROR_FG, t.error_fg);
        set_c(CC_HINT_BG, t.hint_bg);
        set_c(CC_HINT_FG, t.hint_fg);
        set_c(CC_ACCENT, t.accent);
        set_c(CC_STRIKE_ACT, t.strike_active);
        set_c(CC_STRIKE_INACT, t.strike_inactive);

        init_pair(PAIR_DEFAULT, CC_FG, CC_BG);
        init_pair(PAIR_BOARD_BORDER_OUTER, CC_BORDER_OUTER, CC_BG);
        init_pair(PAIR_BOARD_BORDER_BOX, CC_BORDER_BOX, CC_BG);
        init_pair(PAIR_BOARD_BORDER_INNER, CC_BORDER_INNER, CC_BG);
        init_pair(PAIR_GIVEN_DIGIT, CC_GIVEN, CC_BG);
        init_pair(PAIR_USER_DIGIT, CC_USER, CC_BG);
        init_pair(PAIR_NOTE_DIGIT, CC_NOTE, CC_BG);
        init_pair(PAIR_CURSOR, CC_CURSOR_FG, CC_CURSOR_BG);
        init_pair(PAIR_CURSOR_CROSSHAIR, CC_FG, CC_CROSSHAIR_BG);
        init_pair(PAIR_HIGHLIGHT_SAME, CC_HIGHLIGHT_FG, CC_HIGHLIGHT_BG);
        init_pair(PAIR_ERROR, CC_ERROR_FG, CC_ERROR_BG);
        init_pair(PAIR_HINT, CC_HINT_FG, CC_HINT_BG);
        init_pair(PAIR_TITLE, CC_ACCENT, CC_BG);
        init_pair(PAIR_BADGE_EASY, COLOR_GREEN, CC_BG);
        init_pair(PAIR_BADGE_MEDIUM, COLOR_YELLOW, CC_BG);
        init_pair(PAIR_BADGE_HARD, COLOR_RED, CC_BG);
        init_pair(PAIR_BADGE_EXPERT, COLOR_MAGENTA, CC_BG);
        init_pair(PAIR_STATUS_LABEL, CC_NOTE, CC_BG);
        init_pair(PAIR_STATUS_VALUE, CC_FG, CC_BG);
        init_pair(PAIR_STRIKE_ACTIVE, CC_STRIKE_ACT, CC_BG);
        init_pair(PAIR_STRIKE_INACTIVE, CC_STRIKE_INACT, CC_BG);
        init_pair(PAIR_KEYPAD_DIGIT, CC_FG, CC_CROSSHAIR_BG);
        init_pair(PAIR_KEYPAD_COMPLETED, CC_NOTE, CC_BG);
        init_pair(PAIR_MENU_SELECTED, CC_BG, CC_ACCENT);
        init_pair(PAIR_MENU_NORMAL, CC_FG, CC_BG);
        init_pair(PAIR_VICTORY_BANNER, COLOR_YELLOW, CC_BG);
        init_pair(PAIR_FIREWORK_SPARK, COLOR_CYAN, CC_BG);
    } else {
        // Fallback for standard 8/16-color terminals
        init_pair(PAIR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
        init_pair(PAIR_BOARD_BORDER_OUTER, COLOR_CYAN, COLOR_BLACK);
        init_pair(PAIR_BOARD_BORDER_BOX, COLOR_BLUE, COLOR_BLACK);
        init_pair(PAIR_BOARD_BORDER_INNER, COLOR_BLACK, COLOR_BLACK);
        init_pair(PAIR_GIVEN_DIGIT, COLOR_CYAN, COLOR_BLACK);
        init_pair(PAIR_USER_DIGIT, COLOR_GREEN, COLOR_BLACK);
        init_pair(PAIR_NOTE_DIGIT, COLOR_YELLOW, COLOR_BLACK);
        init_pair(PAIR_CURSOR, COLOR_BLACK, COLOR_CYAN);
        init_pair(PAIR_CURSOR_CROSSHAIR, COLOR_WHITE, COLOR_BLACK);
        init_pair(PAIR_HIGHLIGHT_SAME, COLOR_YELLOW, COLOR_BLUE);
        init_pair(PAIR_ERROR, COLOR_WHITE, COLOR_RED);
        init_pair(PAIR_HINT, COLOR_BLACK, COLOR_YELLOW);
        init_pair(PAIR_TITLE, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(PAIR_BADGE_EASY, COLOR_GREEN, COLOR_BLACK);
        init_pair(PAIR_BADGE_MEDIUM, COLOR_YELLOW, COLOR_BLACK);
        init_pair(PAIR_BADGE_HARD, COLOR_RED, COLOR_BLACK);
        init_pair(PAIR_BADGE_EXPERT, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(PAIR_STATUS_LABEL, COLOR_WHITE, COLOR_BLACK);
        init_pair(PAIR_STATUS_VALUE, COLOR_YELLOW, COLOR_BLACK);
        init_pair(PAIR_STRIKE_ACTIVE, COLOR_RED, COLOR_BLACK);
        init_pair(PAIR_STRIKE_INACTIVE, COLOR_WHITE, COLOR_BLACK);
        init_pair(PAIR_KEYPAD_DIGIT, COLOR_WHITE, COLOR_BLACK);
        init_pair(PAIR_KEYPAD_COMPLETED, COLOR_WHITE, COLOR_BLACK);
        init_pair(PAIR_MENU_SELECTED, COLOR_BLACK, COLOR_CYAN);
        init_pair(PAIR_MENU_NORMAL, COLOR_WHITE, COLOR_BLACK);
        init_pair(PAIR_VICTORY_BANNER, COLOR_YELLOW, COLOR_BLACK);
        init_pair(PAIR_FIREWORK_SPARK, COLOR_CYAN, COLOR_BLACK);
    }
}

} // namespace sudoku

