#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <stack>
#include <cstdlib>

#include "cli/cli.h"
#include "loader/loader.h"
#include "machine/machine.h"
#include "parser/parser.h"
#include "exceptions/exceptions.h"
#include "tracers/tracers.h"
#include "engine/engine.h"
using namespace std;



int main(int argc, char *argv[])
{

    try
    {

        clog.tie(&cout);

        MachineState machine;
        Options options = parseArgs(argc, argv);
        if (options.verbose)
            clog << "Input " << options.inputFile << "\n";

        unordered_map<int, int> positions;
        unordered_map<int, int> revpos;

        loadSource(machine, options);

        validateBrackets(machine, positions, revpos);
        printMapTrace(options, positions, revpos);

        // if (options.verbose)
        // {
        //     clog << "Executing the program" << endl;
        // }

        // int cnt = 0;

        // unsigned long long steps = 0;
        // while (static_cast<size_t>(machine.pc) < machine.program.length())
        // {

        //     if (options.verbose)
        //     {
        //         clog << "Step Count : " << steps << endl;
        //     }
        //     if (steps >= options.maxSteps)
        //     {
        //         throw RuntimeError("[ERROR] Step limit of " + std::to_string(options.maxSteps) + " reached at index  " + std::to_string(machine.pc) + "\n");
        //     }
        //     steps++;

        //     if (machine.data_ptr == 30000)
        //     {
        //         throw RuntimeError("[OVERFLOW] Tape overflow !! You have crossed 30k cells . \n");
        //     }

        //     if (options.verbose)
        //     {
        //         clog << "------------------------------------------------" << endl;
        //         clog << "Iteration : " << cnt++ << endl;
        //         clog << " char : " << machine.program[machine.pc] << endl;
        //     }

        //     switch (machine.program[machine.pc])
        //     {
        //     case '+':

        //         traceInstruction(machine.program[machine.pc], options);
        //         dumpState(machine, options);

        //         machine.tape[machine.data_ptr]++;
        //         machine.pc++;

        //         break;

        //     case '-':

        //         traceInstruction(machine.program[machine.pc], options);
        //         dumpState(machine, options);

        //         machine.tape[machine.data_ptr]--;
        //         machine.pc++;

        //         break;

        //     case '>':

        //         traceInstruction(machine.program[machine.pc], options);
        //         dumpState(machine, options);

        //         machine.data_ptr++;
        //         machine.pc++;

        //         break;

        //     case '<':

        //         traceInstruction(machine.program[machine.pc], options);

        //         dumpState(machine, options);

        //         if (machine.data_ptr == 0)
        //         {
        //             throw RuntimeError("[ERROR] Access out of tape bounds  trying to access negative  bounds   ");
        //         }
        //         machine.data_ptr--;
        //         machine.pc++;

        //         break;

        //     case '.':

        //         traceInstruction(machine.program[machine.pc], options);
        //         dumpState(machine, options);

        //         cout << machine.tape[machine.data_ptr];

        //         machine.pc++;

        //         break;

        //     case ',':

        //         traceInstruction(machine.program[machine.pc], options);
        //         dumpState(machine, options);

        //         machine.pc++;

        //         break;

        //     case '[':

        //         traceInstruction(machine.program[machine.pc], options);

        //         if (machine.tape[machine.data_ptr] == 0)
        //         {
        //             dumpState(machine, options);

        //             machine.pc = positions[machine.pc] + 1;
        //         }
        //         else
        //         {
        //             dumpState(machine, options);

        //             machine.pc++;
        //         }

        //         break;

        //     case ']':

        //         traceInstruction(machine.program[machine.pc], options);
        //         if (machine.tape[machine.data_ptr] != 0)
        //         {
        //             dumpState(machine, options);

        //             machine.pc = revpos[machine.pc] + 1;
        //         }
        //         else
        //         {
        //             dumpState(machine, options);

        //             machine.pc++;
        //         }

        //         break;

        //     default:

        //         dumpState(machine, options);
        //         if (options.verbose)
        //         {
        //             clog << " Skipping other instructions such as new lines and spaces and tabs " << endl;
        //         }

        //         machine.pc++;
        //     }
        //     if (options.verbose)
        //     {
        //         clog << "------------------------------------------------" << endl;
        //     }
        // }

        // if (options.verbose)
        // {
        //     clog << "\n Final Dump state " << endl;
        //     dumpState(machine, options);
        // }
        executeCode(machine, options,positions,revpos);
        
    }
    catch (const RuntimeError &e)
    {
        std::cerr << "RUNTIME ERROR : " << e.what() << endl;
        exit(1);
    }
    catch (const CommandLineArgumentException &e)
    {
        std::cerr << "CLI Error: " << e.what() << '\n';
        std::cerr << "Usage: ./bf <input-file> [--verbose|-v] [--max-steps N]\n";
        exit(1);
    }
    catch (const InputSourceException &e)
    {
        cerr << "Input Source Error : " << e.what() << " \n";
        exit(1);
    }
    catch (const InvalidBracketsException &e)
    {
        cerr << "Invalid Bracket Error : " << e.what() << " \n";
        exit(1);
    }
    catch (const std::exception &e)
    {
        cerr << "[ERROR]: " << e.what() << "\n";
        exit(1);
    }

    return 0;
}