#pragma once

#include <string>

const unsigned long long MAX_STEPS = 10000000;


struct MachineState
{
    int pc = 0;       // next instruction, an index into `program`
    int data_ptr = 0; // current cell, an index into `tape`
    std::string program;   // the Brainfuck source text
    unsigned char tape[30000] = {0};
};