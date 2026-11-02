# Code Review #4

**Date:** 2026-11-01
**Reviewing:** `bf.cpp` after the `Details` struct and the `dumpState` function (T-1)
**Previous review:** `review-03b.md`

> [!NOTE]
> Same rules: questions and experiments, not ready-made fixes. IDs match `docs/TRACKER.md`. Where I say "I tested", I ran the idea on a copy of the earlier version of your code, not on this exact file.

---

## 1. Verdict

This is the biggest structural step so far, and most of it is done well: eleven copy-pasted trace blocks are now one function, it takes the state by `const` reference, and `tpptr` and `strptr` really live in the struct (no duplicate variables). What is left is to **finish the job**: the struct only holds part of the state, the program text now exists twice, a global rename changed some text it should not have, and **BUG-7 from review #3b is still there**, so any program with a comment or a space still fails.

| Area | Review #3b | Now |
|---|---|---|
| Trace code | 11 copies | One function (`dumpState`), called 11 times |
| State | Loose variables | Struct holds `tpptr`, `strptr`, a copy of the program text; `tape` is separate |
| Unsupported-character check (BUG-7) | Broken | **Unchanged** |
| Stale comments / leftover code | Several | More (old dump blocks are commented out, not deleted) |
| Tests | None | None |

---

## 2. Progress since review #3b

| Item | Status | Notes |
|---|---|---|
| T-1 `dumpState` | Mostly done | See REF-1 for what is missing |
| T-1 struct | Half done | `tape` and the single copy of the program text are not in it |
| BUG-7 | Open | Not touched; see section 5 |
| CLEAN-8 stale TODOs | Open | `TODO(BUG-5)`, `TODO(BUG-2)` (overflow part), `// break;`, "debug liines" all still there |
| T-2 verbose flag | Not started | |
| T-3 / T-4 | Not started | |

---

## 3. What you did well

1. **You applied the `const` reference advice** (`const Details &data`). A function that only prints cannot modify the state by accident.
2. **You chose the "home" approach for the pointers.** `tpptr` and `strptr` exist only inside the struct; there is no second copy that could drift. That was the more important of the two options I described.
3. **You kept the old blocks around while you checked the new ones.** That is a sensible way to compare during a refactor. Now that you have checked, delete them (REF-3).
4. **Every `dumpState` call is at the same point as the block it replaced**, before the case changes any state, so the dump shows the same values as before.
5. **The struct definition is at file scope**, so the function signature can name it.

---

## 4. Issues

### REF-1 (Medium): the struct is only half of the machine state

Look at what `dumpState` needs: `data1` **and** `tape`. And look at where the program text lives.

- How many copies of the program text exist now? (`rdTxt` in `main`, and `data1.rdtxt`.) Which one does the `while` loop and every `switch` read? Which one does `dumpState` read?
- Both hold the same content today. If one of them were ever modified (say, an optimization pass in Stage 3), what would happen to the other? Which one is the "real" program?
- This is the same copy-vs-home question as before, applied to the text. Which choice do you want, and does the text belong in the *machine state* at all (DEC-7)? Think about it: the tape and the two pointers change as the program runs. Does the program text?
- `dumpState` takes `const unsigned char tape[]`. In C++, what is the type of that parameter really? What does `sizeof(tape)` return **inside** the function? What would the function lose that the caller has?
- If the tape also lived in the struct, how many parameters would `dumpState` need? What would you have to be careful about with a 30000-byte member (think about accidental copies, and about the `= {0}` initialization)?

### REF-2 (Low/Medium): the rename changed things it should not have

You renamed `tpptr` and `strptr` to `data1.tpptr` and `data1.strptr` with search and replace. Look at the results:

| Where | What happened |
|---|---|
| `[DEBUG]` message in `case '<'` | The **output string** now reads `"Not moving data1.tpptr , or else data1.tpptr will go in negative"`. Program output changed |
| "Machine model" design note | The `strptr`/`tpptr` lines say `data1.strptr` / `data1.tpptr`, but `tape` does not |
| Several comments | `data1.` appears in sentences ("data1.strptr must advance...") |
| Old comments | `// int data1.tpptr = 0;` and `// int data1.strptr = 0;` are nonsense now |
| `TODO(T-1)` text | Says "implement" about something that is now implemented |

Questions:

- This is exactly what the **before/after `diff`** from review #3b catches. Run the program on `<` with the old binary and the new one. What differs, and why?
- A plain-text replace does not know the difference between code, comments and strings. What does your editor offer for **renaming a symbol** instead? (Search "rename symbol" for your editor.) How does `git diff` help you review a mechanical change?
- You also kept these lines: `data1.tpptr = 0;` and `data1.strptr = 0;`. Look at the struct definition. Do these assignments do anything the struct does not already do? (Search **default member initializer**.)
- Are `Details` and `data1` names that will still be clear in six months? What would you call this type, given what it represents?

### REF-3 (Low): delete the commented-out blocks

Ten commented-out trace blocks sit next to the calls that replaced them. They made the refactor safer, but once the output matches they are noise (and they are what your sanitized file got rid of). Git remembers them. Also check the inconsistent indentation of the new calls; a formatter (`clang-format`) fixes that in one command.

### REF-4 (Optional, worth thinking about): is the call really needed in every case?

Compare what each case does first. Questions:

- Which statements come at the *start* of every case? (Not all of them are in every case; look closely, and look at `default:` in particular.)
- If a statement is identical at the start of every branch of a `switch`, where could it go instead?
- **Careful:** I tested the idea of calling the trace once before the `switch`. For all eight commands the output was identical, but for a non-command character the output changed, because one case does something different from the others. Which one, and what does that tell you about checking the output after a refactor?
- Is the order of the two trace lines (`encWrt` first, then the dump) something you want to preserve?

### Carried over

| ID | State |
|---|---|
| **BUG-7** | Unchanged. `x+` still reports `+` as unsupported; `+++ hello` still exits with status 1. This is the one to fix next; see below |
| CLEAN-8 | Stale `TODO(BUG-5)` (the check exists) and `TODO(BUG-2)` (overflow now exits 1), `// break;`, "debug liines", the "decisions to make" block |
| BUG-2 | Messages still on `cout` |
| BUG-6, CLEAN-1..7 | Unchanged |

---

## 5. Why BUG-7 now matters more

Before this refactor, the always-true condition lived in one block among eleven. Now every other case is short and clear, which makes `default:` the *only* case still doing something wrong. Re-read review #3b, section 4, and run the three experiments (`x+`, `+++ hello`, a CRLF file). The questions there lead to all four problems.

A related point: BUG-7 would have been caught by a test. A file containing a comment is the first thing a test suite for a Brainfuck interpreter would include.

---

## 6. Tests would help you refactor safely

You are about to change structure again (finish the struct, add a verbose flag, maybe exceptions). Every one of those changes should leave behavior identical, and a script can check that for you. You do not need the full T-3 yet. Start with:

- A folder with a handful of `.bf` programs: a loop, a `[-]`, `<` at cell 0, a bad bracket, a comment.
- A script that runs each program and saves output **and exit status**.
- Run it before a refactor and after, and `diff` the two result folders.

Notice that this is what you did by hand in the last section; the script just repeats it.

---

## 7. Test checklist (additions)

| Input | What to check |
|---|---|
| `<` | Compare against the old binary. Which line changed? (REF-2) |
| `x+`, `+++ hello` | Still exits 1 (BUG-7) |
| `+++++++[>++++++++++<-]>.` | Same output as before the refactor |
| Any program, old vs new binary | `diff` the outputs |

---

## 8. Suggested order

1. **BUG-7:** decide DEC-9, then fix it in the right layer.
2. **REF-2, REF-3, CLEAN-8:** clean up the rename damage and delete dead comments and code.
3. **REF-1:** decide what the struct owns (tape? text?) and give each thing exactly one home (DEC-7).
4. **Mini test script:** before you change the structure again.
5. **T-2:** the verbose flag, now that the trace is one function. Consider REF-4 here.
6. **DEC-10:** error-handling mechanism (the exceptions question from review #3b).

---

## 9. Tracker updates to make

**Done log:**

| ID | What | Root cause | Lesson (write your own) |
|---|---|---|---|
| D-15 | Trace moved into `dumpState(const Details&, ...)` | Eleven duplicated blocks | |

**Add as open:**

| ID | Item |
|---|---|
| T-7 | Give every piece of machine state exactly one home (program text, tape); decide DEC-7 |
| T-8 | Consider calling the trace once per iteration (mind `default:`) |
| CLEAN-9 | Undo rename side effects: the `[DEBUG]` string, comments, leftover assignments; choose better names than `Details` / `data1` |
| CLEAN-10 | Delete commented-out dump blocks; run a formatter |
| Q-12 | What is an array function parameter really (decay)? What does `sizeof` give inside the function? |
| Q-13 | Default member initializers vs assignments in `main` |

**Update:**

- T-1: in progress (about two thirds done; see REF-1).
- BUG-7: still open, now the top priority.
- DEC-7: becomes urgent (program text duplicated).

---

## 10. Scorecard

| Area | #1 | #2 | #3 | #4 |
|---|---|---|---|---|
| Core semantics | Good | Good | Good | Good |
| Input handling | Weak | Good | Good | Good |
| Error handling | Missing | Partial | Mostly there | One broken check (BUG-7) |
| Robustness | Missing | Partial | Better | Same |
| Code structure | Weak | Weak | Weak | **Improving** (T-1 in progress) |
| Tooling and tests | None | None | None | None (a small script would help now) |
| Self-diagnosis and notes | Good | Good | Very good | Good (stale comments creeping back) |
