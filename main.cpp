#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string> // solved an error 
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

/*
Reflections on the bracket jumps:
- Jumping to just after the matching bracket is correct for both '[' and ']'.
- Re-testing the condition after the jump is unnecessary: the cell has not
  changed between the test that triggered the jump and the landing point.
- The textbook rule says ']' jumps back to the matching '[', which then
  re-tests the cell. Landing just after '[' is an equivalent shortcut that
  gives the same tape and output.
*/

void encWrt(char c)
{
    cout << "Encountered " << c << endl;
}

// this is a function which I will use it for printing all the traces 
// questions which i need to ask 
// so how do  i access the variables 
// learning part 
//  Dumping problem solution - what is the solution 
// 

void dumpState(){}

// note : how to handle unbalanced brackets ?
// how will  i identify them ?
// que2 : now , how do we identify the same in nested loops ?
// example : ++++[*-***-*>**>]]
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
            continue;
        }
        // Note: this part had a bug in an earlier version.
        if (str[i] == ']')
        {
            if (!stk.empty())
            {
                // Two maps give us bidirectional lookup.
                pos[stk.top()] = i;
                rpos[i] = stk.top();
                stk.pop();
                continue;
                // fixed this bug , added continue , earlier it was just hitting it on every character  bcz i did not continue the loop , or come out of that iteration , I applied the same above in another if block too 
            }

            // we reach here where we encounter a ']' , but we dont have a correponding opening bracket 
            cout << "[ERROR] Program must have balanced paranthesis . (Missing '[' )\n";
            // learn more about exit codes , what is the significance of them ?
            exit(0);
            return;
        }
    }
    if(!stk.empty()){
        // if we have somemthing on stack , that is if we have a just [ and we have not ] to pop it out , then we enter here .
        cout << "[ERROR] Program must have balanced paranthesis . (Missing ']' ) \n";
        // check for exit codes here too
        exit(0);
        return;
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
    string rdLine , rdTxt;

    // Read the BF code from input.txt.
    // TODO: Make the file handling safer, so that we don't fail when we hit
    // EOF or similar, and decide how to handle code that spans multiple
    // lines. For now, only a single line of code is supported.
    ifstream file("input.txt");
    while (getline(file, rdLine))
    {
        cout << " The text in the file : " << rdLine << endl;
        // Once we extract a line , we will  simply append it 
        rdTxt.append(rdLine);
        cout << "[APPEND LOGS] After appending , we have this content : " << rdTxt << endl;
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
    while (strptr < rdTxt.length() )
    {
        // check for thr upper bound 
        // report an error and exit from the loop 
        // fox this bug : I was checking the tape overflow with cnt , it should be tpptr
        if(tpptr == 30000){
            cout << "[OVERFLOW] Tape overflow !! You have crossed 30k cells . \n";
            break;
        }

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
                 // I was casting it to a char here , but I belive I did it because I was seeing integers and I wanted to see the chars
                 << " Tape content : " << (int)tape[tpptr]                    // Cast to char
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
            // fix :  if we are already at tpptr = 0  , then dont update it 
            if(tpptr == 0){
                cout << "[DEBUG] Not moving tpptr , or else tpptr will go in negative " << endl;
                break;
            }
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
                // Cell is zero: look up the matching bracket and continue
                // one position after it.
                // Note: I originally used tpptr as the key. The map is keyed
                // by positions in the code string, so the lookup must use
                // strptr, because it is the string position that moves here.
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
                // Cell is non-zero: jump back to the start of the loop,
                // using the reverse lookup, and continue one position after
                // the matching '['.
                // Note: the earlier version used a post-increment on the map
                // entry (revpos[strptr]++), which modified the map instead of
                // only reading it. Now the value is read from the map and 1 is
                // added to it separately, so the map stays unchanged.
                strptr = revpos[strptr] + 1;
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