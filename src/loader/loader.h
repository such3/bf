#pragma once

#include <string>
#include <stdexcept>

#include "./../machine/machine.h"
#include "./../cli/cli.h"

class InputSourceException : public std::runtime_error {
public:
    explicit InputSourceException(const std::string& message)
        : std::runtime_error(message) {}
};

void loadSource(MachineState& machine, Options& options);