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