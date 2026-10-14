#include "GameEngine.hpp"
#include "../core/Generator.hpp"
#include <ctime>
#include <iomanip>

namespace sudoku {

GameEngine& GameEngine::instance() {
    static GameEngine engine;
    return engine;
}

GameEngine::GameEngine() {
    init();
}

void GameEngine::init() {
    state_ = GameState::MAIN_MENU;
    cursor_ = {4, 4};
    note_mode_ = false;
    strikes_ = 0;
    hints_used_ = 0;
    difficulty_ = Difficulty::MEDIUM;
    timer_running_ = false;
    accumulated_seconds_ = 0;
    menu_selection_ = 0;
    score_submitted_ = false;
}

void GameEngine::update() {
    if (status_ticks_ > 0) {
        status_ticks_--;
        if (status_ticks_ == 0) {
            status_message_.clear();
            board_.clear_all_errors();
        }
    }
}

void GameEngine::start_new_game(Difficulty diff) {
    difficulty_ = diff;
    board_ = Generator::generate(diff);
    cursor_ = {0, 0};
    note_mode_ = false;
    strikes_ = 0;
    hints_used_ = 0;
    undo_stack_.clear();
    redo_stack_.clear();
    active_hint_ = HintResult{};
    accumulated_seconds_ = 0;
    timer_start_ = std::chrono::steady_clock::now();
    timer_running_ = true;
    score_submitted_ = false;
    state_ = GameState::PLAYING;
    set_status_message("Game started! Difficulty: " + difficulty_to_string(diff) + ". Enjoy!", 50);
}

void GameEngine::pause_game() {
    if (state_ == GameState::PLAYING) {
        if (timer_running_) {
            auto now = std::chrono::steady_clock::now();
            accumulated_seconds_ += static_cast<int>(
                std::chrono::duration_cast<std::chrono::seconds>(now - timer_start_).count());
            timer_running_ = false;
        }
        state_ = GameState::PAUSED;
    }
}

void GameEngine::resume_game() {
    if (state_ == GameState::PAUSED) {
        timer_start_ = std::chrono::steady_clock::now();
        timer_running_ = true;
        state_ = GameState::PLAYING;
    }
}

void GameEngine::restart_current_game() {
    // Clear user moves and reset errors/notes
    for (int r = 0; r < Board::SIZE; ++r) {
        for (int c = 0; c < Board::SIZE; ++c) {
            if (!board_.is_given(r, c)) {
                board_.clear_value(r, c);
                board_.clear_notes(r, c);
                board_.set_error(r, c, false);
            }
        }
    }
    strikes_ = 0;
    hints_used_ = 0;
    undo_stack_.clear();
    redo_stack_.clear();
    active_hint_ = HintResult{};
    accumulated_seconds_ = 0;
    timer_start_ = std::chrono::steady_clock::now();
    timer_running_ = true;
    score_submitted_ = false;
    state_ = GameState::PLAYING;
    set_status_message("Game restarted. Good luck!", 40);
}

void GameEngine::return_to_main_menu() {
    timer_running_ = false;
    state_ = GameState::MAIN_MENU;
    menu_selection_ = 0;
}

int GameEngine::elapsed_seconds() const {
    int total = accumulated_seconds_;
    if (timer_running_) {
        auto now = std::chrono::steady_clock::now();
        total += static_cast<int>(
            std::chrono::duration_cast<std::chrono::seconds>(now - timer_start_).count());
    }
    return total;
}

int GameEngine::current_score() const {
    return ScoreManager::calculate_score(difficulty_, elapsed_seconds(), strikes_, hints_used_);
}

void GameEngine::move_cursor(int d_row, int d_col) {
    cursor_.row = (cursor_.row + d_row + Board::SIZE) % Board::SIZE;
    cursor_.col = (cursor_.col + d_col + Board::SIZE) % Board::SIZE;
}

void GameEngine::set_cursor(int row, int col) {
    if (row >= 0 && row < Board::SIZE && col >= 0 && col < Board::SIZE) {
        cursor_ = {row, col};
    }
}

void GameEngine::toggle_note_mode() {
    note_mode_ = !note_mode_;
    set_status_message(note_mode_ ? "Pencil Notes Mode: ON" : "Normal Entry Mode: ON", 35);
}

void GameEngine::clear_active_hint() {
    active_hint_ = HintResult{};
}

void GameEngine::enter_digit(int digit) {
    if (digit < 1 || digit > 9) return;
    if (state_ != GameState::PLAYING) return;

    int r = cursor_.row;
    int c = cursor_.col;

    if (board_.is_given(r, c)) {
        set_status_message("Starting clue cell cannot be modified.", 30);
        return;
    }

    if (note_mode_) {
        // Pencil note mode: toggle candidate note without penalty
        MoveRecord rec;
        rec.pos = cursor_;
        rec.type = MoveType::TOGGLE_NOTE;
        rec.prev_notes = board_.get_notes(r, c);
        board_.toggle_note(r, c, digit);
        rec.new_notes = board_.get_notes(r, c);
        undo_stack_.push_back(rec);
        redo_stack_.clear();
        return;
    }

    // Normal mode entry
    int current_val = board_.get_value(r, c);
    if (current_val == digit) return; // Already entered

    int correct_val = board_.get_solution(r, c);
    if (digit == correct_val) {
        // Correct placement
        MoveRecord rec;
        rec.pos = cursor_;
        rec.type = MoveType::SET_DIGIT;
        rec.prev_val = current_val;
        rec.new_val = digit;
        rec.prev_notes = board_.get_notes(r, c);

        board_.set_value(r, c, digit);
        board_.clear_notes(r, c);
        board_.set_error(r, c, false);

        // Remove this digit from peer cells' pencil marks
        rec.affected_peer_notes = board_.remove_note_from_peers(r, c, digit);

        undo_stack_.push_back(rec);
        redo_stack_.clear();

        // Clear active hint if placed at hint cell
        if (active_hint_.found && active_hint_.pos == cursor_) {
            active_hint_ = HintResult{};
        }

        check_victory();
    } else {
        // Strike! Incorrect digit placement
        strikes_++;
        board_.set_error(r, c, true);
        set_status_message("Strike " + std::to_string(strikes_) + "/" +
                           std::to_string(MAX_STRIKES) + "! " +
                           std::to_string(digit) + " is incorrect for this cell.", 60);

        if (strikes_ >= MAX_STRIKES) {
            timer_running_ = false;
            state_ = GameState::GAME_OVER_STRIKES;
        }
    }
}

void GameEngine::clear_current_cell() {
    if (state_ != GameState::PLAYING) return;
    int r = cursor_.row;
    int c = cursor_.col;

    if (board_.is_given(r, c)) {
        set_status_message("Starting clue cell cannot be modified.", 30);
        return;
    }

    int current_val = board_.get_value(r, c);
    uint16_t current_notes = board_.get_notes(r, c);

    if (current_val == 0 && current_notes == 0) return;

    MoveRecord rec;
    rec.pos = cursor_;
    rec.type = MoveType::CLEAR_CELL;
    rec.prev_val = current_val;
    rec.new_val = 0;
    rec.prev_notes = current_notes;
    rec.new_notes = 0;

    board_.clear_value(r, c);
    board_.clear_notes(r, c);
    board_.set_error(r, c, false);

    undo_stack_.push_back(rec);
    redo_stack_.clear();
}

void GameEngine::undo() {
    if (undo_stack_.empty() || state_ != GameState::PLAYING) {
        set_status_message("Nothing to undo.", 25);
        return;
    }

    MoveRecord move = undo_stack_.back();
    undo_stack_.pop_back();

    int r = move.pos.row;
    int c = move.pos.col;

    if (move.type == MoveType::SET_DIGIT) {
        board_.set_value(r, c, move.prev_val);
        board_.set_notes(r, c, move.prev_notes);
        // Restore affected peer notes
        for (const auto& peer : move.affected_peer_notes) {
            board_.get_cell_mut(peer.first.row, peer.first.col).notes = peer.second;
        }
    } else if (move.type == MoveType::TOGGLE_NOTE) {
        board_.set_notes(r, c, move.prev_notes);
    } else if (move.type == MoveType::CLEAR_CELL) {
        board_.set_value(r, c, move.prev_val);
        board_.set_notes(r, c, move.prev_notes);
    }

    board_.set_error(r, c, false);
    cursor_ = move.pos;
    redo_stack_.push_back(move);
    set_status_message("Action undone.", 25);
}

void GameEngine::redo() {
    if (redo_stack_.empty() || state_ != GameState::PLAYING) {
        set_status_message("Nothing to redo.", 25);
        return;
    }

    MoveRecord move = redo_stack_.back();
    redo_stack_.pop_back();

    int r = move.pos.row;
    int c = move.pos.col;

    if (move.type == MoveType::SET_DIGIT) {
        board_.set_value(r, c, move.new_val);
        board_.clear_notes(r, c);
        for (const auto& peer : move.affected_peer_notes) {
            board_.remove_note(peer.first.row, peer.first.col, move.new_val);
        }
    } else if (move.type == MoveType::TOGGLE_NOTE) {
        board_.set_notes(r, c, move.new_notes);
    } else if (move.type == MoveType::CLEAR_CELL) {
        board_.clear_value(r, c);
        board_.clear_notes(r, c);
    }

    cursor_ = move.pos;
    undo_stack_.push_back(move);
    set_status_message("Action redone.", 25);
}

void GameEngine::request_hint() {
    if (state_ != GameState::PLAYING) return;

    active_hint_ = HintEngine::get_hint(board_);
    if (active_hint_.found) {
        cursor_ = active_hint_.pos;
        hints_used_++;
        set_status_message("Hint [" + active_hint_.technique + "]: " + active_hint_.explanation, 140);
    } else {
        set_status_message("No hints available for current board state.", 40);
    }
}

void GameEngine::check_victory() {
    if (board_.is_solved()) {
        timer_running_ = false;
        state_ = GameState::GAME_OVER_VICTORY;
        final_score_ = ScoreManager::calculate_score(difficulty_, elapsed_seconds(), strikes_, hints_used_);
        score_submitted_ = false;
    }
}

void GameEngine::set_status_message(const std::string& msg, int duration_ticks) {
    status_message_ = msg;
    status_ticks_ = duration_ticks;
}

void GameEngine::append_player_name(char c) {
    if (player_name_.length() < 16 && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ' || c == '_')) {
        player_name_.push_back(c);
    }
}

void GameEngine::pop_player_name() {
    if (!player_name_.empty()) {
        player_name_.pop_back();
    }
}

void GameEngine::submit_victory_score() {
    if (score_submitted_) return;
    std::string name = player_name_.empty() ? "Player" : player_name_;

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_c);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M");

    ScoreEntry entry;
    entry.player_name = name;
    entry.difficulty = difficulty_;
    entry.score = final_score_;
    entry.time_seconds = elapsed_seconds();
    entry.mistakes = strikes_;
    entry.hints_used = hints_used_;
    entry.date_str = ss.str();

    ScoreManager::instance().add_score(entry);
    score_submitted_ = true;
    set_status_message("High score recorded to leaderboard!", 50);
}

void GameEngine::menu_up(int count) {
    menu_selection_ = (menu_selection_ - 1 + count) % count;
}

void GameEngine::menu_down(int count) {
    menu_selection_ = (menu_selection_ + 1) % count;
}

} // namespace sudoku

