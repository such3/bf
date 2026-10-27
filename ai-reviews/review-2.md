# Code Review #2

**Date:** 2026-10-27
**Reviewing:** `bf.cpp` after the multi-line input, balanced-bracket and tape-bound changes
**Previous review:** `review-01.md` (your earlier combined doc, archived)

> [!NOTE]
> Same rules as before: issues are written as questions and experiments, not ready-made fixes. Item IDs (BUG-1, T-3, ...) match `docs/TRACKER.md`.

---

## 1. Verdict

Real progress. You fixed the two most important problems from review #1 (input handling and bracket validation) and you diagnosed several bugs yourself along the way. One **new serious bug** came in with the lower-bound change (BUG-1), and the exit-code question you wrote down in a comment is the next most valuable thing to learn.

| Area | Review #1 | Now |
|---|---|---|
| File input | Likely buggy | Works for multi-line files |
| Bracket matching | Correct | Correct, plus validation |
| Unbalanced brackets | Silent | Detected, but reports success (BUG-2) |
| Tape upper bound | Awkward | Checked, stops with a message |
| Tape lower bound | Missing | **New bug: infinite loop (BUG-1)** |
| Trace code | Duplicated ten times | Still duplicated (T-1) |
| Structure | Everything in `main` | Same |

---

## 2. Progress since review #1

| Item | Status | Notes |
|---|---|---|
| 3.1 `getline` overwrote `rdTxt` | Fixed | See 3.1 below |
| 3.2 Unbalanced brackets | Fixed, with caveats | Detection works; see BUG-2, CLEAN-2 |
| 3.3 Upper bound | Fixed | Moved to the top of the loop, uses `tpptr` |
| 3.3 Lower bound | Introduced a bug | See BUG-1 |
| 3.5 `>` case cast | Fixed | Now `(int)`, like the others (the `// Cast to char` comment is stale) |
| 3.8 `#include <string>` | Fixed | |
| 3.4 One compact structure instead of two maps | Not started | See DESIGN-1 |
| 3.6 Program output vs trace output | Not started | `.` still prints a label |
| 3.5 Trace into a function | Not started | `dumpState()` is still an empty stub |

---

## 3. What you did well

1. **Your input fix shows the right insight.** The failing final `getline` call clears `rdLine`, not `rdTxt`. By separating "the line I just read" from "everything so far", the problem disappears. That is a real understanding of what `getline` does, not a workaround.
2. **You diagnose your own bugs.** The `cnt` vs `tpptr` mix-up, the missing `strptr++` in `>`, the map-key mix-up: each is explained in a comment. Keep doing that, but see section 6 on where those notes should live.
3. **You noticed the exit-code question yourself** ("learn more about exit codes"). That's the right thing to be curious about.
4. **You added tagged messages** (`[ERROR]`, `[OVERFLOW]`, `[DEBUG]`). Categorizing output is the start of a logging design.
5. **You wrote your doubts down as questions** inside the code. Those are valuable; they just belong in a tracker (see `TRACKER.md`, "Questions").

---

## 4. Issues

### BUG-1 (High): `<` at cell 0 causes an infinite loop

**Where:** `case '<'`, the `if (tpptr == 0)` branch.

**Experiment:** put just `<` in `input.txt` and run with a safety net:

```bash
timeout 2 ./bf ; echo "exit status: $?"
```

(An exit status of 124 means `timeout` killed it.)

**Questions to answer:**

- After that `break`, what is the value of `strptr`?
- What does the `while` loop execute on its next iteration?
- Look at the `break` in your tape-overflow check and the `break` in this branch. Both use the same keyword. Which construct does each one leave? (Hint: a `break` leaves the **nearest enclosing** loop or `switch`.)

**A design question that is bigger than the bug:** even if you make the program advance, is silently ignoring a `<` correct? The rest of the program will then run on the wrong cell, and its output will be wrong without any warning. Your upper bound **stops with an error**. Should the lower bound behave the same way? Consistency is a good principle; record your decision as DEC-1 in the tracker.

**Another thing to explore:** once you decide to stop, how do you stop from *inside* a `switch` inside a `while`? List the options (a flag, `return`, `exit`, restructuring) and think about the trade-offs of each.

### BUG-2 (Medium): error paths exit with status 0

**Where:** both `exit(0)` calls in `computerPos`, and the tape-overflow `break` (the program then ends normally with `return 0`).

**Experiment:**

```bash
./bf ; echo $?
```

Try it with valid input and with an unbalanced program. What do you see in both cases?

**What to learn (your own TODO):**

- Every process ends with an **exit status** (0 to 255) that the shell, `make`, scripts and CI read. By convention, `0` means success and any nonzero value means failure. A program that reports an error but exits with 0 tells every script "all fine".
- Your future test script (Stage 0) will depend on this: it needs to tell "program ran" from "program rejected the input".
- Look up `EXIT_SUCCESS` and `EXIT_FAILURE` (and which header they live in; right now `<cstdlib>` is not included, and it only works through another header).
- Different nonzero codes can mean different categories (bad program, runtime error, I/O error). Decide whether you want that (DEC-3).

**Related questions:**

- Errors currently go to `cout`. What if the user runs `./bf > out.txt`? Where would the error message go? What's the difference between `cout` and `cerr`?
- `return;` after `exit(...)` can never run. Why?
- Should a helper like `computerPos` be allowed to kill the whole process? How would you test it if it does? What could it do instead (return a status, return a position, throw)? You don't need to change it yet, but think about it before Stage 1.

### BUG-3 (Medium): a missing or empty input file fails silently

**Where:** `ifstream file("input.txt");`

**Experiments:**

1. Rename `input.txt` and run. What happens? What exit status do you get?
2. Make `input.txt` empty and run.

**Questions:**

- How can you ask the stream whether opening succeeded?
- Is an empty program an error, or a valid program that does nothing? That is your call; write it down as a decision.

### BUG-4 (Low): `.` mixes a label into program output

`case '.'` still prints ` Content in the Cell : ...`. A BF program's output should be just its characters. This is your review-#1 item 3.6 ("think more about making it robust"). Things to think about:

- What are all the *kinds* of output your program produces? (program output, trace, errors, banner.) Which ones should be on `stdout`, which on `stderr`?
- If you run `./bf > out.txt`, what should `out.txt` contain?
- How would you switch the trace on and off without editing the code?

### CLEAN-1: one `continue` is necessary, one is not

In `computerPos` you added two `continue` statements and wrote that without them the code "was just hitting it on every character". Test that explanation with experiments:

1. Remove the `continue` after `stk.push(i)`. Does anything change? Why or why not? (Think about what the next `if` tests.)
2. Restore it, then remove the `continue` inside the `if (!stk.empty())` block. Now what happens, and why?

The two have different answers, and understanding why is worth a few minutes. Also look up `else if` as the other way to write this.

### CLEAN-2: error messages say *what* but not *where*

Your example `++++[*-***-*>**>]]`:

- **Your note asks how to identify unbalanced brackets in nested loops.** Trace that string by hand with your stack: write the stack contents after every bracket. At which index does the second `]` find an empty stack? You have already built the answer: the stack pairs each `]` with the innermost open `[`, so nested loops need no special handling.
- Try the same trace on `[[]`, `[]]` and `][`.
- Your message could include the **index** of the offending bracket. For an unclosed `[`, what is left on the stack at the end, and what does that tell you?
- Later, with line and column (Stage 1), you will need to track positions while reading. Notice that concatenating lines with `append` throws that information away. Not a bug today; just remember it.
- Spelling: "paranthesis" should be "parentheses" (or just say "brackets").

### DESIGN-1: two maps vs something simpler (still open)

Questions from review #1 that you highlighted:

- Can one code position be both `[` and `]`? (No.) So what does that say about needing two tables?
- Your keys are positions `0 .. n-1`. What is the simplest container that is indexed by consecutive integers? How big would it need to be?
- Now that brackets are validated before running, every lookup is guaranteed to find its key. Does that change how safe `operator[]` is? Is it still the right tool for *reading*?

### T-1: the trace function (still open)

`dumpState()` is still an empty stub. You already have the idea from our last discussion: put `tape`, `tpptr`, `strptr` (and decide about `rdTxt`) into one struct, make that the real home of those variables, and pass it by `const` reference. Try it on `+` and `>` first, compare output with the old version, then convert the rest of the cases.

### Small things

| Item | Note |
|---|---|
| `computerPos` | Probably `computePos` |
| `string &str` | Only read; `const string&` |
| `int` vs `size_t` | `i < str.size()` and `strptr < rdTxt.length()`. Compile with `-Wall -Wextra` and read the warnings |
| `<cstdlib>` | Needed for `exit`; currently arrives indirectly |
| `tpptr == 30000` | Works because the pointer moves by 1. What does `>=` protect against if code changes later? |
| Stale comment | `// Cast to char` on the `>` case, now an `(int)` cast |
| Banner | "brainfuck compiler" is still an interpreter |
| Comment style | Several comments describe history ("fixed this bug..."). See section 6 |

---

## 5. Test checklist

Create small `.bf` files for these and record what you observe. **Predict the result first, then run.** Use `timeout 2` for anything that might loop.

| Input | What to check |
|---|---|
| (empty file) | Behavior and exit status |
| (no file at all) | Behavior and exit status |
| `<` | Must terminate; compare with before the fix |
| `]` | Error message, exit status |
| `[` | Error message, which bracket |
| `[]]`, `[[]`, `][` | Same |
| `++++[*-***-*>**>]]` | Index in the error message |
| `-` then `.` | Cell wraps to 255; what does the trace print? |
| `+[-]` | Cell ends at 0 |
| `+++++++[>++++++++++<-]>.` | Cell 1 becomes 70, ASCII `F` |
| 30000 `>` characters | No overflow message, because nothing touches cell 30000. Why? |
| 30000 `>` then `+` | Overflow message. Why the difference? |
| Program split over two lines | Same result as one line |
| File with and without trailing newline | Same result |
| The Hello World on the Wikipedia Brainfuck page | Output text (ignore trace noise for now) |

Generating 30000 `>`: `printf '>%.0s' $(seq 30000) > input.txt` (bash).

---

## 6. Where notes and history should live

Your code comments currently serve four jobs at once: explaining the code, logging fixes, recording questions and holding to-dos. That makes the code noisy and the history hard to find. Suggested split (details in `TRACKER.md`):

| Kind of note | Where it goes |
|---|---|
| Why the code is the way it is now | A short code comment |
| "I fixed bug X by doing Y" | Git commit message and the tracker's Done log |
| "Questions I need to answer" | Tracker, "Questions" section |
| "TODO" | `// TODO(T-5)` in code, with the ID, plus the tracker |
| Design choices (tape policy, exit codes) | Tracker, "Design decisions" |

---

## 7. Suggested order for the next session

1. **BUG-1:** reproduce the hang, understand why, decide the policy (DEC-1), fix it.
2. **BUG-2 and BUG-3:** exit codes, `cerr`, and open-failure handling. Check each with `echo $?`.
3. **T-1:** the state struct and `dumpState`. This also simplifies BUG-4.
4. **CLEAN-1 / CLEAN-2:** the `continue` experiments and error positions.
5. **DESIGN-1:** compact jump table.
6. Write the tests from section 5 into a script. That finishes Stage 0.

---

## 8. Scorecard

| Area | Review #1 | Review #2 |
|---|---|---|
| Correctness of core semantics | Good | Good |
| Input handling | Weak | Good |
| Error handling | Missing | Partial |
| Robustness (bounds) | Missing | Partial, one bad bug |
| Code structure | Weak | Weak (next focus) |
| Tooling and tests | None | None (next focus) |
| Self-diagnosis and notes | Good | Good |