# Compiler settings
CXX = g++
CXXFLAGS = -I./include   # Include paths
LDFLAGS = -L./lib                             # Library path
LDLIBS = -lmpsse -lftd2xx64                   # Libraries to link

# Paths
SRC_DIR = src
UTILS_DIR = utils
BIN_DIR = bin

# List of utility objects (e.g., "../utils/utils.o")
UTILS_OBJ := $(wildcard lib/compiled/*.o)

# Rule to compile a specific .cpp file from src/ into bin/
# Usage: make TARGET=core
TARGET = core # Default target (no extension)

# Build rule
$(BIN_DIR)/$(TARGET).exe: $(SRC_DIR)/$(TARGET).cpp $(UTILS_OBJ)
	@echo "Building $@..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -o $@

# Shortcut
build: $(BIN_DIR)/$(TARGET).exe