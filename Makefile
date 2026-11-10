# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined -Isrc

# Source files
SRC = src/bf.cpp \
      src/loader/loader.cpp \
      src/cli/cli.cpp \
	  src/parser/parser.cpp

# Executable
TARGET = bf

# Default input file
INPUT = src/input.bf

# Verbose mode (default: disabled)
VERBOSE ?= 0
MAX_STEPS ?= 10000000


ifeq ($(VERBOSE),1)
    FLAGS = -v
else
    FLAGS =
endif

# Default target
all: $(TARGET)

# Compile
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Manual execution
run: $(TARGET)
	./$(TARGET)  $(INPUT) $(FLAGS) --max-steps $(MAX_STEPS)

# Automated tests
test: $(TARGET)
	./tests/run_tests.sh ./$(TARGET)

# Clean
clean:
	rm -f $(TARGET)

.PHONY: all run test clean