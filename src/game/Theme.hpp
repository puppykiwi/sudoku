#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace sudoku {

enum class ThemeId {
    TOKYO_NIGHT = 0,
    DRACULA = 1,
    NORD = 2,
    CYBERPUNK = 3,
    MONOKAI = 4,
    PAPER_LIGHT = 5,
    COUNT
};

// Logical color pair IDs for ncurses
enum ColorRole {
    PAIR_DEFAULT = 1,
    PAIR_BOARD_BORDER_OUTER,
    PAIR_BOARD_BORDER_BOX,
    PAIR_BOARD_BORDER_INNER,
    PAIR_GIVEN_DIGIT,
    PAIR_USER_DIGIT,
    PAIR_NOTE_DIGIT,
    PAIR_CURSOR,
    PAIR_CURSOR_CROSSHAIR,
    PAIR_HIGHLIGHT_SAME,
    PAIR_ERROR,
    PAIR_HINT,
    PAIR_TITLE,
    PAIR_BADGE_EASY,
    PAIR_BADGE_MEDIUM,
    PAIR_BADGE_HARD,
    PAIR_BADGE_EXPERT,
    PAIR_STATUS_LABEL,
    PAIR_STATUS_VALUE,
    PAIR_STRIKE_ACTIVE,
    PAIR_STRIKE_INACTIVE,
    PAIR_KEYPAD_DIGIT,
    PAIR_KEYPAD_COMPLETED,
    PAIR_MENU_SELECTED,
    PAIR_MENU_NORMAL,
    PAIR_VICTORY_BANNER,
    PAIR_FIREWORK_SPARK
};

struct RgbColor {
    int16_t r{0}; // 0-1000 for ncurses
    int16_t g{0};
    int16_t b{0};

    static RgbColor from_hex(uint32_t hex) {
        int r8 = (hex >> 16) & 0xFF;
        int g8 = (hex >> 8) & 0xFF;
        int b8 = hex & 0xFF;
        return {
            static_cast<int16_t>((r8 * 1000) / 255),
            static_cast<int16_t>((g8 * 1000) / 255),
            static_cast<int16_t>((b8 * 1000) / 255)
        };
    }
};

struct ThemeDefinition {
    ThemeId id;
    std::string name;
    std::string description;
    bool is_light;

    // Palette hex values
    uint32_t bg;
    uint32_t fg;
    uint32_t border_outer;
    uint32_t border_box;
    uint32_t border_inner;
    uint32_t given_digit;
    uint32_t user_digit;
    uint32_t note_digit;
    uint32_t cursor_bg;
    uint32_t cursor_fg;
    uint32_t crosshair_bg;
    uint32_t highlight_same_bg;
    uint32_t highlight_same_fg;
    uint32_t error_bg;
    uint32_t error_fg;
    uint32_t hint_bg;
    uint32_t hint_fg;
    uint32_t accent;
    uint32_t strike_active;
    uint32_t strike_inactive;
};

class ThemeManager {
public:
    static ThemeManager& instance();

    void init();
    void set_theme(ThemeId id);
    void next_theme();
    void prev_theme();

    ThemeId current_theme_id() const { return current_theme_id_; }
    const ThemeDefinition& current_theme() const { return themes_[static_cast<int>(current_theme_id_)]; }
    const std::vector<ThemeDefinition>& all_themes() const { return themes_; }

    void apply_current_theme();

private:
    ThemeManager();
    void setup_themes();
    void init_theme_colors(const ThemeDefinition& theme);

    ThemeId current_theme_id_{ThemeId::TOKYO_NIGHT};
    std::vector<ThemeDefinition> themes_;
    bool colors_initialized_{false};
    bool can_change_colors_{false};
};

} // namespace sudoku

