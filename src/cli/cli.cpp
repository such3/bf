#include "cli.h"

#include <iostream>
#include  <stdexcept>

Options parseArgs(int argc , char* argv[]){
    Options options;

    if(argc < 2){
        throw std::runtime_error("Missing input File");
    }

    // Decision , as of now the 1st arguement will be the input file 
    options.inputFile = argv[1];

    for (int i = 2; i < argc;i++){
        std::string arg = argv[i];

        if(arg == "--verbose" || arg == "-v"){
            options.verbose = true;
            std::clog << "This is a Brainfuck interpreter! \n";
            std::clog << "Verbose mode enabled \n";
            std::clog << "\nNumber of cmd line args : " << argc << std::endl;
        }else {
                // cerr << " Help   " << endl;
                // the below line is just an explanation of the code , nothing else , so it for someone reviews the code
                // clog << "This dump starts after the file name !! So only the flags will be printed  (from index 2 )" << endl;
                std::cerr << "[USAGE] --verbose or -v for verbose mode . " << std::endl;
        }
        if (options.verbose)
            {
                std::clog << "Argument " << i << " : " << argv[i] << std::endl;
            }
    }
    return options;

}
