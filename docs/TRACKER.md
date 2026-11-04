# Project tracker

This file is the living list of the project’s known work, decisions, and follow-up items. It is meant to match the current state of [bf.cpp](../bf.cpp), not a stale draft from an earlier version of the project.

> Current status: the interpreter is functional as a file-backed CLI, but it is still in a hardening phase. The next work is about cleanup and correctness rather than shipping a full compiler.

---

## Current state summary

### Working today

- Reads a Brainfuck source file from the command line.
- Validates bracket balance before execution.
- Executes the core Brainfuck instructions (`+ - < > . [ ]`).
- Tracks a tape of 30000 byte cells.
- Prints debug state when `--verbose` is used.
- Guards against tape overflow at the right end and out-of-range left access.

### Still unfinished or rough

- `,` input is not implemented.
- Unknown flags and extra arguments are not yet handled cleanly: the current code prints help for unsupported flags but does not short-circuit cleanly before execution.
- The project still uses verbose tracing heavily while it is being debugged.
- The jump resolution uses the current two-map setup and can be simplified later.
- There is no single formal CLI policy yet for the interpreter’s error and exit paths.
- Line-and-column diagnostics are not tracked yet because the source is concatenated during reading.

### Verified current behavior

A fresh run of `./run input.txt --ver` currently prints the help text and then exits with the left-bound tape error:

```text
Help
Use --verbose for verbose mode .
[ERROR] Access out of tape bounds
[ERROR] trying to move data_ptr before 0
```

---

## Open items

| ID | Area | Summary | Status |
|---|---|---|---|
| T-4 | Runtime safety | Add a step limit so a runaway loop cannot hang the process indefinitely | Open |
| T-5 | CLI | Improve error messages when the program path is missing or invalid | Open |
| T-2 | Debugging | Keep `--verbose` but make the trace more controlled and less noisy | Open |
| T2-8 | CLI policy | Define and clean up behavior for extra args and unsupported flags | Open |
| T-6 | Implementation | Replace the two jump maps with a more compact structure | Open |
| T-10 | Features | Implement the `,` input instruction | Open |
| T-11 | Diagnostics | Report useful line and column information for bracket or runtime problems | Open |
| DEC-5 | Design | Decide whether a zero-byte file is valid or a hard error | Open |
| DEC-6 | Design | Decide how multi-line source, comments, and diagnostic tracking fit together | Open |
| DEC-11 | Design | Revisit whether the verbose flag should become part of a real options structure | Open |
| DEC-13 | Design | Keep the interpreter output entirely under program control; do not add implicit output | Open |

---

## Code-level notes from the current source

These are the actual active concerns visible in [bf.cpp](../bf.cpp):

- `TODO(T2-8)`: extra arguments and unsupported flags still need a clearer policy.
- `TODO(T-4)`: a step limit should prevent infinite loops from stalling the interpreter.
- `TODO(T-10)`: the `,` instruction is still intentionally skipped.
- `TODO(T-6)`: the current forward and reverse jump maps should eventually be simplified.
- `TODO(T2-5)`: file-open failures could report the exact path cleanly.
- `TODO(DEC-11)`: verbose tracing is currently a global flag, not a richer options object.
- `TODO(REF-5)`: the final debug dump reads at `program[pc]` at the end-of-program boundary and should be revisited in a later phase.

---

## Decisions already captured in code

| ID | Decision | Note |
|---|---|---|
| DEC-1 | Left-bound movement is an error | Moving left from cell 0 stops the program with an error |
| DEC-3 | Failures exit with status 1 | The code uses `exit(1)` on runtime and file errors |
| DEC-5 | Empty files are an error | This is enforced in the reading path |
| DEC-9 | Non-command characters are comments | The switch falls through to the default case and skips them |
| DEC-12 | Trace output uses `clog` and stays separate from program output | This keeps `.` output on stdout |
| DEC-13 | The interpreter adds no output of its own | It relies entirely on the Brainfuck program to emit output |

---

## Planned next milestones

1. Stabilize the CLI and error reporting.
2. Add a step limit and better guard rails for runaway programs.
3. Implement `,` input handling.
4. Separate program loading, analysis, and execution into clearer phases.
5. Move toward the IR and compiler-stage roadmap in [docs/ROADMAP.md](ROADMAP.md).

---

## Review note

This tracker is intentionally current and minimal. It avoids the older review-only wording because the project has already moved past that stage. The source comments are the definitive guide for what is still open.