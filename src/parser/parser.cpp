#include "parser.h"
#include <string>
#include <stack>

void validateBrackets(MachineState &machine , std::unordered_map<int, int> &pos, std::unordered_map<int, int> &rpos)
{
    const std::string &str = machine.program; 
    std::stack<int> stk;
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == '[')
        {
            stk.push(i);
            // NOTE: no `continue` is needed here. The next `if` tests for ']',
            // which is false for a '[', so nothing else happens this iteration.
        }
        if (str[i] == ']')
        {
            if (!stk.empty())
            {
                pos[stk.top()] = i;
                rpos[i] = stk.top();
                stk.pop();
                // NOTE: required. Without it, a matched ']' would fall through
                // to the unmatched-']' error below.
                continue;
            }

            // A ']' with no open '['. Exit status 0 means success, so a
            // failure must exit nonzero.
            throw InvalidBracketsException("[ERROR] Unmatched ']' at index " + std::to_string(i) + ": there is no '[' to match it.\n");
            // exit(1);
        }
    }

    // Anything left on the stack is a '[' that was never closed. The top of the
    // stack is the innermost one.
    if (!stk.empty())
    {
        throw InvalidBracketsException("[ERROR] Unmatched '[' at index " + std::to_string(stk.top()) + ": it is never closed.\n");
        // exit(1);
    }

}


void printMapTrace(Options &options , std::unordered_map<int, int> &pos,std::unordered_map<int, int> &rpos){
    if(options.verbose){
std::clog << "Contents of Forward Map " << std::endl;
            for (const auto &pair : pos)
            {
                std::clog << pair.first << ": " << pair.second << "\n";
            }

            std::clog << "Contents of Backward Map " << std::endl;
            for (const auto &pair : rpos)
            {
                std::clog << pair.first << ": " << pair.second << "\n";
            }
    }
    
}