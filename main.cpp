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

/* Specific changes for printing the values in the tape 
cout << "Dump State : Tape ptr : " << tpptr 
     << " String ptr : " << strptr 
     // 1. Cast to (int) so cell values like 0 or 65 print as numbers ("0", "65") instead of characters
     << " Tape content : " << (int)tape[tpptr] 
     // 2. Cast to (unsigned char) then (int) to safely print the character's numeric ASCII code
     << " String Content : " << (int)(unsigned char)rdTxt[strptr] 
     << endl;

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
        // I made an error here earlier.
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

    // Tape
    // Declarations of variables and Data structures
    // questions to answer about the tape
    // how will I handle if pointer goes less than 0 , e.g  -1 , - 2 or
    // overflows more than 30000 ?
    // should we throw some error ? or should we use absolute and find modulus and keep it ?
    // but using the modulus , it will just re-write content which will give wrong output
    // and by default I am storing 0 in the tape
    unsigned char tape[30000] = {0};

    // notes from google ai , in this case , how I am using the tape for printing the  contents wont work .
    // so , we need to find the way and why that exact thing needs to be studied
    // there is one more error , here , printing the array contents

    // using 2 maps
    // so that , we have one for searching wrt [
    // and one more to find the index wrt ]
    unordered_map<int, int> positions;
    unordered_map<int, int> revpos;

    // Not a real pointer, just an index that tracks where we are
    // on the 30000-cell tape.
    int tpptr = 0;  // tape pointer
    int strptr = 0; // string pointer
    string rdTxt;

    // Read the BF code from input.txt
    ifstream file("input.txt");
    while (getline(file, rdTxt))
    {
        cout << " The text in the file : " << rdTxt << endl;
    }
    computerPos(rdTxt, positions, revpos);

    // printing what we calculated
    // printing first map
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

    // Process the text
    cout << "We will process the bf pl here " << endl;

    /*
    Things to keep in mind:
    strptr advances after every character, while tpptr only changes
    on '>' and '<'. '[' and ']' may also change strptr (jumps).
    */

    // iteration counter
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
            tape[tpptr]++;
            strptr++;

            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            break;

        case '-':
            encWrt(rdTxt[strptr]);
            // Decrement the value at the current cell by one.
            tape[tpptr]--;
            strptr++;
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;

            break;

        case '>':
            encWrt(rdTxt[strptr]);
            // Move the tape index to the right.
            tpptr++;
            // error , i forgot to increment the str , so it kept on printing the same character
            strptr++;
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
            break;

        case '<':
            encWrt(rdTxt[strptr]);
            // Move the tape index to the left.
            tpptr--;
            strptr++;
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;

            break;

        case '.':
            encWrt(rdTxt[strptr]);
            // Print the current value of the cell.
            cout << " Content in the Cell : " << tape[tpptr];
            strptr++;
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;

            break;

        case ',':
            encWrt(rdTxt[strptr]);
            // Reads input.
            // Skipped for phase 1.
            strptr++;
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;

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

            if (tape[tpptr] == 0)
            {
                // Cell is zero: look up the matching bracket.
                // The map is keyed by positions in the code string,
                // so the lookup should use strptr.
                strptr = positions[tpptr]++;
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                     << endl;
            }
            else
            {
                // Cell is non-zero: just step into the loop body.
                strptr++;
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                     << endl;
            }

            encWrt(rdTxt[strptr]);
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;

            break;

        case ']':
            encWrt(rdTxt[strptr]);
            if (tape[tpptr] != 0)
            {
                // Cell is non-zero: jump back to the start of the loop.
                // This uses the reverse lookup.
                strptr = revpos[strptr]++;
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                     << endl;
            }
            else
            {
                strptr++;
                cout << "Dump State : Tape ptr : " << tpptr
                     << " String ptr : " << strptr
                     << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                     << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                     << endl;
            }

            break;

        default:
            // if space or new line , we will skip it
            strptr++;
            cout << " Not a valid Brain Fuck instruction " << endl;
            cout << "Dump State : Tape ptr : " << tpptr
                 << " String ptr : " << strptr
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to number
                 << " String Content : " << (int)(unsigned char)rdTxt[strptr] // Cast to number safely
                 << endl;
        }
        cout << "------------------------------------------------" << endl;
    }

    return 0;
}