# bf: a Brainfuck interpreter in C++

A learning project. It starts as a plain interpreter and grows, stage by stage, into an optimizing compiler with C, assembly and LLVM backends. The goal is to understand how compilers, LLVM and computer architecture fit together by building each piece myself.

> [!NOTE]
> **Status:** Stage 0 of 9 (hardening the interpreter). It runs simple BF programs, but it is very chatty (a trace is printed for every instruction) and has known bugs. See [`docs/TRACKER.md`](docs/TRACKER.md).

---

## What is Brainfuck?

A minimal language with eight commands operating on a tape of byte-sized cells and one data pointer. Every other character is a comment.

| Command | Meaning |
|---|---|
| `>` | Move the data pointer one cell right |
| `<` | Move the data pointer one cell left |
| `+` | Increment the current cell |
| `-` | Decrement the current cell |
| `.` | Output the current cell as a character |
| `,` | Read one character into the current cell *(not implemented yet)* |
| `[` | If the current cell is 0, jump past the matching `]` |
| `]` | If the current cell is nonzero, jump back to just after the matching `[` |

Example: `+++++++[>++++++++++<-]>.` computes 7 × 10 = 70 in the second cell and prints it, which is ASCII `F`.

---

## Build and run

Adjust the source path to wherever your file lives.

```bash
# Development build: warnings on, debug info, sanitizers
g++ -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined src/bf.cpp -o bf

# Run (currently reads ./input.txt from the working directory)
./bf
```

Planned: take the program file from the command line (`./bf program.bf`).

---

## Current behaviour

| Topic | Behaviour today |
|---|---|
| Tape | 30000 cells, each an 8-bit unsigned value, all starting at 0 |
| Cell arithmetic | Wraps around: `0 - 1` is 255, `255 + 1` is 0 |
| Tape pointer past the right end | Stops with an `[OVERFLOW]` message |
| Tape pointer past the left end | **Known bug**, see BUG-1 in the tracker |
| Unbalanced brackets | Detected before running; prints an error |
| Multi-line source | Lines are joined into one program |
| Input (`,`) | Not implemented |
| Output | Mixed with debug trace (to be separated) |
| Exit status | **Known bug**: errors currently exit with 0 |

---

## Project layout

```
bf-interpreter/
├── README.md                this file
├── src/
│   └── bf.cpp               the interpreter
├── tests/
│   ├── programs/            .bf test programs
│   ├── expected/            expected output for each program
│   └── run_tests.sh         runs every program and diffs the output
└── docs/
    ├── ROADMAP.md           the long-term plan (stages 0 to 9)
    ├── TRACKER.md           bugs, to-dos, decisions, questions, done log
    └── reviews/
        ├── review-01.md     first review (archived)
        └── review-02.md     latest review
```

`tests/` does not exist yet; creating it is part of Stage 0.

---

## Documentation

| Document | Use it for |
|---|---|
| [`docs/ROADMAP.md`](docs/ROADMAP.md) | Where this project is going and why |
| [`docs/TRACKER.md`](docs/TRACKER.md) | What is broken, what is next, what was decided |
| [`docs/reviews/`](docs/reviews/) | Dated snapshots of code reviews |

---

## Roadmap at a glance

| Stage | Title | State |
|---|---|---|
| 0 | Harden the interpreter | In progress |
| 1 | Separate load / analyze / execute | |
| 2 | Introduce an IR | |
| 3 | Optimize the IR | |
| 4 | Fast interpreter, architecture lessons | |
| 5 | Backend: emit C | |
| 6 | Backend: assembly | |
| 7 | Backend: LLVM IR | |
| 8 | JIT (optional) | |
| 9 | MLIR dialect (optional) | |

Details are in [`docs/ROADMAP.md`](docs/ROADMAP.md).

---

## Testing

Planned for Stage 0: every program in `tests/programs/` has an expected output in `tests/expected/`, and `tests/run_tests.sh` runs them all, comparing output and exit status. Later stages add differential testing (every backend must agree with the interpreter).

---

## Learning goals

- How a language implementation is structured (front end, IR, passes, back end)
- How optimizations are expressed and proven safe
- How code generation maps to real machines (registers, ISA, calling conventions)
- How LLVM IR works and what the LLVM optimizer does