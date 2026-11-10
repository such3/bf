#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

// class names which we have planned 
// exception class for brackets mismatch - Syntax Error

// class SyntaxError : public std:: 
class RuntimeError : public std::runtime_error {
    public :
        explicit RuntimeError(const std::string & message) :
            std::runtime_error(message) {}
};


#endif