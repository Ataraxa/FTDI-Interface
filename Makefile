# Compiler and flags (Windows-compatible)
CXX := g++
CXXFLAGS := -I./include -std=c++17
LDFLAGS := -L./lib
LDLIBS := -lmpsse -lftd2xx64

# Paths (use forward slashes)
SRC_DIR := src
UTILS_DIR := utils
BIN_DIR := bin
OBJ_DIR := lib/compiled

# For thread-specific files in src/
THREAD_SRCS := $(wildcard $(SRC_DIR)/thread*.cpp)
THREAD_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(THREAD_SRCS))

# Files
UTILS_SRCS := $(wildcard $(UTILS_DIR)/*.cpp)
UTILS_OBJS := $(patsubst $(UTILS_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(UTILS_SRCS))
TARGET ?= core

# Main executable
$(BIN_DIR)/$(TARGET).exe: $(SRC_DIR)/$(TARGET).cpp $(UTILS_OBJS) $(THREAD_OBJS)
	@echo "=== Linking $@ ==="
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@ $(LDLIBS)

# Compile thread*.cpp files from src/ to .o files
$(OBJ_DIR)/thread%.o: $(SRC_DIR)/thread%.cpp
	@echo "=== Compiling $< ==="
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Compile utils/ to .o files
$(OBJ_DIR)/%.o: $(UTILS_DIR)/%.cpp
	@echo "=== Compiling $< ==="
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean (Windows-compatible)
clean:
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"
	@echo "Cleaned!"