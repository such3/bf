# Project Tracker

The single place for **what is broken, what is next, what was decided, what I still need to learn, and what is done.** Reviews (`docs/reviews/`) are dated snapshots; this file is the living record.

## How to use this file

- Every item has a **stable ID**. Never reuse or renumber one.
  - `BUG-n` something wrong · `T-n` to-do · `CLEAN-n` small cleanup · `DEC-n` design decision · `Q-n` question to learn · `D-n` done
- Status values: `Open`, `In progress`, `Done`, `Dropped`.
- When something is fixed, **move its row to the Done log** and note what caused it and what you learned. Do not delete history.
- Refer to IDs in code and commits, for example `// TODO(T-5): take filename from argv` and `fix(BUG-1): advance strptr when '<' hits cell 0`. Then `grep -rn "TODO" src/` lists your open to-dos.
- Keep code comments for *why the code is like this now*. History goes in commits and the Done log.
- If the list gets long, move to GitHub Issues with labels `bug`, `design`, `learn`, `stage-0` ... and one milestone per stage. The IDs here map directly onto issue titles.

---

## 1. Open bugs

| ID | Severity | Summary | Where | Found | Status |
|---|---|---|---|---|---|
| BUG-1 | High | `<` at cell 0: the branch breaks out of the `switch` without advancing `strptr`, so the same instruction repeats forever | `case '<'` | Review #2 | Open |
| BUG-2 | Medium | Error paths report success: `exit(0)` on unbalanced brackets; the tape-overflow stop ends with `return 0`. Errors also go to `cout` | `computerPos`, main loop | Review #2 | Open |
| BUG-3 | Medium | Failure to open `input.txt` is not detected; an empty program is not handled deliberately | `main` | Review #2 | Open |
| BUG-4 | Low | `.` prints a debug label as part of program output | `case '.'` | Review #1 (3.6) | Open |

## 2. To-do

### Stage 0 (hardening)

| ID | Task | Status |
|---|---|---|
| T-1 | Put `tape`, `tpptr`, `strptr` (and decide about `rdTxt`) in a state struct; write `dumpState` taking `const&`; replace the ~10 repeated trace blocks | Open |
| T-2 | Make the trace switchable (verbose flag) and separate it from program output | Open |
| T-3 | Create `tests/` with programs, expected outputs and a `run_tests.sh` that also checks exit status | Open |
| T-4 | Add a step limit so an infinite loop cannot hang a run | Open |
| T-5 | Take the program filename from `argv` | Open |
| T-6 | Replace the two jump maps with one compact table | Open |

### Stage 1 and later

| ID | Task | Status |
|---|---|---|
| T-10 | Implement `,` (input) | Open |
| T-11 | Report errors with line and column (needs positions tracked while reading) | Open |
| T-12 | Separate load / analyze / execute into functions | Open |

### Small cleanups

| ID | Task | Status |
|---|---|---|
| CLEAN-1 | Experiment with the two `continue`s in `computerPos`; keep only what is needed (or use `else if`) | Open |
| CLEAN-2 | Include the bracket index in the error messages; fix the spelling of "parentheses" | Open |
| CLEAN-3 | Rename `computerPos` to `computePos`; take `const string&` | Open |
| CLEAN-4 | Use `size_t` where comparing with sizes; build with `-Wall -Wextra` and clear the warnings | Open |
| CLEAN-5 | Add `#include <cstdlib>` for `exit`/`EXIT_FAILURE` | Open |
| CLEAN-6 | Fix the stale `// Cast to char` comment in `case '>'` | Open |
| CLEAN-7 | Banner says "compiler"; it is an interpreter | Open |

---

## 3. Design decisions

Write the decision and the reason, even for small ones. Future you will forget why.

| ID | Question | Options | Decision | Status |
|---|---|---|---|---|
| DEC-1 | Pointer moves left of cell 0 | Stop with an error / wrap to the end / clamp (ignore the move) / grow the tape | _undecided_ (code currently clamps, which causes BUG-1) | Open |
| DEC-2 | Pointer moves right of the last cell | Same options | Stop with an error (current) | Decided, implemented |
| DEC-3 | Exit codes | One failure code, or separate codes for bad program / runtime error / I/O error | _undecided_ | Open |
| DEC-4 | Where do errors and trace go | `stdout` vs `stderr`; flag for trace | _undecided_ | Open |
| DEC-5 | Empty program | Error, or valid no-op | _undecided_ | Open |
| DEC-6 | Multi-line source | Concatenate lines (current). Consequence: line and column information is lost | Revisit in Stage 1 | Decided for now |
| DEC-7 | Is `rdTxt` part of the machine state or the program the machine runs? | In the state struct / separate | _undecided_ (matters for Stage 2) | Open |

---

## 4. Questions and things to learn

| ID | Question | Status | Where answered |
|---|---|---|---|
| Q-1 | What are exit codes and why do they matter? | Open | review-02, BUG-2 |
| Q-2 | How does the unsigned representation work, and why do casts change what `cout` prints? | Partly answered | review-01 3.7; check integer promotion yourself |
| Q-3 | How does a helper function access `main`'s variables, and what if there are many? | Answered | Group into a struct, pass by `const&` (T-1) |
| Q-4 | How do I identify unbalanced brackets in nested loops? | Answered | The stack pairs each `]` with the innermost open `[`; trace `++++[*-***-*>**>]]` to see it |
| Q-5 | Why did printing tape content show wrong values? | Answered | Casting to `(int)` (plus `unsigned char`) |
| Q-6 | Why does `unordered_map` lack two-way lookup, and do I even need two maps? | Open | review-02, DESIGN-1 |
| Q-7 | What is the difference between `cout` and `cerr`? | Open | review-02, BUG-2 |
| Q-8 | What does `break` leave when it is inside a `switch` inside a loop? | Open | review-02, BUG-1 |

---

## 5. Done log

Newest first. Format: **what happened, root cause, lesson.**

| ID | Fixed in | What | Root cause | Lesson |
|---|---|---|---|---|
| D-9 | Review #2 | Tape upper-bound check | Was comparing `cnt` (iteration count) instead of `tpptr` | Check the variable that actually represents the limit |
| D-8 | Review #2 | Unbalanced bracket detection, with messages | No validation; a stray `]` was ignored | Validate input before running; a stack handles nesting |
| D-7 | Review #2 | Multi-line input and `getline` | The loop overwrote `rdTxt`, and the failing last call cleared it | Separate "current line" from "accumulated text" |
| D-6 | Review #2 | `#include <string>` | Relied on another header to provide it | Include what you use |
| D-5 | Review #2 | `>` trace cast | A leftover `(char)` cast | Keep similar code paths consistent |
| D-4 | Before review #1 | `]` jump mutated the map | `revpos[strptr]++` modified the stored value | Read, then add; do not post-increment a lookup |
| D-3 | Before review #1 | `[` jump used the wrong key | Looked up with `tpptr` instead of `strptr` | The map is keyed by code positions |
| D-2 | Before review #1 | `>` printed the same character forever | Forgot to advance `strptr` | Every case must advance the program position (see BUG-1 for the same class of bug) |
| D-1 | Before review #1 | Nested loops matched wrongly | Matching logic, fixed with a stack | A stack is the natural tool for nesting |

---

## 6. Review history

| Review | Date | Highlights |
|---|---|---|
| #1 | before 2026-10-27 | First pass: structure, input bug, bounds, roadmap |
| #2 | 2026-10-27 | Input and brackets fixed; BUG-1 (lower bound) found; exit codes next |