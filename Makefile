CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
LDFLAGS ?= -lncursesw

SRC_DIR = src
BUILD_DIR = build
TARGET = sudoku
TEST_SOLVER_TARGET = test_solver
TEST_GAMEPLAY_TARGET = test_gameplay

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/core/Board.cpp \
       $(SRC_DIR)/core/Solver.cpp \
       $(SRC_DIR)/core/Generator.cpp \
       $(SRC_DIR)/core/HintEngine.cpp \
       $(SRC_DIR)/game/Theme.cpp \
       $(SRC_DIR)/game/ScoreManager.cpp \
       $(SRC_DIR)/game/GameEngine.cpp \
       $(SRC_DIR)/ui/Renderer.cpp \
       $(SRC_DIR)/ui/Input.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

TEST_SOLVER_OBJS = $(BUILD_DIR)/tests/test_solver.o \
                   $(BUILD_DIR)/core/Board.o \
                   $(BUILD_DIR)/core/Solver.o \
                   $(BUILD_DIR)/core/Generator.o \
                   $(BUILD_DIR)/core/HintEngine.o

TEST_GAMEPLAY_OBJS = $(BUILD_DIR)/tests/test_gameplay.o \
                     $(BUILD_DIR)/core/Board.o \
                     $(BUILD_DIR)/core/Solver.o \
                     $(BUILD_DIR)/core/Generator.o \
                     $(BUILD_DIR)/core/HintEngine.o \
                     $(BUILD_DIR)/game/GameEngine.o \
                     $(BUILD_DIR)/game/ScoreManager.o \
                     $(BUILD_DIR)/game/Theme.o

all: $(TARGET) $(TEST_SOLVER_TARGET) $(TEST_GAMEPLAY_TARGET)

$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Build successful: ./$(TARGET)"

$(TEST_SOLVER_TARGET): $(TEST_SOLVER_OBJS)
	@echo "Linking $(TEST_SOLVER_TARGET)..."
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful: ./$(TEST_SOLVER_TARGET)"

$(TEST_GAMEPLAY_TARGET): $(TEST_GAMEPLAY_OBJS)
	@echo "Linking $(TEST_GAMEPLAY_TARGET)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Build successful: ./$(TEST_GAMEPLAY_TARGET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/%.o: tests/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_SOLVER_TARGET) $(TEST_GAMEPLAY_TARGET)
	@echo "=== Running Solver Unit Tests ==="
	./$(TEST_SOLVER_TARGET)
	@echo ""
	@echo "=== Running Gameplay & State Tests ==="
	./$(TEST_GAMEPLAY_TARGET)

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_SOLVER_TARGET) $(TEST_GAMEPLAY_TARGET)

.PHONY: all test clean

