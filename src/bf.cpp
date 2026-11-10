// =============================================================================
// bf.cpp : a Brainfuck interpreter (learning project)
//
// Usage:  ./bf <program.bf> [--verbose]
// Build:  g++ -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined src/bf.cpp -o bf
//
// Status: Stage 0 (hardening). Open bugs and to-dos are in docs/TRACKER.md,
//         the plan is in docs/ROADMAP.md, code reviews are in docs/reviews/.
//
// Comment conventions in this file
//   NOTE:      why the code is the way it is right now
//   TODO(ID):  open work; ID is an entry in docs/TRACKER.md
// Fix history belongs in git commits and the tracker's Done log, not here.
// =============================================================================

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <stack>
#include <cstdlib> // exit()

// include user files and things
#include "cli/cli.h"
#include "loader/loader.h"
#include "machine/machine.h"
using namespace std;

// -----------------------------------------------------------------------------
// Design notes
// -----------------------------------------------------------------------------
//
// Machine model (fields of struct MachineState)
//   pc        index of the next instruction in the program text (program counter)
//   data_ptr  index of the current cell on the tape (data pointer)
//   tape      30000 unsigned 8-bit cells, all start at 0; arithmetic wraps
//   program   the Brainfuck source text, all lines concatenated
//
// Bracket jumps
//   Both '[' and ']' land one position AFTER the matching bracket.
//     '[' with a zero cell     : jump past the matching ']'
//     ']' with a nonzero cell  : jump back to just after the matching '['
//   Re-testing the cell at the bracket itself is unnecessary: the cell cannot
//   change between the test that triggered the jump and the landing point.
//   Some formulations land on the bracket and re-test; the result is the same.
//
// Jump tables
//   Two unordered_maps give two-way lookup, keyed by position in the program
//   text (not by tape position):
//     positions[i] = index of the ']' matching the '[' at i
//     revpos[j]    = index of the '[' matching the ']' at j
//   TODO(T-6): replace the two maps with one compact table.
//
// Output streams
//   cout  the program's own output only (the bytes written by '.')
//   cerr  errors
//   clog  the trace, printed only with --verbose
//   Redirect them separately:  ./bf prog.bf > out.txt 2> err.txt
//   NOTE: clog and cerr both write to stderr, so a redirect of stderr captures
//   both. cerr is tied to cout by default (cout is flushed before each cerr
//   write); clog is not, so main ties it explicitly. Without that tie, the
//   program's own output would appear after the trace in a terminal (DEC-12).
//
// Trace output
//   With --verbose every instruction prints a state dump. Two casts keep it
//   readable:
//     - A tape cell is an unsigned char, which a stream prints as a character.
//       Cast to int to print its numeric value.
//     - A program character is cast through unsigned char first, so bytes
//       above 127 do not turn into negative numbers.
//   TODO(Q-2): finish understanding the unsigned representation and printing
//              characters by their integer value.
//
// Policy decisions
//   DEC-1   '<' at cell 0 is an error, like the right end (decided; see BUG-8)
//   DEC-3   every failure exits with status 1 (decided for now)
//   DEC-5   files with only whitespace or only comments are valid programs that
//           do nothing; a 0-byte file is an error (decided for now)
//   DEC-8   unknown flags and extra arguments: in progress (see T2-8)
//   DEC-9   characters that are not commands are skipped as comments (decided)
//   DEC-11  verbose is a global flag, not part of an options struct (decided
//           for now)
//   DEC-12  the trace uses clog, tied to cout (decided)
//   DEC-13  the interpreter adds no output of its own, not even a final
//           newline (decided)

// -----------------------------------------------------------------------------
// Machine state
// -----------------------------------------------------------------------------

// Set while parsing the command line. Every trace function checks it itself,
// so callers cannot forget to.
// TODO(DEC-11): revisit when more flags exist (an options struct, Stage 1).
bool verbose_on = false;

// we will keep count of  some steps , so that we dont run on a infinite loop
const unsigned long long MAX_STEPS = 10000000; // 10 million

// Everything that changes while a program runs, plus the program text.
// NOTE: the default member initializers start pc and data_ptr at 0 and the
// tape at all zeros, so no separate initialization is needed.
// TODO(DEC-7): the text moves out of the machine state in Stage 2 (the IR).


// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// Trace (verbose only, on clog): the instruction about to run.
void traceInstruction(char c)
{
    if (verbose_on)
    {
        clog << "Encountered " << c << endl;
    }
}

// Trace (verbose only, on clog): pc, data_ptr, the current cell and the
// character at pc (as a number). Called before the instruction runs, so it
// shows the state that instruction sees.
// NOTE: the verbose check is inside the function so no caller can forget it.
// It costs one load and one well-predicted branch per call; measure it in
// Stage 4 rather than guess.
// NOTE (REF-5): after the loop pc == program.length(), so program[pc] is the
// string's terminating '\0'. That is defined for std::string, so the final dump
// prints 0 for "program[pc]", meaning "end of program".
// TODO(REF-5): must be fixed before Stage 2; reading at size() of a vector is
//              undefined behavior.
void dumpState(const MachineState &data)
{
    if (verbose_on)
    {
        clog << "State: pc = " << data.pc
             << ", data_ptr = " << data.data_ptr
             << ", cell = " << (int)data.tape[data.data_ptr]
             << ", program[pc] = " << (int)(unsigned char)data.program[data.pc]
             << endl;
    }
}

// Checks that every bracket in `str` is matched and records each pair in `pos`
// and `rpos` (see "Jump tables" above).
//
// A stack handles nesting: each ']' pairs with the innermost open '['.
// An unmatched bracket prints an error on cerr and exits with status 1.
// Example of an invalid program: ++++[*-***-*>**>]]   (the last ']' has no '[')
//
// NOTE: `str` is const because the function only reads it.
// NOTE: indices in the messages count characters in the concatenated program
// text, starting at 0 (line and column: T-11).
// NOTE: the stack and both maps hold int indices, so assigning the size_t loop
// index to them converts silently. That is safe for programs shorter than
// 2^31 characters.
// TODO(Q-9): decide whether a helper should be allowed to call exit().
void matchBrackets(const string &str, unordered_map<int, int> &pos, unordered_map<int, int> &rpos)
{
    stack<int> stk;
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
            cerr << "[ERROR] Unmatched ']' at index " << i << ": there is no '[' to match it.\n";
            exit(1);
        }
    }

    // Anything left on the stack is a '[' that was never closed. The top of the
    // stack is the innermost one.
    if (!stk.empty())
    {
        cerr << "[ERROR] Unmatched '[' at index " << stk.top() << ": it is never closed.\n";
        exit(1);
    }
}

// -----------------------------------------------------------------------------
// main
//   1. Read the command line
//   2. Open and read the program file
//   3. Validate brackets and build the jump tables
//   4. Execute
// -----------------------------------------------------------------------------

// argc counts the arguments including the program name:
//   ./bf prog.bf --verbose   ->   argc = 3
//   argv[0] = "./bf", argv[1] = "prog.bf", argv[2] = "--verbose"
int main(int argc, char *argv[])
{

    try
    {

        // Tie clog to cout so the program's pending output is flushed before every
        // trace write; the two streams then appear in the order things happened.
        clog.tie(&cout);

        MachineState machine;
        // Options
        // getting this , so we have the options
        Options options = parseArgs(argc, argv);
        if(options.verbose)
        clog << "Input " << options.inputFile << "\n";

       

        // // ---- 1. Command line ----

        // // The first argument is always the program path; flags may follow it.
        // // A path is required.
        // if (argc < 2)
        // {
        //     cerr << "\n[ERROR] Command requires input file as a parameter !!\n";
        //     exit(1);
        // }

        // Scan for flags. The loop starts at 2 because argv[0] is the program name
        // and argv[1] is the path. Flags are parsed before any code that prints
        // trace output, because those prints depend on verbose_on.
        // NOTE: argv[i] is a char*, and == on two char* compares addresses, not
        // text. The std::string copy makes the comparison compare the characters.
        // TODO(T2-8): the first clog line below is not guarded by verbose_on, so it
        //             prints once per extra argument even without --verbose. The
        //             else branch prints help for every argument that is not
        //             --verbose, but the program carries on and exits 0 (DEC-8).
        // for (int i = 2; i < argc; i++)
        // {
        //     string verbose = "--verbose";
        //     string argument = argv[i];

        //     if (argument == verbose)
        //     {
        //         verbose_on = true;
        //         clog << "\n Verbose is turned on !!" << endl;
        //     }
        //     else
        //     {
        //         cerr << " Help   " << endl;
        //         // the below line is just an explanation of the code , nothing else , so it for someone reviews the code
        //         // clog << "This dump starts after the file name !! So only the flags will be printed  (from index 2 )" << endl;
        //         cerr << "Use --verbose for verbose mode . " << endl;
        //     }

        // The flag was just processed above, so verbose_on is already up to
        // date for this argument.
        // if (verbose_on)
        // {
        //     clog << "Argument " << i << " : " << argv[i] << endl;
        // }

        // Trace: startup information (verbose only).
        // if (options.verbose)
        // {
            
        // }

        // ---- Working variables ----

        // Jump tables, filled in by matchBrackets (see "Jump tables" above).
        unordered_map<int, int> positions;
        unordered_map<int, int> revpos;

        // The line just read. The whole program accumulates in machine.program.
        // string rd_line;


        // we will just call the loader here 
        loadSource(machine,options);
        // string input_file_name;

        // ---- 2. Read the program ----

        // argv[1] is the path of the Brainfuck program (argc >= 2 was checked above).
        // TODO(T2-5): the error messages below do not say which path failed.
        // input_file_name = argv[1];

        // ifstream file(options.inputFile);

        // The file could not be opened (missing, no permission, ...).
        // if (!file.is_open())
        // {
        //     cerr << "\n[ERROR] Error in opening file !! or File does not exist !! \n";
        //     cerr << "\n[USAGE] the command should be something like this  : ./run <input-file>.txt --verbose (optional) \n";
        //     exit(1);
        // }

        // A 0-byte file is an error. A file with only whitespace or only comments
        // is a valid program that does nothing, and exits with status 0 (DEC-5).
        // TODO(DEC-5): confirm that a 0-byte file should stay an error.
        // if (file.peek() == EOF)
        // {
        //     cerr << "\n[ERROR] There are no contents in this file !! \n";
        //     exit(1);
        // }

        // Read line by line and concatenate.
        // NOTE: rd_line is separate from machine.program because the final, failing
        // getline call clears the string it reads into; reading straight into
        // machine.program would leave it empty.
        // NOTE: concatenating loses line and column information (DEC-6, T-11).

        // while (getline(file, rd_line))
        // {

        //     if (options.verbose)
        //     {
        //         clog << " The text in the file : " << rd_line << endl;
        //     }

        //     machine.program.append(rd_line);

        //     if (options.verbose)
        //     {
        //         clog << "[APPEND LOGS] After appending , we have this content : " << machine.program << endl;
        //     }
        // }

        // The program is in memory, so the file is no longer needed.
        // (The ifstream would also close itself at the end of main.)
        // file.close();

        // ---- 3. Validate brackets and build the jump tables ----

        matchBrackets(machine.program, positions, revpos);

        // Trace: the jump tables (verbose only).
        if (options.verbose)
        {
            clog << "Contents of Forward Map " << endl;
            for (const auto &pair : positions)
            {
                clog << pair.first << ": " << pair.second << "\n";
            }

            clog << "Contents of Backward Map " << endl;
            for (const auto &pair : revpos)
            {
                clog << pair.first << ": " << pair.second << "\n";
            }
        }

        // ---- 4. Execute ----

        if (options.verbose)
        {
            clog << "Executing the program" << endl;
        }

        // Rule for every case below: pc must advance or jump, otherwise the same
        // instruction repeats forever. data_ptr changes only on '>' and '<'.
        // '[' and ']' may also change pc (jumps).

        // Iteration counter, used only by the trace (it only advances with --verbose).
        int cnt = 0;

        // NOTE: pc is an int that never goes below 0 (it starts at 0 and only
        // advances or jumps forward to a matching bracket), so the cast to size_t
        // is safe and compares the two values as unsigned numbers.
        // TODO(T-4): add a step limit so a runaway loop cannot hang the program.

        // this is something to check for the infinite loop
        unsigned long long steps = 0;
        while (static_cast<size_t>(machine.pc) < machine.program.length())
        {

            // This is used to check infinite loops and to see how many steps the program is running.
            if (options.verbose)
            {
                clog << "Step Count : " << steps << endl;
            }
            if (steps >= MAX_STEPS)
            {
                cerr << "[ERROR] Step limit of " << MAX_STEPS << " reached at index  " << machine.pc << "\n";
                exit(1);
            }
            steps++;

            // Stop when the tape pointer has run off the right end. Checked at the
            // top of the iteration, before any cell is accessed. Reports on cerr
            // and exits with status 1.
            if (machine.data_ptr == 30000)
            {
                cerr << "[OVERFLOW] Tape overflow !! You have crossed 30k cells . \n";
                exit(1);
            }

            // Trace: iteration header (verbose only).
            if (options.verbose)
            {
                clog << "------------------------------------------------" << endl;
                clog << "Iteration : " << cnt++ << endl;
                clog << " char : " << machine.program[machine.pc] << endl;
            }

            switch (machine.program[machine.pc])
            {
            case '+':
                // Increment the current cell (255 wraps to 0).

                traceInstruction(machine.program[machine.pc]);
                dumpState(machine);

                machine.tape[machine.data_ptr]++;
                machine.pc++;

                break;

            case '-':
                // Decrement the current cell (0 wraps to 255).

                traceInstruction(machine.program[machine.pc]);
                dumpState(machine);

                machine.tape[machine.data_ptr]--;
                machine.pc++;

                break;

            case '>':
                // Move the data pointer right. Running past the last cell is
                // caught at the top of the next iteration.

                traceInstruction(machine.program[machine.pc]);
                dumpState(machine);

                machine.data_ptr++;
                machine.pc++;

                break;

            case '<':
                // Move the data pointer left. At cell 0 there is no cell to move
                // to, so the program stops with an error, like the right end (DEC-1).
                // TODO(BUG-8): the branch below calls exit(0), which reports
                //              success. Failures exit with status 1 (DEC-3).
                // TODO(CLEAN-13): the message names an internal variable; word it
                //                 for the user and say where it happened.

                traceInstruction(machine.program[machine.pc]);

                dumpState(machine);

                if (machine.data_ptr == 0)
                {
                    cerr << "[ERROR] Access out of tape bounds  trying to access negative  bounds   " << endl;
                    exit(1);
                }
                machine.data_ptr--;
                machine.pc++;

                break;

            case '.':
                // Output the current cell as one raw byte on stdout: no label and
                // no newline. The trace goes to clog.
                // NOTE: cout is not flushed here; it is flushed at exit, when its
                // buffer fills, or before a clog write (the tie in main).

                traceInstruction(machine.program[machine.pc]);
                dumpState(machine);

                cout << machine.tape[machine.data_ptr];

                machine.pc++;

                break;

            case ',':
                // Read one character into the current cell.
                // TODO(T-10): not implemented; the instruction is skipped.

                traceInstruction(machine.program[machine.pc]);
                dumpState(machine);

                machine.pc++;

                break;

            case '[':
                // Zero cell: skip the loop by jumping just past the matching ']'.
                // Nonzero cell: enter the loop body.
                // NOTE: the lookup key is pc (position in the program text),
                // not data_ptr (position on the tape).

                traceInstruction(machine.program[machine.pc]);

                if (machine.tape[machine.data_ptr] == 0)
                {
                    dumpState(machine);

                    machine.pc = positions[machine.pc] + 1;
                }
                else
                {
                    dumpState(machine);

                    machine.pc++;
                }

                break;

            case ']':
                // Nonzero cell: jump back to just after the matching '['.
                // Zero cell: leave the loop.
                // NOTE: the jump target is read from revpos and 1 is added to the
                // value; the stored entry is not modified.

                traceInstruction(machine.program[machine.pc]);
                if (machine.tape[machine.data_ptr] != 0)
                {
                    dumpState(machine);

                    machine.pc = revpos[machine.pc] + 1;
                }
                else
                {
                    dumpState(machine);

                    machine.pc++;
                }

                break;

            default:
                // Any other character is a comment and is skipped (DEC-9).
                // TODO(T-12): an optional strict mode, behind a flag, could reject
                //             non-command characters in a validation pass before
                //             execution starts.

                dumpState(machine);
                if (options.verbose)
                {
                    clog << " Skipping other instructions such as new lines and spaces and tabs " << endl;
                }

                machine.pc++;
            }
            // Trace: iteration footer (verbose only).
            if (options.verbose)
            {
                clog << "------------------------------------------------" << endl;
            }
        }

        // Trace: the state after the last instruction. The per-instruction dump
        // runs before each instruction, so the effect of the last one is only
        // visible here. "program[pc]" prints 0 here (end of program); see the NOTE
        // above dumpState.
        if (options.verbose)
        {
            clog << "\n Final Dump state " << endl;
            dumpState(machine);
        }
    }
    catch(const InputSourceException &e ){
        cerr << "Input Source Error : " << e.what() << " \n";
        exit(1);
    }
    catch (const std::exception &e)
    {
        cerr << "[ERROR]: " << e.what() << "\n";
        exit(1);
    }

    // NOTE (DEC-13): the interpreter adds no output of its own, not even a final
    // newline. The Brainfuck program owns every byte it writes; a program that
    // wants a newline prints the value 10.
    return 0;
}