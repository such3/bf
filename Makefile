# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined -Isrc

# Source files
SRC = src/bf.cpp \
      src/loader/loader.cpp \
      src/cli/cli.cpp \
      src/parser/parser.cpp \
      src/tracers/tracers.cpp \
      src/exceptions/exceptions.cpp \
      src/engine/engine.cpp

# Executable
TARGET = bf

# Default input file
INPUT ?= src/input.bf

# Verbose mode (default: disabled)
VERBOSE ?= 0

# Optional maximum execution steps (no default)
MAX_STEPS ?=

ifeq ($(VERBOSE),1)
    FLAGS = -v
else
    FLAGS =
endif

# Run arguments
RUN_ARGS = $(INPUT) $(FLAGS)

ifneq ($(strip $(MAX_STEPS)),)
    RUN_ARGS += --max-steps $(MAX_STEPS)
endif

# Default target
all: $(TARGET)

# Compile
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Manual execution
run: $(TARGET)
	./$(TARGET) $(RUN_ARGS)

# Automated tests
test: $(TARGET)
	./tests/run_tests.sh ./$(TARGET)

# Clean
clean:
	rm -f $(TARGET)

.PHONY: all run test clean