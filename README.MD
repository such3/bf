# Brainfuck interpreter

This project is a small C++ Brainfuck interpreter built as a learning exercise in compiler construction. The implementation currently lives in [bf.cpp](bf.cpp), and the project is organized around the interpreter itself, the runner, and the design notes in [docs/ROADMAP.md](docs/ROADMAP.md) and [docs/TRACKER.md](docs/TRACKER.md).

> Status: the interpreter is working as a file-based CLI and is in the hardening / design-cleanup phase. The code is not a finished compiler yet; it is a practical stepping stone toward understanding how interpreters, jumps, diagnostics, and later compiler backends fit together.

---

## What the interpreter does

Brainfuck is a tiny language built around a tape of byte cells and a data pointer. The current implementation supports the core instructions used by the language model:

| Command | Meaning |
|---|---|
| `>` | Move the tape pointer right |
| `<` | Move the tape pointer left |
| `+` | Increment the current cell |
| `-` | Decrement the current cell |
| `.` | Output the current cell as a byte |
| `,` | Input one byte into the current cell *(not implemented yet)* |
| `[` | Jump past the matching `]` if the current cell is zero |
| `]` | Jump back to the matching `[` if the current cell is nonzero |

The project follows the usual Brainfuck semantics: every other character is treated as a comment, and the source text is read as a program stream.

---

## Build and run

From the project root:

```bash
g++ -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined bf.cpp -o run
./run input.txt
./run input.txt --verbose
```

The program expects the Brainfuck source file as the first argument. The script and the executable are intentionally simple at this stage: the source file is read from disk, validated, and executed immediately.

The supported optional flag is `--verbose`. Unsupported flags currently print a help message and continue processing; this is part of the current CLI behavior while the parser is still being hardened.

Example:

```bash
./run input.txt --verbose
```

This prints the state trace to stderr when verbose mode is enabled and leaves the program’s own output on stdout.

> Fresh verification: running `./run input.txt --ver` in the current project prints the help text and then triggers the left-bound tape error, which is consistent with the source logic in [bf.cpp](bf.cpp).

---

## Current behavior in this repo

The current implementation is not a theoretical design document; it matches the actual code in [bf.cpp](bf.cpp):

| Topic | Current behavior |
|---|---|
| File input | Reads the Brainfuck source from a path passed on the command line |
| Tape model | 30000 cells, each a byte-sized unsigned value, all starting at zero |
| Cell arithmetic | Wraps around on overflow/underflow |
| Right-bound safety | Stops with an overflow error when the tape pointer moves past the last cell |
| Left-bound safety | Stops with an error if the pointer moves below zero |
| Bracket validation | Checks matching `[` and `]` before execution |
| Non-command characters | Ignored as comments |
| `,` input | Not implemented yet |
| Verbose tracing | Optional via `--verbose`; prints instruction and state dumps |
| Output model | Writes raw bytes produced by `.` to stdout; the interpreter itself adds no extra newline |

The current code also distinguishes output from tracing by separating stdout and stderr usage, although the implementation still uses verbose logging heavily while it is being debugged.

---

## Repository layout

```text
Brainfuck/
├── bf.cpp                 C++ interpreter source
├── run                    compiled executable produced by the build step
├── input.txt              example Brainfuck input file
├── README.MD              project overview and usage
├── index.html             browser-facing prototype/demo entry point
├── web/                   frontend assets for the browser view
├── docs/
│   ├── ROADMAP.md         long-term plan from interpreter to compiler stages
│   ├── TRACKER.md         active issues, decisions, and to-dos
│   └── reviews/           archived review notes
├── dev-logs/              working notes and project history
└── rules/                 language/project rules and notes
```

---

## Documentation

| Document | Purpose |
|---|---|
| [docs/ROADMAP.md](docs/ROADMAP.md) | Long-term plan and compiler path |
| [docs/TRACKER.md](docs/TRACKER.md) | Current issues, decisions, follow-up work |
| [docs/reviews/](docs/reviews/) | Historical review snapshots |
| [dev-logs/](dev-logs/) | Project notes and working history |

---

## Roadmap in brief

The long-term direction is already visible in the code comments and the roadmap:

1. Harden the interpreter and fix runtime edge cases.
2. Separate loading, validation, and execution phases.
3. Introduce an intermediate representation.
4. Add optimization passes.
5. Move toward C, assembly, and LLVM-based backends.

This is a compiler-learning project, not a final product. The current interpreter is the foundation for the stages that follow.

---

## Notes

- The project intentionally keeps the code readable and educational, with comments describing decisions and open follow-up work.
- The code is structured as a learning exercise, so design comments and TODO markers are part of the source itself.
- The current status is intentionally honest: the project is functional but still rough around the edges while it is being hardened.