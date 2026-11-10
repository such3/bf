#include "cli.h"
#include <iostream>
#include <string>
#include <stdexcept>

Options parseArgs(int argc, char* argv[]) {
    Options options;

    if (argc < 2) {
        throw CommandLineArgumentException("Missing input file");
    }

    // First argument is the input file.
    options.inputFile = argv[1];

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--verbose" || arg == "-v") {
            options.verbose = true;
        }

        else if (arg == "--max-steps") {
            if (i + 1 >= argc) {
                throw CommandLineArgumentException(
                    "--max-steps requires a value"
                );
            }

            std::string value = argv[++i];
            size_t pos = 0;

            try {
                options.maxSteps = std::stoll(value, &pos);

                if (pos != value.length()) {
                    throw CommandLineArgumentException(
                        "Invalid value for --max-steps: " + value
                    );
                }
            }
            catch (const std::invalid_argument&) {
                throw CommandLineArgumentException(
                    "Invalid number for --max-steps: " + value
                );
            }
            catch (const std::out_of_range&) {
                throw CommandLineArgumentException(
                    "--max-steps value is out of range"
                );
            }

            if (options.maxSteps < 0) {
                throw CommandLineArgumentException(
                    "--max-steps cannot be negative"
                );
            }
        }

        else {
            throw CommandLineArgumentException(
                "Unknown argument: " + arg
            );
        }
    }

    if (options.verbose) {
        std::clog << "Brainfuck interpreter\n";
        std::clog << "Verbose mode enabled\n";
        std::clog << "Input file: " << options.inputFile << '\n';
        std::clog << "Max steps: " << options.maxSteps << '\n';
    }

    return options;
}