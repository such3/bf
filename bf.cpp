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
// TODO(CLEAN-5): add <cstdlib>; exit() currently arrives through another header.
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
//   NOTE: clog and cerr both write to stderr, but cerr is tied to cout (cout is
//   flushed before each cerr write) and clog is not. With the trace on clog,
//   the program's own output can therefore appear after the trace in a
//   terminal. See DEC-12.
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
//   DEC-1   '<' at cell 0 is currently ignored (open; see case '<')
//   DEC-3   every failure exits with status 1 (decided for now)
//   DEC-5   files with only whitespace or only comments are valid programs that
//           do nothing; a 0-byte file is an error (decided for now)
//   DEC-8   unknown flags and extra arguments are silently ignored (open)
//   DEC-9   characters that are not commands are skipped as comments (decided)
//   DEC-11  verbose is a global flag, not part of an options struct (decided
//           for now)

// -----------------------------------------------------------------------------
// Machine state
// -----------------------------------------------------------------------------

// Set while parsing the command line. Every trace function checks it itself,
// so callers cannot forget to.
// TODO(DEC-11): revisit when more flags exist (an options struct, Stage 1).
bool verboseOn = false;

// Everything that changes while a program runs, plus the program text.
// TODO(DEC-7): the text moves out of the machine state in Stage 2 (the IR).
struct MachineState
{
    int pc = 0;       // next instruction, an index into `program`
    int data_ptr = 0; // current cell, an index into `tape`
    string program;   // the Brainfuck source text
    unsigned char tape[30000] = {0};
};

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// Trace (verbose only, on clog): the instruction about to run.
void traceInstruction(char c)
{
    if (verboseOn)
        clog << "Encountered " << c << endl;
}

// Trace (verbose only, on clog): pc, data_ptr, the current cell and the
// character at pc. Called before the instruction runs, so it shows the state
// that instruction sees.
// NOTE: the verbose check is inside the function so no caller can forget it.
// It costs one load and one well-predicted branch per call; measure it in
// Stage 4 rather than guess.
// TODO(REF-5): after the loop, pc == program.length(), so program[pc] is the
//              string's terminating '\0' (legal for std::string, but not a
//              program character). The final dump prints 0 for it.
void dumpState(const MachineState &data)
{
    if (verboseOn)
        clog << "Dump State : Tape ptr : " << data.data_ptr
             << " String ptr : " << data.pc
             << " Tape content : " << (int)data.tape[data.data_ptr]
             << " String Content : " << (int)(unsigned char)data.program[data.pc]
             << endl;
}

// Checks that every bracket in `str` is matched and records each pair in `pos`
// and `rpos` (see "Jump tables" above).
//
// A stack handles nesting: each ']' pairs with the innermost open '['.
// An unmatched bracket prints an error on cerr and exits with status 1.
// Example of an invalid program: ++++[*-***-*>**>]]   (the last ']' has no '[')
//
// TODO(CLEAN-3): take `const string &str`; it is only read.
// TODO(CLEAN-4): compare with size_t instead of int in the loop.
// TODO(CLEAN-2): include the index of the offending bracket in the messages and
//                fix the spelling of "parentheses" in them.
// TODO(Q-9): decide whether a helper should be allowed to call exit().
void matchBrackets(string &str, unordered_map<int, int> &pos, unordered_map<int, int> &rpos)
{
    stack<int> stk;
    for (int i = 0; i < str.size(); i++)
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
            cerr << "[ERROR] Program must have balanced paranthesis . (Missing '[' )\n";
            exit(1);
            // TODO(CLEAN-12): unreachable after exit(); remove.
            return;
        }
    }

    // Anything left on the stack is a '[' that was never closed.
    if (!stk.empty())
    {
        cerr << "[ERROR] Program must have balanced paranthesis . (Missing ']' ) \n";
        exit(1);
        // TODO(CLEAN-12): unreachable after exit(); remove.
        return;
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
    // pc, data_ptr and tape start at their default values (0, 0, all zeros).
    MachineState machine;

    // ---- 1. Command line ----

    // The first argument is always the program path; flags may follow it.
    // A path is required.
    if (argc < 2)
    {
        cerr << "\n[ERROR] Command requires input file as a parameter !!\n";
        exit(1);
    }

    // Scan for flags. The loop starts at 2 because argv[0] is the program name
    // and argv[1] is the path. Flags are parsed before any code that prints
    // trace output, because those prints depend on verboseOn.
    // NOTE: argv[i] is a char*, and == on two char* compares addresses, not
    // text. The std::string copy makes the comparison compare the characters.
    // TODO(DEC-8): unknown flags and extra arguments are silently ignored.
    // TODO(T2-6): this dump runs before the flag check in the same iteration,
    //             so it only prints arguments that come after --verbose (never
    //             the flag itself), and it writes to cout, which is the
    //             program's own output. Print all arguments once, after the
    //             loop, on clog.
    for (int i = 2; i < argc; i++)
    {
        if (verboseOn)
            cout << "Arguement " << i << " : " << argv[i] << endl;
        string verbose = "--verbose";
        string arguement = argv[i];
        if (arguement == verbose)
        {
            verboseOn = true;
            clog << "\n Verbose is turned on !!" << endl;
        }
    }

    // Trace: startup information (verbose only).
    if (verboseOn)
        clog << " \n Number of  cmd line args : " << argc << endl;

    // TODO(CLEAN-7): this is an interpreter, not a compiler.
    if (verboseOn)
        clog << "This is a brainfuck compiler!" << endl;

    if (verboseOn)
        clog << "This is verbose flag : " << verboseOn << endl;

    // ---- Working variables ----

    // Jump tables, filled in by matchBrackets (see "Jump tables" above).
    unordered_map<int, int> positions;
    unordered_map<int, int> revpos;

    // The struct's default member initializers already set both to 0, so these
    // two assignments repeat that.
    // TODO(Q-13): decide whether they should stay.
    machine.data_ptr = 0;
    machine.pc = 0;

    // The line just read. The whole program accumulates in machine.program.
    string rdLine;

    string input_file_name;

    // ---- 2. Read the program ----

    // argv[1] is the path of the Brainfuck program (argc >= 2 was checked above).
    // TODO(T2-5): the error messages below do not say which path failed, and
    //             the "Number of cmd line args" trace is the only place the
    //             path is ever shown.
    input_file_name = argv[1];

    ifstream file(input_file_name);

    // The file could not be opened (missing, no permission, ...).
    if (!file.is_open())
    {
        cerr << "\n[ERROR] Error in opening file !! or File does not exist !! \n";
        exit(1);
    }

    // A 0-byte file is an error. A file with only whitespace or only comments
    // is a valid program that does nothing, and exits with status 0 (DEC-5).
    // TODO(DEC-5): confirm that a 0-byte file should stay an error.
    if (file.peek() == EOF)
    {
        cerr << "\n[ERROR] There are no contents in this file !! \n";
        exit(1);
    }

    // Read line by line and concatenate.
    // NOTE: rdLine is separate from machine.program because the final, failing
    // getline call clears the string it reads into; reading straight into
    // machine.program would leave it empty.
    // NOTE: concatenating loses line and column information (DEC-6, T-11).
    while (getline(file, rdLine))
    {
        if (verboseOn)
            clog << " The text in the file : " << rdLine << endl;

        machine.program.append(rdLine);

        if (verboseOn)
            clog << "[APPEND LOGS] After appending , we have this content : " << machine.program << endl;
    }

    // The program is in memory, so the file is no longer needed.
    // (The ifstream would also close itself at the end of main.)
    file.close();

    // ---- 3. Validate brackets and build the jump tables ----

    matchBrackets(machine.program, positions, revpos);

    // Trace: the jump tables (verbose only).
    if (verboseOn)
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

    if (verboseOn)
        clog << "We will process the bf pl here " << endl;

    // Rule for every case below: pc must advance or jump, otherwise the same
    // instruction repeats forever. data_ptr changes only on '>' and '<'.
    // '[' and ']' may also change pc (jumps).

    // Iteration counter, used only by the trace (it only advances with --verbose).
    int cnt = 0;

    // TODO(CLEAN-4): int vs size_t comparison.
    // TODO(T-4): add a step limit so a runaway loop cannot hang the program.
    while (machine.pc < machine.program.length())
    {
        // Stop when the tape pointer has run off the right end. Checked at the
        // top of the iteration, before any cell is accessed. Reports on cerr
        // and exits with status 1.
        if (machine.data_ptr == 30000)
        {
            cerr << "[OVERFLOW] Tape overflow !! You have crossed 30k cells . \n";
            exit(1);
        }

        // Trace: iteration header (verbose only).
        if (verboseOn)
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
            // Move the data pointer left.
            // TODO(DEC-1): at cell 0 the move is silently ignored and the
            //              program carries on with the wrong cell. Decide
            //              whether this should stop with an error instead, to
            //              match the right end.

            traceInstruction(machine.program[machine.pc]);

            dumpState(machine);

            if (machine.data_ptr == 0)
            {
                // TODO(CLEAN-13): this message names an internal variable; word
                //                 it for the user instead.
                if (verboseOn)
                    clog << "[DEBUG] Not moving data_ptr , or else data_ptr will go in negative " << endl;
                // NOTE: pc must advance even though data_ptr does not move.
                machine.pc++;
                break;
            }
            machine.data_ptr--;
            machine.pc++;

            break;

        case '.':
            // Output the current cell as one raw byte on stdout: no label and
            // no newline. The trace goes to clog.
            // NOTE: cout is not flushed here; it is flushed at exit (or when
            // its buffer fills). See "Output streams" for the ordering effect.

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

            if (verboseOn)
            {

                dumpState(machine);
                clog << " Skipping other instructions such as new lines and spaces and tabs " << endl;
            }

            machine.pc++;
        }
        // Trace: iteration footer (verbose only).
        if (verboseOn)
        {
            clog << "------------------------------------------------" << endl;
        }
    }

    // Trace: the state after the last instruction. The per-instruction dump
    // runs before each instruction, so the effect of the last one is only
    // visible here.
    // TODO(REF-5): this dump reads one past the end of the program text (see
    //              dumpState).
    if (verboseOn)
    {
        clog << "\n Final Dump state " << endl;
        dumpState(machine);
    }

    // Prints one newline on stdout after the program finishes.
    // NOTE: this is a byte the Brainfuck program did not output, so every run
    // writes at least "\n", even a program that prints nothing. It is not
    // written when the program exits through an error.
    // TODO(T2-7): decide whether the interpreter should add it.
    cout << endl;
    return 0;
}