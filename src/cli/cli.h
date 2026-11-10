#pragma once 


#include <string>
#include <vector>


struct Options {
    // major  thins we will have here 
    // file name which contains the input 
    // verbose if on or off
    // max steps will take 2 arguements 
    // some additions 
    std::string inputFile;
    bool verbose = false;

};


// here we just define the function 
Options parseArgs(int argc , char * argv[]);