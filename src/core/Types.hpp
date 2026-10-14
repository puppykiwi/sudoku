#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace sudoku {

enum class Difficulty {
    EASY = 0,
    MEDIUM = 1,
    HARD = 2,
    EXPERT = 3
};

inline std::string difficulty_to_string(Difficulty diff) {
    switch (diff) {
        case Difficulty::EASY: return "Easy";
        case Difficulty::MEDIUM: return "Medium";
        case Difficulty::HARD: return "Hard";
        case Difficulty::EXPERT: return "Expert";
        default: return "Unknown";
    }
}

inline int difficulty_base_score(Difficulty diff) {
    switch (diff) {
        case Difficulty::EASY: return 1500;
        case Difficulty::MEDIUM: return 3500;
        case Difficulty::HARD: return 6000;
        case Difficulty::EXPERT: return 10000;
        default: return 1000;
    }
}

inline int difficulty_target_clues(Difficulty diff) {
    switch (diff) {
        case Difficulty::EASY: return 40;    // 38-44 clues
        case Difficulty::MEDIUM: return 34;  // 32-37 clues
        case Difficulty::HARD: return 29;    // 27-31 clues
        case Difficulty::EXPERT: return 24;  // 23-26 clues
        default: return 36;
    }
}

struct CellPos {
    int row{0};
    int col{0};

    bool operator==(const CellPos& other) const {
        return row == other.row && col == other.col;
    }
    bool operator!=(const CellPos& other) const {
        return !(*this == other);
    }
};

enum class MoveType {
    SET_DIGIT,
    CLEAR_DIGIT,
    TOGGLE_NOTE,
    CLEAR_CELL
};

struct MoveRecord {
    CellPos pos;
    MoveType type;
    int prev_val{0};
    int new_val{0};
    uint16_t prev_notes{0};
    uint16_t new_notes{0};
    // If setting a digit removed notes from peer cells, store them for exact undo
    std::vector<std::pair<CellPos, uint16_t>> affected_peer_notes;
};

struct ScoreEntry {
    std::string player_name;
    Difficulty difficulty{Difficulty::MEDIUM};
    int score{0};
    int time_seconds{0};
    int mistakes{0};
    int hints_used{0};
    std::string date_str;
};

enum class GameState {
    MAIN_MENU,
    DIFFICULTY_SELECT,
    PLAYING,
    PAUSED,
    GAME_OVER_VICTORY,
    GAME_OVER_STRIKES,
    LEADERBOARD,
    THEME_SELECT,
    HELP
};

} // namespace sudoku

