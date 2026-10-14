#pragma once

#include "../core/Types.hpp"
#include <vector>
#include <string>

namespace sudoku {

class ScoreManager {
public:
    static ScoreManager& instance();

    // Compute dynamic score based on difficulty, time, strikes/mistakes, and hints
    static int calculate_score(Difficulty diff, int time_seconds, int mistakes, int hints_used);

    // Save and load scores
    bool load();
    bool save();

    void add_score(const ScoreEntry& entry);
    std::vector<ScoreEntry> get_top_scores(Difficulty diff, size_t limit = 10) const;
    std::vector<ScoreEntry> get_all_top_scores(size_t limit = 10) const;

    // Format time in MM:SS
    static std::string format_time(int seconds);

private:
    ScoreManager();
    std::string get_storage_path() const;

    std::vector<ScoreEntry> scores_;
};

} // namespace sudoku

