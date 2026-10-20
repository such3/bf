#include <iostream>
#include <fstream>
using namespace std;

void encWrt(char c){
    cout << "Encountered " << c << endl;
}

int main(){
    cout << "This is a brainfuck compiler!" << endl;
    
    // First try 
    unsigned char tape[30000] = {0};
    int ptr;
    string rdTxt;

    // read the bg code  from input.txt
    ifstream file("input.txt");
    while(getline(file,rdTxt)){
        cout << " The text in the file : " << rdTxt << endl;
    }

    // processing the text
    cout << "We will process the bf pl here " << endl;

    for(char c : rdTxt){
        cout << " char : " << c << endl;
        switch (c)
        {
        case '+':
            encWrt(c);
            // + logic goes here 
            break;

        case '-':
            encWrt(c);
            // - logic goes here
            break;
        case '>':
            encWrt(c);
            break;
        case '<':
            encWrt(c);
            break;
        case '.':
            encWrt(c);
            break;
        case ',':
            encWrt(c);
            break;
        case '[':
            encWrt(c);
            break;
        case ']':
            encWrt(c);
            break;
        default :
            cout << " Not a valid Brain Fuck instruction " << endl;
        }
    }

    return 0;
}