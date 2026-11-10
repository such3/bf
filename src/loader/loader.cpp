#include "loader.h"

#include <iostream>
#include <fstream>
#include <string>

void loadSource(MachineState &machine, Options &options)
{

    std::string rd_line;
    // avoid unnecessary copying of the filename
    const std::string &filename = options.inputFile;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        throw InputSourceException(
            "Unable to open source file: " + filename + " . File Does not exist or is corrupted !");
    }

    if (file.peek() == EOF)
    {
        throw InputSourceException(
            "The input file is empty.");
    }

    while (std::getline(file, rd_line))
    {

        if (options.verbose)
        {
            std::clog << "[SOURCE] " << rd_line << '\n';
        }

        machine.program.append(rd_line);
    }

    if (file.bad())
    {
        throw InputSourceException(
            "Error while reading source file: " + filename);
    }
}