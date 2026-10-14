#define _XOPEN_SOURCE_EXTENDED 1
#include "core/Board.hpp"
#include "core/Solver.hpp"
#include "core/Generator.hpp"
#include "core/HintEngine.hpp"
#include "game/GameEngine.hpp"
#include "game/ScoreManager.hpp"
#include "game/Theme.hpp"
#include "ui/Renderer.hpp"
#include "ui/Input.hpp"

#include <csignal>
#include <chrono>
#include <thread>
#include <iostream>

using namespace sudoku;

void clean_exit(int sig) {
    (void)sig;
    Renderer::instance().shutdown();
    std::exit(0);
}

int main(int argc, char* argv[]) {
    // Command line arguments check
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Modern C++ Sudoku - Standard Rules & 3-Strikes Mode\n";
            std::cout << "Usage:\n";
            std::cout << "  " << argv[0] << "              Run interactive TUI game\n";
            std::cout << "  " << argv[0] << " --help       Show this help message\n";
            std::cout << "  " << argv[0] << " --version    Show version info\n";
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            std::cout << "Modern C++ Sudoku v1.0.0 (Custom Engine)\n";
            return 0;
        }
    }

    // Register signal handlers for clean terminal restoration
    std::signal(SIGINT, clean_exit);
    std::signal(SIGTERM, clean_exit);

    // Initialize UI and Game Engine
    Renderer::instance().init();
    GameEngine::instance().init();

    // Game loop: ~60 ticks per second
    constexpr auto FRAME_DURATION = std::chrono::milliseconds(16);

    while (true) {
        auto frame_start = std::chrono::steady_clock::now();

        // 1. Process user inputs (Keyboard & Mouse)
        if (!InputHandler::instance().process_input()) {
            break; // Exit requested
        }

        // 2. Update engine state (Timers, notifications, victory checks)
        GameEngine::instance().update();

        // 3. Render current frame
        Renderer::instance().render();

        // 4. Frame rate throttling
        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start);
        if (elapsed < FRAME_DURATION) {
            std::this_thread::sleep_for(FRAME_DURATION - elapsed);
        }
    }

    Renderer::instance().shutdown();
    return 0;
}

