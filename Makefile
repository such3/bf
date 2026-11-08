# Makefile (project root)
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined

bf: src/bf.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

test: bf
	./tests/run_tests.sh ./bf

clean:
	rm -f bf

.PHONY: test clean
