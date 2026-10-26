# Brainfuck interpreter: TODO

Status as of commit `b77b106`. Jump logic (`[` and `]`) works. Items below are still open.

Compile with warnings and sanitizers on while developing:

```bash
g++ -g -Wall -Wextra -fsanitize=address,undefined main.cpp -o main
```

## 1. Tape bounds (do this first)

- [ ] Decide where the check lives: in the `>` and `<` handlers, or in the loop condition
- [ ] Decide what the error message says (which instruction, which position in the program)
- [ ] Decide what `main` returns when the run is cut short, and how that differs from a normal finish
- [ ] Make a clear distinction between "program finished" and "program was stopped"

Tests (write your prediction first, then run):

- [ ] `<`
- [ ] `+[>+]`
- [ ] `+++++++[>++++++++++<-]>.` (should still print `F`)

## 2. Output you can trust

- [ ] Fix the dump in the `>` case so it prints the cell value the same way as the other cases
- [ ] Separate real program output from debug output (the `.` label should not be part of what the BF program prints)
- [ ] Make debug tracing optional instead of always on

## 3. Robustness

- [ ] Unmatched brackets: test `[+` and `+]`, predict `strptr` before running
- [ ] File reading: trailing newline in `input.txt` currently gives zero iterations, find out why
- [ ] File reading: what if `input.txt` does not exist or cannot be opened?
- [ ] Fix the two signed/unsigned comparison warnings
- [ ] Add an iteration limit while debugging so `+[]` cannot flood the terminal

## 4. Features

- [ ] `,` (input)
- [ ] Multi-line programs: decide how newlines and non-instruction characters are treated
- [ ] Replace the repeated dump block with a single `dumpState` function
- [ ] Test programs: Hello World, nested loops, a loop that never runs

## 5. Stretch

- [ ] Strip non-instruction characters up front instead of skipping them at run time
- [ ] Collapse runs of `+`, `-`, `>`, `<` into single operations
- [ ] Think about what an intermediate form would look like if this became a real compiler

## Habits

- Write the test program and the expected output before running it
- One logical change per commit
- Update this file when an item is done, and note the commit it was finished in