# Code Review #6

**Date:** 2026-11-01
**Reviewing:** `bf.cpp` after the `Tape_Data` / `instance` rename, reading straight into the struct, the BUG-7 check being commented out, and the final dump after the loop
**Previous review:** `review-05.md`

> [!NOTE]
> Same rules: questions and experiments, not ready-made fixes. IDs match `docs/TRACKER.md`. To test your latest changes I rebuilt them on a copy (rename, direct append, check removed, final dump) and ran it with `-fsanitize=address,undefined`. I did not run your exact file.

---

## 1. Verdict

A good round. **BUG-7 is resolved in behavior**: programs with comments, spaces and CRLF line endings now run and exit with status 0. The duplicate read buffer is gone, the rename did no damage this time, and you noticed something real about how your trace works (the final dump). What remains is no longer about correctness of the interpreter's core. It is **housekeeping** (stale comments, one dead variable), **tooling** (tests, the verbose flag), and **decisions** that are still unwritten.

| Area | Review #5 | Now |
|---|---|---|
| Unsupported-character check (BUG-7) | Broken | Removed; non-command characters are skipped |
| Program text | Read into a buffer, then copied | Appended straight into the struct |
| Rename side effects | Reverted | None found |
| Final state | Never printed | Printed after the loop |
| Stale comments | Same list | Same list, plus a few new ones |
| Tests | None | None |

---

## 2. Progress since review #5

| Item | Status | Notes |
|---|---|---|
| BUG-7 | Fixed | Tested; see section 6 |
| DEC-9 (skip vs reject) | Decided | "Skip, as the language says". Strict mode kept as a future idea |
| T-9 / REF-1 duplicate buffer | Done | `instance.rdtxt` is the only copy of the program text |
| CLEAN-9 names | Changed, not solved | `Tape_Data` / `instance`; see section 4 |
| CLEAN-8 stale comments | Open | New items added below |
| T-2, T-3, T-4 | Not started | |
| BUG-2 `cerr` | Open | |

---

## 3. Rename check ("change all occurrences")

You asked me to look for comments and print statements damaged by the rename. I read every comment and every string literal in the file.

| Check | Result |
|---|---|
| String literals (messages and trace labels) | All unchanged. None contains `instance` or `Tape_Data` |
| Prose comments | None damaged. "Machine model (these are fields of struct Tape_Data)" is the intended new name |
| Commented-out code | Uses the current names (`instance.rdtxt[instance.strptr]`), which is correct |
| `// instance.rdtxt = rdTxt;` | Dead code, not damage; delete it |

So **nothing needs fixing from the rename this time**. Three habits made the difference: renaming a smaller set of names, reading the result, and keeping the program's output strings free of identifiers. After any mechanical change, `git diff` still deserves a read.

---

## 4. Issues

### REF-5 (Low): the final dump

Your comment says the trace is "one index behind", so you print the state once more after the loop. That is a real observation: the trace prints the state **before** each instruction runs, so the effect of the last instruction was never shown. The final dump fixes that.

Run it and look at the last line for `+++++++[>++++++++++<-]>.`:

```
Dump State : Tape ptr : 1 String ptr : 24 Tape content : 70 String Content : 0
```

Questions:

- What is `String Content : 0` here? Is it a character of your program? What is at index `rdtxt.length()` of a `std::string`, and why is reading it allowed?
- After the loop, `strptr` is one past the last instruction. Does a field called "String Content" mean anything at that point? How could the final dump leave it out, or label it differently?
- The final dump only runs on normal termination. What state does a user see if the program exits through the overflow path or an unbalanced bracket?
- Should the final dump be part of the trace (hidden behind the verbose flag, T-2) or part of the program's normal output?

### CLEAN-11 (Low): leftovers from removing the buffer

| Where | Problem |
|---|---|
| `string rdLine, rdTxt;` | `rdTxt` is no longer used. Which line would the compiler have complained about if the variable had a simple type? (It does not complain for `std::string`, so only you will notice.) |
| "rdLine: the line just read. rdTxt: the whole program..." | Describes a variable that no longer exists |
| `NOTE: rdLine is separate from rdTxt because...` | The reason is still true, but it should now say what the program text is called |
| `// copy the rdtxt`, `// directly append to the struct`, `// instance.rdtxt = rdTxt;` | History and dead code; delete |

### CLEAN-8 (still open): stale comments

| Where | Problem |
|---|---|
| `TODO(T-1)` in the design notes and above `dumpState` | Describes finished work |
| `TODO(BUG-5)` above `argv[1]` | The `argc` check exists |
| `TODO(BUG-2)` above the overflow check | Overflow exits with 1; only the `cerr` part remains |
| `// break;` in the overflow branch | Dead code |
| "decisions to make" block | Belongs in the tracker |
| "debug liines ( remove after checks )" | Done checking |
| In `default:`: the commented-out `if` block and the whole question list above it | You answered every question in that list. Move the conclusions into the tracker and delete the comments |
| "fixed this tape into the struct" and the commented `unsigned char tape[...]` | History; the struct and git remember |

Keep the one line about a strict mode, but give it an ID (see section 8) so it lives in the tracker, not in a code comment.

### CLEAN-9 (still open): names

- `Tape_Data` holds the pointers, the **program text** and the tape. Is "tape data" what it represents? What would you call something that holds all the state a running program needs?
- `instance` says nothing, because every variable of a type is an instance. What is it an instance *of*, and what is it doing in `main`?
- C++ type names are usually written `CamelCase` and variable names `snake_case` or `camelCase`. Whichever you choose, consistency is more useful than the convention itself. Compare `Tape_Data`, `rdtxt`, `strptr`, `input_file_name` and `rdLine`.
- Use the editor's *rename symbol* for this, and read `git diff` afterwards.

### DEC-5 (needs a decision): what is an "empty" program?

You now have three kinds of "nothing" and they behave differently. I ran each:

| File | Result |
|---|---|
| 0 bytes | Error, exit 1 (the `peek()` check) |
| Only spaces | Runs, does nothing, exit 0 |
| Only a newline | Runs, does nothing, exit 0 |

Is that consistent? A program of only comments is valid Brainfuck that does nothing. Decide whether a 0-byte file should also be a valid no-op, or whether "nothing to run" is an error for all three. Then write down the reason.

### Carried over

| ID | State |
|---|---|
| DEC-1 | `<` at cell 0 is still silently ignored (`<+` leaves the cell at 1) |
| BUG-2 | Errors on `cout`, not `cerr` |
| BUG-4 | `.` prints a label and the trace into the program's output |
| T-2 | Verbose flag not started |
| CLEAN-2, 3, 4, 5, 7 | Typos in strings, `computerPos` name and `const`, `size_t` (two warnings), `<cstdlib>`, banner |
| Q-13 | `instance.tpptr = 0;` and `instance.strptr = 0;` repeat what the struct already does |

---

## 5. What you did well

1. **You followed the language, not your first idea.** You had a check you wanted; once the questions showed it conflicted with the definition of Brainfuck, you dropped it. And you kept the *intent* ("later, a strict version") instead of throwing it away.
2. **You noticed that the trace lags one step behind** and fixed it. That is the kind of observation that tells you the trace is really understood.
3. **You removed the duplicate copy of the program text** instead of leaving two sources of truth.
4. **You wrote your questions down as comments, then answered them** ("so, in bf we just skip things, we do not reject them"). Next step is to move the answers somewhere permanent and delete the comments.

---

## 6. What I ran

On a copy that reproduces your changes (compiled with `-Wall -Wextra -fsanitize=address,undefined`):

| Program | Exit status | Result |
|---|---|---|
| `+++ hello` | 0 | Final cell 3 |
| `x+` | 0 | Final cell 1 |
| `+++` with CRLF ending | 0 | Final cell 3 |
| `+++++++[>++++++++++<-]>.` | 0 | Prints `F` |
| Spaces only | 0 | No-op |
| Newline only | 0 | No-op; the final dump reads index 0 of an empty string without a problem |
| `<+` | 0 | Final cell 1 (the `<` was ignored) |
| `]` | 1 | Bracket error |

The sanitizers reported nothing. The compiler gave only the two known `int` vs `size_t` warnings.

---

## 7. Where Stage 0 stands

Done: input handling, bracket validation, tape bounds, exit statuses, argument checks, the struct and the trace function, the decision about other characters.

| Remaining | What it needs |
|---|---|
| Comments | Delete the stale list above (one commit) |
| T-3 | The test script; `+++ hello` should now **pass** |
| T-2 | Verbose flag, covering the final dump (REF-5) and BUG-4 |
| BUG-2 / DEC-10 | `cerr`, and your choice of error mechanism |
| T-4 | Step limit |
| Decisions | DEC-1, DEC-3, DEC-5, DEC-8, DEC-10 written down |
| Names | CLEAN-9 |

---

## 8. Suggested order

1. **One cleanup commit:** the CLEAN-8 and CLEAN-11 lists, nothing else.
2. **T-3:** the test script. Use the table from review #5; this time the comment program is expected to pass.
3. **T-2:** the verbose flag, with the final dump and BUG-4 in mind.
4. **BUG-2 and DEC-10:** `cerr`, then the exceptions-versus-return-values choice from review #3b.
5. **T-4:** the step limit.
6. **DEC-1, DEC-5:** write the policies down and make the code match.
7. **CLEAN-9:** names, using rename symbol.
8. **T-12 (later):** a strict mode behind a flag, validated before running.

---

## 9. Tracker updates to make

**Done log:**

| ID | What | Root cause | Lesson (write your own) |
|---|---|---|---|
| D-19 | BUG-7: non-command characters are skipped | The check contradicted the language, and its logic and index were also wrong | |
| D-20 | Program text read directly into the struct | A second buffer and a copy | |
| D-21 | Final state printed after the loop | The trace shows state before each instruction, so the last one was never shown | |

**Update:**

- BUG-7: closed.
- DEC-9: decided: skip other characters.
- T-9: done.
- BUG-6: superseded by DEC-5 (see section 4).
- CLEAN-9: names changed, still unclear.

**Add as open:**

| ID | Item |
|---|---|
| T-12 | Optional strict mode behind a flag, checked before execution |
| REF-5 | Final dump: meaning of `String Content`, abnormal exits, verbose flag |
| CLEAN-11 | Remove the unused `rdTxt` and the comments that describe it |

---

## 10. Scorecard

| Area | #1 | #2 | #3 | #4 | #5 | #6 |
|---|---|---|---|---|---|---|
| Core semantics | Good | Good | Good | Good | Good | Good |
| Input handling | Weak | Good | Good | Good | Good | Good |
| Error handling | Missing | Partial | Mostly there | One broken check | One broken check | Good (cerr still open) |
| Robustness | Missing | Partial | Better | Same | Same | Better |
| Code structure | Weak | Weak | Weak | Improving | Good | Good |
| Tooling and tests | None | None | None | None | None | None (next focus) |
| Self-diagnosis and notes | Good | Good | Very good | Good | Good | Very good |
