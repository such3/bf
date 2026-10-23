#include <iostream>
#include <fstream>
#include <unordered_map>
#include <stack>
using namespace std;

/*
Problems encountered:
- std::unordered_map has no built-in two-way lookup, so for now I use
  two maps: one for '[' -> ']' and one for ']' -> '['.
*/

/*
Specific changes for printing the values on the tape:

cout << "Dump State : Tape ptr : " << tpptr
     << " String ptr : " << strptr
     // 1. Cast to (int) so cell values like 0 or 65 print as numbers
     //    ("0", "65") instead of characters.
     << " Tape content : " << (int)tape[tpptr]
     // 2. Cast to (unsigned char) and then to (int) to safely print the
     //    character's numeric ASCII code.
     << " String Content : " << (int)(unsigned char)rdTxt[strptr]
     << endl;
*/

/*
TODO: Find a way to print characters using their integer representation,
and understand the unsigned representation.
*/

void encWrt(char c)
{
    cout << "Encountered " << c << endl;
}

// void dumpState()

void computerPos(string &str, unordered_map<int, int> &pos, unordered_map<int, int> &rpos)
{
    // Compute the matching position for each bracket so that
    // nested loops are handled correctly.
    stack<int> stk;
    for (int i = 0; i < str.size(); i++)
    {
        if (str[i] == '[')
        {
            stk.push(i);
        }
        // Note: I made an error here earlier.
        if (str[i] == ']')
        {
            if (!stk.empty())
            {
                // Two maps give us bidirectional lookup.
                pos[stk.top()] = i;
                rpos[i] = stk.top();
                stk.pop();
            }
        }
    }
}

// void printCell(int &ptr, int &tape)
// {
//     cout << "Contents of the Cell : " << ptr(tape;
// }

int main()
{
    cout << "This is a brainfuck compiler!" << endl;

    // ----- Tape -----
    // Declarations of variables and data structures.
    //
    // Questions to answer about the tape:
    // - How do I handle the pointer going below 0 (e.g. -1, -2) or
    //   past 30000?
    // - Should I throw an error, or take the absolute value / modulus
    //   and wrap around?
    // - Wrapping with a modulus would just overwrite existing content,
    //   which gives wrong output.
    // - By default, every cell on the tape is initialised to 0.
    unsigned char tape[30000] = {0};

    // Note from Google AI: with my current approach, printing the tape
    // contents won't work. I need to find out why and study that exact
    // behaviour.
    // There is one more error here: printing the array contents.

    // Using two maps:
    // one to look up the match for '[' and one to look up the match for ']'.
    unordered_map<int, int> positions;
    unordered_map<int, int> revpos;

    // Not a real pointer, just an index that tracks where we are
    // on the 30000-cell tape.
    int tpptr = 0;  // tape pointer
    int strptr = 0; // string pointer
    string rdTxt;

    // Read the BF code from input.txt.
    // need improvements and safety measure  to be taken in file handling , so that we dont fail when we hit EOF or smth and also how to handle the file when we have multiple lines of code ?
    ifstream file("input.txt");
    while (getline(file, rdTxt))
    {
        cout << " The text in the file : " << rdTxt << endl;
    }
    computerPos(rdTxt, positions, revpos);

    // Print what we calculated.
    // First map:
    cout << "Contents of Forward Map " << endl;
    for (const auto &pair : positions)
    {
        cout << pair.first << ": " << pair.second << "\n";
    }

    // Second map:
    cout << "Contents of Backward Map " << endl;
    for (const auto &pair : revpos)
    {
        cout << pair.first << ": " << pair.second << "\n";
    }

    // Process the text.
    cout << "We will process the bf pl here " << endl;

    /*
    Things to keep in mind:
    strptr advances after every character, while tpptr only changes
    on '>' and '<'. '[' and ']' may also change strptr (jumps).
    */

    // Iteration counter.
    int cnt = 0;
    while (strptr < rdTxt.length() && tpptr < 30000)
    {
        cout << "------------------------------------------------" << endl;
        cout << "Iteration : " << cnt++ << endl;

        cout << " char : " << rdTxt[strptr] << endl;
        switch (rdTxt[strptr])
        {
        case '+':
            encWrt(rdTxt[strptr]);
            // Increment the value at the current cell by one.
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            tape[tpptr]++;
            strptr++;

            break;

        case '-':
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            // Decrement the value at the current cell by one.
            tape[tpptr]--;
            strptr++;

            break;

        case '>':
            encWrt(rdTxt[strptr]);
            // Move the tape index to the right.
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (char)tape[tpptr]                   // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            tpptr++;
            // Earlier error: I forgot to increment strptr, so the same
            // character was printed repeatedly.
            strptr++;

            break;

        case '<':
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            // Move the tape index to the left.
            tpptr--;
            strptr++;

            break;

        case '.':
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            // Print the current value of the cell.
            cout << " Content in the Cell : " << tape[tpptr] << endl;
            strptr++;

            break;

        case ',':
            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            // Reads input.
            // Skipped for phase 1.
            strptr++;

            break;

        case '[':
            // Rule:
            // On '[', check whether the current cell is zero.
            // If it is zero, jump to the matching ']' and continue after it.
            /*
            My thinking:
            Take code like +++++++[>++++++++++<-]>.
            When I encounter '[', I check whether the current cell is 0.
            If it is not, I continue with the next character.
            If it is 0, I need to move the pointer past the matching ']'.
            Can we do this by looking the position up in the map?
            */
            encWrt(rdTxt[strptr]);

            if (tape[tpptr] == 0)
            {
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                     << endl;
                // Cell is zero: look up the matching bracket.
                // The map is keyed by positions in the code string,
                // so the lookup should use strptr.

                // i was updating this as tpptr , so i changed it to strptr  , because we need it to move across the string , if something needs to be moved
                strptr = positions[strptr] + 1;
            }
            else
            {
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                     << endl;
                // Cell is non-zero: just step into the loop body.
                strptr++;
            }

            break;

        case ']':
            encWrt(rdTxt[strptr]);
            if (tape[tpptr] != 0)
            {
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                     << endl;
                // Cell is non-zero: jump back to the start of the loop.
                // This uses the reverse lookup.
                strptr = revpos[strptr] + 1;

                // changed the incrementation seperately , so that it uses the value and then increment
                // this is the error ? I was updating something in the map , I would need to update the value of the variable in the first place ;
            }
            else
            {
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                     << endl;
                strptr++;
            }

            break;

        default:
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            // Spaces, newlines, tabs and other characters are skipped.
            strptr++;
            cout << " Skipping other instructions such as new lines and spaces and tabs " << endl;
        }
        cout << "------------------------------------------------" << endl;
    }

    return 0;
}