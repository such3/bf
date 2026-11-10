#pragma once

#include <string>
#include <stdexcept>
#include "./../machine/machine.h"


struct Options {
    std::string inputFile;
    bool verbose = false;
    long long unsigned maxSteps = MAX_STEPS;
};

class CommandLineArgumentException : public std::runtime_error {
public:
    explicit CommandLineArgumentException(const std::string& message)
        : std::runtime_error(message) {}
};

Options parseArgs(int argc, char* argv[]);