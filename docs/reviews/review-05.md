# Code Review #5

**Date:** 2026-11-01
**Reviewing:** `bf.cpp` after the tape moved into the struct, `dumpState` took a single argument, and the rename damage in comments and the `[DEBUG]` string was reverted
**Previous review:** `review-04.md`

> [!NOTE]
> Same rules: questions and experiments, not ready-made fixes. IDs match `docs/TRACKER.md`. Where I say "I ran", I built the file and compared it against an earlier build of your code.

---

## 1. Verdict

The structural work from review #4 is essentially done. The tape, the pointers and the program text now live in one struct, the trace is one function with one parameter, and the execute loop reads everything through the struct. I ran 13 cases (a loop, `<` at cell 0, tape overflow, bad brackets, an empty file, no arguments, a missing file, `x+`) against the build from before the refactor, and the output and exit status are identical. So the refactor did what a refactor should: it changed the structure and nothing else.

What is left is no longer structure. It is **correctness** (BUG-7), **housekeeping** (stale comments), and **tooling** (tests and the verbose flag). BUG-7 is now the only wrong behavior left inside the `switch`.

| Area | Review #4 | Now |
|---|---|---|
| Trace code | One function, two parameters | One function, one parameter |
| State | Pointers and a copy of the text in the struct; tape loose | Pointers, text and tape all in the struct |
| Rename side effects | Present | Reverted (comments and the `[DEBUG]` string) |
| Unsupported-character check (BUG-7) | Broken | **Unchanged** (now marked "UNRESOLVED" in a comment) |
| Stale comments | Several | Same ones remain |
| Tests | None | None |

---

## 2. Progress since review #4

| Item | Status | Notes |
|---|---|---|
| T-1 `dumpState` and the struct | Done | See REF-1 for one small leftover |
| REF-1 single home for state | Mostly done | The execute loop uses `data1.rdtxt` and `data1.tape` only |
| REF-2 rename damage | Done | Reverted |
| REF-3 commented-out dump blocks | Done | Deleted |
| CLEAN-10 formatting | Done | Formatter applied |
| REF-4 call the trace once per iteration | Not applied | Optional; see section 4 |
| Q-12 array parameter decay | No longer applies | `tape` is not a parameter any more |
| BUG-7 | Open | Same condition as before |
| CLEAN-8 stale TODOs | Open | Same list |
| CLEAN-9 better names than `Details` / `data1` | Open | |
| T-2, T-3, T-4 | Not started | |

---

## 3. What you did well

1. **You finished the refactor.** In review #4 the struct held part of the state. Now `dumpState(const Details &)` is all one idea: the machine is the struct.
2. **You kept `const` correctness.** Nothing in the trace can change the state by accident.
3. **You spotted the rename problem and described its cause** ("they happened because I used change all occurrences"). Diagnosing the mechanism is more valuable than the fix.
4. **You marked BUG-7 as unresolved** in the code instead of letting it blend in. That is honest bookkeeping.
5. **The code is now formatted consistently**, which makes `git diff` much easier to read.

---

## 4. Issues

### BUG-7 (High, unchanged): the unsupported-character check

The condition in `default:` is still `x != '\t' || x != '\n' || x != '\0'`. Re-run the experiments from review #3b:

```bash
printf 'x+'        > t1.bf
printf '+++ hello' > t2.bf
printf '+++\r\n'   > t3.bf
```

All three still exit with status 1. The four problems listed in review #3b (always true, wrong index, contradicts the language, wrong layer) are all still present.

**A suggestion that fits where you are now:** write the failing test first. A file with a comment (`+++ hello`) should run and exit with status 0 if you choose the standard behavior (DEC-9). Today it exits 1. A test that fails for a known reason is a precise description of the bug, and it proves the fix when it passes. See section 6.

### REF-1 (Low): the read buffer and the struct's copy

`data1.rdtxt = rdTxt;` copies the text once, after reading. From then on:

- `computerPos` validates the local `rdTxt`.
- The execute loop reads `data1.rdtxt`.

Today they are identical. Questions:

- Is there a reason to have both after the file is read? What if the lines were appended straight into the struct's text?
- If `computerPos` took the struct's text instead, how many places would hold the program?
- This is DEC-7 in disguise. You have effectively decided that the program text is part of the machine state. Write that down, with the reason, and note that Stage 2 (the IR) is where it will probably change, because the program becomes a list of instructions instead of text.

### REF-4 (Optional): the trace in every case

Each case still starts with `encWrt(...)` and `dumpState(data1)`, and you also print `char : ...` before the switch. Think about what you see on screen for one instruction: three lines that all name the same character.

- Is each of those lines telling you something different?
- When you add the verbose flag (T-2), how many places must you touch to switch the trace off? How many could it be?
- The earlier hint still applies: one case (`default:`) does not call `encWrt`. Whatever you decide, run the before/after `diff` on a program with a comment character to see what the change does to the output.

### CLEAN-8 (still open): stale comments

None of these has changed since review #4:

| Where | Problem |
|---|---|
| `TODO(T-1)` in the design notes and above `dumpState` | Both describe work that is finished |
| `TODO(BUG-5)` above `argv[1]` | The `argc` check exists |
| `TODO(BUG-2)` above the overflow check | Overflow now exits with 1; only the `cerr` part remains |
| `// break;` in the overflow branch | Dead code |
| "debug liines ( remove after checks )" and the commented `cout` below it | Remove when BUG-7 is fixed |
| "decisions to make" block | Belongs in the tracker (DEC-8, T-2) |
| "// UNRESOLVED BUG STILL NEEDS TO BE FIXED" | Make it `TODO(BUG-7)` so it can be searched and linked |

**Habit:** a TODO is deleted in the same commit that does the work it describes.

### CLEAN-9 (still open): names

`Details` and `data1` tell the reader nothing. What does this struct represent? What would you call one instance of it? A good name saves a comment.

**Tip for next time:** a text search-and-replace cannot tell code from comments from strings. A *rename symbol* command (in VS Code, F2 on the name) understands the language and changes only the identifier.

### Small things

| Item | Note |
|---|---|
| Redundant assignments | `data1.tpptr = 0;` and `data1.strptr = 0;` repeat what the struct's default initializers already do (Q-13). Does either line do anything? |
| `int` vs `size_t` | Two warnings with `-Wall -Wextra`: in `computerPos` and in the `while` condition (CLEAN-4) |
| `<cstdlib>` | Still missing (CLEAN-5) |
| Large struct | `Details` is about 30 KB. You pass it by reference, which is right. What would happen on every call if you passed it by value? |
| Error stream | Messages still on `cout` (BUG-2) |
| Typos in strings | "paranthesis", "Arguement" (CLEAN-2) |
| Banner | Still says "compiler" (CLEAN-7) |

---

## 5. Where Stage 0 stands

Done: input handling, bracket validation, tape bounds, exit statuses, argument checks, the struct and trace function.

Remaining:

| Item | What it needs |
|---|---|
| BUG-7 | Decide DEC-9, then fix it in the right layer |
| T-2 | Verbose flag; keep the trace out of the program's output (BUG-4) |
| BUG-2 | Errors on `cerr` |
| T-3 | A test script |
| T-4 | A step limit |
| Decisions | DEC-1, DEC-3, DEC-5, DEC-7, DEC-8, DEC-9 written down |
| Cleanup | CLEAN-1 to CLEAN-10 |

Stage 0 is considered finished when the test script passes and the decisions are written down, not when the code merely looks tidy.

---

## 6. A first test script, as a plan

You do not need the final output format to start. Until T-2, the trace is mixed into the output, so test the things that will not change: **exit status** and a few key lines.

Steps:

1. Create `tests/programs/` with one tiny program per file (see the table).
2. Write a script that loops over them, runs each, and compares the exit status with an expected value.
3. For the programs that must produce a particular cell value, search the output for a stable line (for example the one that contains the printed character).
4. When T-2 lands, regenerate expected outputs *deliberately* and compare whole outputs.

| Program | Expected exit status | Notes |
|---|---|---|
| `+++++++[>++++++++++<-]>.` | 0 | Prints `F` |
| `+[-]` | 0 | Cell ends at 0 |
| `+++ hello` | **0** | Comments are allowed. **Fails today (BUG-7)** |
| `<` | decide (DEC-1) | Currently 0 |
| `]` | 1 | Unmatched `]` |
| `[` | 1 | Unclosed `[` |
| 30000 `>` then `+` | 1 | Overflow |
| (empty file) | 1 | |
| (missing file) | 1 | |
| (no arguments) | 1 | |

Do not edit the expected value to make a failing test pass. If the test is right, fix the program.

---

## 7. Suggested order

1. **BUG-7:** write the failing test, decide DEC-9, fix it where it belongs.
2. **CLEAN-8:** delete the stale TODOs and dead comments, in the same commit as the fix that made them stale.
3. **T-3:** the test script (section 6). Everything after this is safer with it.
4. **T-2:** the verbose flag, with REF-4 in mind.
5. **BUG-2 / DEC-10:** `cerr`, and the error-handling mechanism (the exceptions question from review #3b).
6. **T-4:** the step limit.
7. **CLEAN-9, REF-1:** names and the read buffer.

---

## 8. Tracker updates to make

**Done log:**

| ID | What | Root cause | Lesson (write your own) |
|---|---|---|---|
| D-16 | Tape and program text moved into the struct; trace is `dumpState(const Details&)` | Eleven duplicated blocks; state spread over loose variables | |
| D-17 | Rename damage reverted | "Change all occurrences" edited comments and a string literal | |
| D-18 | Commented-out dump blocks deleted; code formatted | | |

**Update:**

- T-1: done.
- T-7: mostly done (REF-1 describes the small leftover).
- DEC-7: decided for now: the program text is part of the machine state (revisit in Stage 2).
- CLEAN-9: side effects fixed; better names still open.
- CLEAN-10: done.
- Q-12: dropped (the tape is no longer a parameter).

**Add as open:**

| ID | Item |
|---|---|
| T-9 | Remove the duplicate read buffer, or write down why it stays |
| Q-14 | What is a "rename symbol" refactoring, and how is it different from text replace? |

---

## 9. Scorecard

| Area | #1 | #2 | #3 | #4 | #5 |
|---|---|---|---|---|---|
| Core semantics | Good | Good | Good | Good | Good |
| Input handling | Weak | Good | Good | Good | Good |
| Error handling | Missing | Partial | Mostly there | One broken check | One broken check |
| Robustness | Missing | Partial | Better | Same | Same |
| Code structure | Weak | Weak | Weak | Improving | **Good** |
| Tooling and tests | None | None | None | None | None (next focus) |
| Self-diagnosis and notes | Good | Good | Very good | Good | Good |
