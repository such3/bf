// =============================================================================
// bf.cpp : a Brainfuck interpreter (learning project)
//
// Usage:  ./bf <program.bf>
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
// Machine model
//   strptr  index of the next instruction in the program text (program counter)
//   tpptr   index of the current cell on the tape (data pointer)
//   tape    30000 unsigned 8-bit cells, all start at 0; arithmetic wraps
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
// Trace output
//   Every instruction prints a state dump. Two casts keep it readable:
//     - A tape cell is an unsigned char, which cout prints as a character.
//       Cast to int to print its numeric value.
//     - A program character is cast through unsigned char first, so bytes
//       above 127 do not turn into negative numbers.
//   TODO(T-1): the dump is copy-pasted into every case; move it into dumpState().
//   TODO(T-2): put the trace behind a verbose flag and keep it separate from
//              the program's own output.
//   TODO(Q-2): finish understanding the unsigned representation and printing
//              characters by their integer value.
//
// Open policy questions
//   DEC-1  what '<' at cell 0 should do (currently ignored, see case '<')
//   DEC-3  exit codes (currently 1 for every failure)
//   DEC-5  what counts as an empty program

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// Trace: announces the instruction being executed.
void encWrt(char c)
{
    cout << "Encountered " << c << endl;
}

// Trace: prints the machine state for the current instruction.
// TODO(T-1): implement. Plan: put tape, tpptr and strptr (and decide about the
//            program text, DEC-7) in one struct and take it by const reference.
void dumpState() {}

// Checks that every bracket in `str` is matched and records each pair in
// `pos` and `rpos` (see "Jump tables" above).
//
// A stack handles nesting: each ']' pairs with the innermost open '['.
// An unmatched bracket prints an error and exits with status 1.
// Example of an invalid program: ++++[*-***-*>**>]]   (the last ']' has no '[')
//
// TODO(CLEAN-3): rename to computePos; take `const string &str`.
// TODO(CLEAN-4): compare with size_t instead of int in the loop.
// TODO(CLEAN-2): include the index of the offending bracket in the messages and
//                fix the spelling of "parentheses".
// TODO(BUG-2): error messages go to cout; they belong on cerr.
// TODO(Q-9): decide whether a helper should be allowed to call exit().
void computerPos(string &str, unordered_map<int, int> &pos, unordered_map<int, int> &rpos)
{
    stack<int> stk;
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == '[')
        {
            stk.push(i);
            // TODO(CLEAN-1): check whether this continue is needed.
            continue;
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

            // A ']' with no open '['.
            cout << "[ERROR] Program must have balanced paranthesis . (Missing '[' )\n";
            // Exit status 0 means success, so a failure must exit nonzero.
            exit(1);
            // TODO: unreachable after exit(); remove.
            return;
        }
    }

    // Anything left on the stack is a '[' that was never closed.
    if (!stk.empty())
    {
        cout << "[ERROR] Program must have balanced paranthesis . (Missing ']' ) \n";
        exit(1);
        // TODO: unreachable after exit(); remove.
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
//   ./bf prog.bf   ->   argc = 2, argv[0] = "./bf", argv[1] = "prog.bf"
int main(int argc, char *argv[])
{
    // TODO(CLEAN-7): this is an interpreter, not a compiler.
    cout << "This is a brainfuck compiler!" << endl;

    // ---- Machine state ----

    // The tape: 30000 cells, zero-initialised, 8-bit unsigned so arithmetic wraps.
    // Pointer policy: past the right end stops the program with [OVERFLOW];
    // at the left end the move is ignored (DEC-1, DEC-2).
    unsigned char tape[30000] = {0};

    // Jump tables, filled in by computerPos (see "Jump tables" above).
    unordered_map<int, int> positions;
    unordered_map<int, int> revpos;

    // Indices, not real pointers.
    int tpptr = 0;  // current cell on the tape
    int strptr = 0; // next instruction in the program text

    // rdLine: the line just read. rdTxt: the whole program, lines concatenated.
    string rdLine, rdTxt;

    // ---- 1. Command line ----

    string input_file_name;

    // Debug: print the arguments.
    // TODO(T-2): move behind the verbose flag.
    cout << " \n Number of  cmd line args : " << argc << endl;

    for (int i = 0; i < argc; i++)
    {
        cout << "Arguement " << i << " : " << argv[i] << endl;
    }

    // argv[1] is the path of the Brainfuck program.
    // TODO(BUG-5): argc is never checked. Running ./bf with no arguments reads a
    //              null pointer here. Print a usage message and exit with
    //              failure. Also decide what to do with extra arguments (DEC-8).
    input_file_name = argv[1];

    // ---- 2. Read the program ----

    ifstream file(input_file_name);

    // The file could not be opened (missing, no permission, ...).
    if (!file.is_open())
    {
        cout << "\n[ERROR] Error in opening file !! or File does not exist !! \n";
        exit(1);
    }

    // The file is empty.
    // TODO(BUG-6): peek() only catches a 0-byte file. A file with only
    //              whitespace, or with no Brainfuck commands, passes this check.
    //              Decide what counts as empty (DEC-5) and check after reading.
    if (file.peek() == EOF)
    {
        cout << "\n[ERROR] There are no contents in this file !! \n";
        exit(1);
    }

    // Read line by line and concatenate.
    // NOTE: rdLine is separate from rdTxt because the final, failing getline
    // call clears the string it reads into; reading straight into rdTxt would
    // leave it empty.
    // NOTE: concatenating loses line and column information (DEC-6, T-11).
    while (getline(file, rdLine))
    {
        cout << " The text in the file : " << rdLine << endl;
        rdTxt.append(rdLine);
        cout << "[APPEND LOGS] After appending , we have this content : " << rdTxt << endl;
    }

    // The program is in memory, so the file is no longer needed.
    // (The ifstream would also close itself at the end of main.)
    file.close();

    // ---- 3. Validate brackets and build the jump tables ----

    computerPos(rdTxt, positions, revpos);

    // Debug: print the jump tables.
    // TODO(T-2): move behind the verbose flag.
    cout << "Contents of Forward Map " << endl;
    for (const auto &pair : positions)
    {
        cout << pair.first << ": " << pair.second << "\n";
    }

    cout << "Contents of Backward Map " << endl;
    for (const auto &pair : revpos)
    {
        cout << pair.first << ": " << pair.second << "\n";
    }

    // ---- 4. Execute ----

    cout << "We will process the bf pl here " << endl;

    // Rule for every case below: strptr must advance or jump, otherwise the
    // same instruction repeats forever. tpptr changes only on '>' and '<'.
    // '[' and ']' may also change strptr (jumps).

    // Iteration counter, for the trace only.
    int cnt = 0;

    // TODO(CLEAN-4): int vs size_t comparison.
    // TODO(T-4): add a step limit so a runaway loop cannot hang the program.
    while (strptr < rdTxt.length())
    {
        // Stop when the tape pointer has run off the right end. Checked at the
        // top of the iteration, before any cell is accessed. This break is
        // outside the switch, so it leaves the while loop.
        // TODO(BUG-2): the program then ends with `return 0` (success) and
        //              reports on cout. It should fail with a nonzero status
        //              and report on cerr.
        if (tpptr == 30000)
        {
            cout << "[OVERFLOW] Tape overflow !! You have crossed 30k cells . \n";
            break;
        }

        cout << "------------------------------------------------" << endl;
        cout << "Iteration : " << cnt++ << endl;

        cout << " char : " << rdTxt[strptr] << endl;
        switch (rdTxt[strptr])
        {
        case '+':
            // Increment the current cell (255 wraps to 0).
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                 << endl;
            tape[tpptr]++;
            strptr++;

            break;

        case '-':
            // Decrement the current cell (0 wraps to 255).
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                 << endl;
            tape[tpptr]--;
            strptr++;

            break;

        case '>':
            // Move the data pointer right. Running past the last cell is
            // caught at the top of the next iteration.
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                 << endl;
            tpptr++;
            strptr++;

            break;

        case '<':
            // Move the data pointer left.
            // TODO(DEC-1): at cell 0 the move is silently ignored and the
            //              program carries on with the wrong cell. Decide
            //              whether this should stop with an error instead, to
            //              match the right end.
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                 << endl;
            if (tpptr == 0)
            {
                cout << "[DEBUG] Not moving tpptr , or else tpptr will go in negative " << endl;
                // NOTE: strptr must advance even though tpptr does not move.
                strptr++;
                break;
            }
            tpptr--;
            strptr++;

            break;

        case '.':
            // Output the current cell.
            // TODO(BUG-4): the label and the trace are mixed into the program's
            //              output. A Brainfuck program should print only the
            //              character itself.
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                 << endl;
            cout << " Content in the Cell : " << tape[tpptr] << endl;
            strptr++;

            break;

        case ',':
            // Read one character into the current cell.
            // TODO(T-10): not implemented; the instruction is skipped.
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                 << endl;
            strptr++;

            break;

        case '[':
            // Zero cell: skip the loop by jumping just past the matching ']'.
            // Nonzero cell: enter the loop body.
            // NOTE: the lookup key is strptr (position in the program text),
            // not tpptr (position on the tape).
            encWrt(rdTxt[strptr]);

            if (tape[tpptr] == 0)
            {
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                     << endl;
                strptr = positions[strptr] + 1;
            }
            else
            {
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                     << endl;
                strptr++;
            }

            break;

        case ']':
            // Nonzero cell: jump back to just after the matching '['.
            // Zero cell: leave the loop.
            // NOTE: the jump target is read from revpos and 1 is added to the
            // value; the stored entry is not modified.
            encWrt(rdTxt[strptr]);
            if (tape[tpptr] != 0)
            {
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                     << endl;
                strptr = revpos[strptr] + 1;
            }
            else
            {
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                     << endl;
                strptr++;
            }

            break;

        default:
            // Any other character is a comment and is skipped.
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr]
                 << endl;
            strptr++;
            cout << " Skipping other instructions such as new lines and spaces and tabs " << endl;
        }
        cout << "------------------------------------------------" << endl;
    }

    return 0;
}