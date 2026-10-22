Here's the review. I'll point at where things go wrong and give you questions to chase, not fixes.

## What you got right

- Precomputing the bracket matches with a stack is the standard, correct approach, and `computerPos` does what it should. Don't touch it.
- `unsigned char` for the tape is a good choice, and you've clearly thought about why.
- The `strptr` / `tpptr` split and the dump-state logging show good debugging instincts. That habit will serve you well.

## Where the mistakes are, most serious first

**1. The jumps in `[` and `]`: this is your main bug.**
Look at `strptr = positions[tpptr]++;` and `strptr = revpos[strptr]++;`. Each line has more than one problem, so count them.

- Read your own comment above the `[` jump and compare it to the key you're using for the lookup.
- What does `x++` return, and what does it also do to `x`? Here `x` is something living inside a map. Think about what happens to that entry the second, third and fourth time the line runs.
- After the jump, which character does `strptr` point at? Will the main loop then execute it or skip it? Does that matter for `[`? For `]`?

Exercise: hand-trace `+++++++[>++++++++++<-]>.` Every time `]` is hit, write down `revpos[21]` (the `]` is at index 21). You'll see something odd within a few iterations. Also notice why the first iteration *looks* fine. A bug that works by coincidence is the most dangerous kind.

**2. Reading the file.**
`while (getline(file, rdTxt))` runs once per line. Questions:

- What is in `rdTxt` after the loop if the file has 3 lines?
- What does `getline` do to the string when it fails at end of file? Does it matter whether the file ends with a newline? Test this by printing `rdTxt` right after the loop, with and without a trailing newline in `input.txt`.

**3. Tape bounds.**
Your own comment asks this question, and the code doesn't answer it yet. Look at the `while` condition.

- Which direction of overflow does it guard against, and which does it ignore?
- What happens at `tape[-1]`? In C++, is that an error, a crash, or something worse?
- Compare `strptr < rdTxt.length()` too. Why is comparing an `int` with a `.length()` result worth a second look? Your compiler warns about this if you ask it to (see the tools section below).

**4. The `>` case.**
Put the `>` dump next to the `<` dump and compare them character by character. This is the "printing the array contents" error your comment mentions. Which cast differs, and what does `cout` do with a `char` holding 0?

**5. The `.` case.**
Compare what your program prints with what a real BF `.` should output.

- Is the label you print part of the program's output or part of your debugging?
- Is there a newline or flush before the next debug line starts?
- Once programs get bigger, how would you tell program output apart from debug output? Think about whether the debug printing should always be on.

**6. The `default` case.**
The message says "Not a valid instruction" for a space or newline. Check what the BF spec says about characters outside the eight commands. Is the message accurate, and is it useful noise or just noise?

## Design observations (not bugs)

- The dump block is copy-pasted about twelve times. You already sketched `dumpState()` in a comment. Ask yourself why each case needs its own copy, and what you'd have to edit if you wanted to add one more field.
- Unmatched brackets are silently ignored in `computerPos`. Is that acceptable for a tool you want others to trust?
- It's currently an interpreter, not a compiler: it executes the text directly. That's a perfectly good first step. If you want to go further later, the natural next ideas are an intermediate form and an optimization pass, such as collapsing runs of `+++++`.

## How much is left

Roughly, and this depends on how fast you debug:

- **Core interpreter (about 75% already there):** fix the bracket jumps, the file read, the tape bounds and the `>` cast. This is the part that makes it correct, probably one focused sitting.
- **Making it usable (not started):** implement `,`, make the debug output optional, and report errors like unmatched brackets or tape overflow clearly.
- **Proving it works (not started):** run a handful of known programs, including nested loops and a Hello World, and compare against known-good output.
- **Stretch:** the `dumpState` cleanup, and then the compiler ideas above.

## Tools that will find these for you

- Compile with `g++ -g -Wall -Wextra -fsanitize=address,undefined bf.cpp`. The warnings and the sanitizer will catch several of the bugs above on their own.
- Write tiny BF programs, one idea each (`+.`, `>+<`, `+[-]`), and predict the output *before* you run them. When the prediction fails, that's the bug.
- When a jump misbehaves, print both maps again *after* a few loop iterations, not only at startup.

Tell me which bug you want to tackle first and what you found, and I'll tell you whether you're on the right track.