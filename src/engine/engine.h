#include  <stdexcept>
#include "./../machine/machine.h"
#include "./../cli/cli.h"
#include <iostream>
#include <unordered_map>

#include "./../exceptions/exceptions.h"

void executeCode(MachineState &machine , Options &options, std::unordered_map<int , int> &positions , std::unordered_map<int, int> &revpos); 