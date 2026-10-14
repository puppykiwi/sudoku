#include "ScoreManager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <sys/stat.h>
#include <unistd.h>

namespace sudoku {

ScoreManager& ScoreManager::instance() {
    static ScoreManager instance;
    return instance;
}

ScoreManager::ScoreManager() {
    load();
}

int ScoreManager::calculate_score(Difficulty diff, int time_seconds, int mistakes, int hints_used) {
    int base = difficulty_base_score(diff);
    int time_deduction = time_seconds * 2;
    int mistake_deduction = mistakes * 300;
    int hint_deduction = hints_used * 250;
    int bonus = (mistakes == 0 ? 500 : 0) + (hints_used == 0 ? 500 : 0);

    int score = base - time_deduction - mistake_deduction - hint_deduction + bonus;
    return std::max(100, score);
}

std::string ScoreManager::format_time(int seconds) {
    int mins = seconds / 60;
    int secs = seconds % 60;
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << mins << ":"
       << std::setfill('0') << std::setw(2) << secs;
    return ss.str();
}

std::string ScoreManager::get_storage_path() const {
    const char* home = getenv("HOME");
    if (home) {
        std::string config_dir = std::string(home) + "/.config/sudoku";
        mkdir(config_dir.c_str(), 0755);
        std::string path = config_dir + "/highscores.json";
        std::ofstream test(path, std::ios::app);
        if (test.is_open()) {
            return path;
        }
    }
    return "highscores.json";
}

bool ScoreManager::save() {
    std::string path = get_storage_path();
    std::ofstream out(path);
    if (!out.is_open()) {
        path = "highscores.json";
        out.open(path);
        if (!out.is_open()) return false;
    }

    out << "[\n";
    for (size_t i = 0; i < scores_.size(); ++i) {
        const auto& s = scores_[i];
        out << "  {\n";
        out << "    \"player_name\": \"" << s.player_name << "\",\n";
        out << "    \"difficulty\": \"" << difficulty_to_string(s.difficulty) << "\",\n";
        out << "    \"score\": " << s.score << ",\n";
        out << "    \"time_seconds\": " << s.time_seconds << ",\n";
        out << "    \"mistakes\": " << s.mistakes << ",\n";
        out << "    \"hints_used\": " << s.hints_used << ",\n";
        out << "    \"date_str\": \"" << s.date_str << "\"\n";
        out << "  }" << (i + 1 < scores_.size() ? ",\n" : "\n");
    }
    out << "]\n";
    return true;
}

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\",");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n\",");
    return str.substr(first, (last - first + 1));
}

bool ScoreManager::load() {
    scores_.clear();
    std::string path = get_storage_path();
    std::ifstream in(path);
    if (!in.is_open()) {
        path = "highscores.json";
        in.open(path);
        if (!in.is_open()) return false;
    }

    std::string line;
    ScoreEntry current;
    bool in_entry = false;

    while (std::getline(in, line)) {
        if (line.find('{') != std::string::npos) {
            current = ScoreEntry{};
            in_entry = true;
            continue;
        }
        if (line.find('}') != std::string::npos && in_entry) {
            scores_.push_back(current);
            in_entry = false;
            continue;
        }

        if (!in_entry) continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string key = trim(line.substr(0, colon));
        std::string val = trim(line.substr(colon + 1));

        if (key == "player_name") {
            current.player_name = val;
        } else if (key == "difficulty") {
            if (val == "Easy") current.difficulty = Difficulty::EASY;
            else if (val == "Medium") current.difficulty = Difficulty::MEDIUM;
            else if (val == "Hard") current.difficulty = Difficulty::HARD;
            else if (val == "Expert") current.difficulty = Difficulty::EXPERT;
        } else if (key == "score") {
            try { current.score = std::stoi(val); } catch (...) {}
        } else if (key == "time_seconds") {
            try { current.time_seconds = std::stoi(val); } catch (...) {}
        } else if (key == "mistakes") {
            try { current.mistakes = std::stoi(val); } catch (...) {}
        } else if (key == "hints_used") {
            try { current.hints_used = std::stoi(val); } catch (...) {}
        } else if (key == "date_str") {
            current.date_str = val;
        }
    }

    return true;
}

void ScoreManager::add_score(const ScoreEntry& entry) {
    scores_.push_back(entry);
    std::sort(scores_.begin(), scores_.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.time_seconds < b.time_seconds;
    });
    save();
}

std::vector<ScoreEntry> ScoreManager::get_top_scores(Difficulty diff, size_t limit) const {
    std::vector<ScoreEntry> filtered;
    for (const auto& s : scores_) {
        if (s.difficulty == diff) {
            filtered.push_back(s);
            if (filtered.size() >= limit) break;
        }
    }
    return filtered;
}

std::vector<ScoreEntry> ScoreManager::get_all_top_scores(size_t limit) const {
    std::vector<ScoreEntry> result;
    for (size_t i = 0; i < scores_.size() && i < limit; ++i) {
        result.push_back(scores_[i]);
    }
    return result;
}

} // namespace sudoku
