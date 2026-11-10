#include  <iostream>
#include <stdexcept>
#include  "./../machine/machine.h"
#include "./../cli/cli.h"
#include <unordered_map>

class InvalidBracketsException : public std::runtime_error {
    public:
        explicit InvalidBracketsException(const std::string &message) : std::runtime_error(message) {}
};


void validateBrackets(MachineState &machine, std::unordered_map<int, int> &pos,std::unordered_map<int, int> &rpos);

void printMapTrace(Options &options , std::unordered_map<int, int> &pos,std::unordered_map<int, int> &rpos);