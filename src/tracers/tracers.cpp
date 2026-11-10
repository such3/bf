#include <iostream>
#include  <string>
#include "tracers.h"


void traceInstruction(char c, Options &options)
{
    if (options.verbose)
    {
        std::clog << "Encountered " << c << std::endl;
    }
}

void dumpState(const MachineState &data, Options &options)
{
    if (options.verbose)
    {
        std::clog << "State: pc = " << data.pc
             << ", data_ptr = " << data.data_ptr
             << ", cell = " << (int)data.tape[data.data_ptr]
             << ", program[pc] = " << (int)(unsigned char)data.program[data.pc]
             << std::endl;
    }
}

