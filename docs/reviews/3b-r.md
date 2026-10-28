# Code Review #3b

**Date:** 2026-10-28
**Reviewing:** `bf.cpp` after the `argc` check, the overflow `exit(1)` and the new unsupported-character check
**Previous review:** `review-03.md`

> [!NOTE]
> Same rules: questions and experiments, not ready-made fixes. IDs match `docs/TRACKER.md`. This review also answers your question about exception handling (section 5).

---

## 1. Verdict

Two real fixes (the `argc` crash and the overflow exit status), and one **new serious bug** (BUG-7) in the unsupported-character check you just added. The instinct behind that check is good (validate your input), but as written it rejects *every* program that contains a comment or a space. I ran it to confirm; the experiments below let you reproduce it yourself.

| Area | Review #3 | Now |
|---|---|---|
| No-argument crash (BUG-5) | Crashes | Fixed, message and exit status 1 |
| Tape overflow status | `return 0` | `exit(1)` |
| Unsupported characters | Skipped as comments | **Every non-command character is an error (BUG-7)** |
| Error output stream | `cout` | `cout` (unchanged) |
| Trace function (T-1) | Not started | Next, per your plan |
| Comment hygiene | Clean | Stale TODOs and leftover code again |

---

## 2. Progress since review #3

| Item | Status | Notes |
|---|---|---|
| BUG-5 `argc` not checked | Fixed | See "small things" for what is still open (usage line, extra arguments) |
| BUG-2 overflow exit status | Fixed | `exit(1)`. Messages still on `cout` |
| BUG-2 `cerr` | Open | Waiting on your stdout/stderr experiments |
| BUG-6 whitespace-only file | Changed by accident | A spaces-only file now errors, but only because of BUG-7 |
| DEC-8 extra arguments | Half decided | Your comment says "version 1: single input". Write down what happens to extras |
| T-1 / T-2 | Next | |

---

## 3. What you did well

1. **You wrote the reason down** when you replaced `break` with `exit(1)` ("I need to exit the program, so I will use exit code"). That is a decision with its justification, which is what makes the tracker useful later.
2. **You tested for the missing-argument case** and wrote a message for it.
3. **You thought of a class of bad input** (unsupported characters) that I did not list. Validating input is the right instinct; BUG-7 is about *where* and *how*, not whether.
4. **You listed decisions to make** ("handling multiple files?", "verbose flag", "other command line args") before coding them. See section 7 for where those notes should live.

---

## 4. Issues

### BUG-7 (High): the unsupported-character check is wrong in four ways

**Where:** the `default:` case of the `switch`.

```cpp
if(rdTxt[strptr] != '\t' || rdTxt[strptr] != '\n' || rdTxt[strptr] != '\0'){
    cout << "[ERROR] Unsupported Character : " ...
    exit(1);
}
```

**Experiments** (use `printf` to create the files, and check `echo $?`):

```bash
printf 'x+'        > t1.bf   # a comment character, then a valid command
printf '+++ hello' > t2.bf   # a program with a comment
printf '+++\r\n'   > t3.bf   # Windows line ending
```

Run all three. What does each print? What exit status?

**Problem 1: the condition is always true.**

- Write a truth table for the three comparisons with the character `'a'`, then `'\t'`, then `'\n'`. Is there *any* character for which the whole `||` expression is false?
- A single character cannot equal two different values at once. So if you want to say "none of these three", which operator joins the comparisons? Look up **De Morgan's laws** and rewrite the sentence "it is not (A or B or C)" in terms of `!=`.

**Problem 2: it checks the wrong character.**

- Look at the order of the statements. When is `strptr++` executed relative to the check?
- In experiment 1 (`x+`), which character does the error message *name*, and is that the character that was unsupported?
- What does `rdTxt[rdTxt.length()]` give you for a `std::string`? (Experiment 3 prints something odd; what is it?)

**Problem 3: it contradicts the language.** Brainfuck treats every character that is not one of the eight commands as a *comment*. Check:

- your own comment in `default:` ("any other character is skipped"),
- the message printed just above the check ("Skipping other instructions..."),
- the README's description of the language.

All of them say "skip". If you really want a strict mode, treat it as an optional feature, not the default. Record the decision as DEC-9.

**Problem 4: it is in the wrong layer.** Think about a program that prints some output and then hits an unsupported character halfway through.

- What has the user already seen by the time the error appears?
- Which phase of your program is the right place to reject a bad program: *before* execution starts (like your bracket check) or in the middle of it?
- Which of the three characters in your condition can even reach `rdTxt` at all, given that `getline` strips newlines? (Experiment 3 shows what *can* appear.)

### Stale or leftover comments (CLEAN-8)

| Where | Problem |
|---|---|
| `TODO(BUG-5)` block above `argv[1]` | Says `argc` is never checked; it is now |
| `TODO(BUG-2)` above the overflow check | Says the program ends with `return 0`; it no longer does. Only the `cerr` part is still true |
| `// break;` in the overflow branch | Commented-out code; delete it (git remembers) |
| "debug liines ( remove after checks )" and its commented `cout` | Remove when you fix BUG-7 |
| "decisions to make..." block | Move to the tracker (DEC-8, T-2) |

**Habit:** when you fix something, delete its `TODO(ID)` in the same commit. A stale TODO is worse than none, because it sends you looking for a bug that is gone.

### Small things

| Item | Note |
|---|---|
| Missing-argument message | Goes to `cout`, and has no usage line. Many tools print `usage: ./bf <program.bf>` |
| Extra arguments | Currently ignored silently. Fine if you write it down (DEC-8) |
| Error stream | All error messages are still on `cout` (BUG-2, section 5 of review #3) |
| Carried over | `<cstdlib>`, `size_t` comparisons, `computerPos` name, "paranthesis", "Arguement", banner, unreachable `return;` |

---

## 5. Your question: can I use exception handling for errors?

**Short answer:** yes. It is a legitimate, idiomatic C++ way to report errors, and it fits several of yours well. But "can I" is the smaller question. The better ones are *which* errors, *where* they should be handled, and *what the alternatives cost*.

### 5.1 What you have now

Every error path does the same two things inline: print a message, call `exit(1)`. Think about what that means:

- The code that *detects* the problem also decides to *kill the whole process*. A helper like `computerPos` cannot be reused or tested without ending the test run.
- Messages are formatted and printed in a dozen places, so there is no single place to switch them to `cerr`.
- `exit()` does not run the destructors of local objects (the Q-9 question from review #3). Look up what it *does* do.

### 5.2 What exceptions are

An exception separates **detecting** an error from **handling** it:

```cpp
#include <iostream>
#include <stdexcept>
#include <string>

struct ConfigError : std::runtime_error {      // your own error type
    using std::runtime_error::runtime_error;
};

int parsePort(const std::string& s) {          // detects, does not decide what to do
    int p = std::stoi(s);                      // may throw by itself
    if (p < 1 || p > 65535)
        throw ConfigError("port out of range: " + s);
    return p;
}

int main() {
    try {
        std::cout << parsePort("99999") << "\n";
    } catch (const ConfigError& e) {           // handles one place, one policy
        std::cerr << "config error: " << e.what() << "\n";
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "unexpected: " << e.what() << "\n";
        return 1;
    }
}
```

(This is a generic example, not your code. Mapping it to your project is the exercise.)

Questions to chew on:

- When `throw` runs, what happens to the local objects in every function between the `throw` and the `catch`? (Search **stack unwinding** and **RAII**.) How does that differ from `exit()`?
- Why `catch (const X& e)` and not `catch (X e)`?
- What happens if **nobody** catches an exception? (Search `std::terminate`. On Linux the process typically dies with status 134. Try it.)
- Why derive your error type from `std::runtime_error` or `std::exception`?

### 5.3 The alternatives

| Mechanism | Good for | Costs |
|---|---|---|
| **`exit()` in place** | Tiny scripts | No cleanup, hard to test, scattered policy |
| **Return codes / `bool` / enum** | Expected, frequent, locally handled failures | Easy to forget to check; error details need an out-parameter |
| **Return a value-or-error type** (`std::optional`, `std::expected` in C++23, `llvm::Expected<T>`) | Errors that are part of normal results, plus details | More verbose; the caller must unpack |
| **Exceptions** | Rare, serious errors found deep in a call chain, with the handler far away | Easy to miss a `catch`; unsafe in code that is not exception-safe; some projects forbid them |

### 5.4 Which of your errors fit what?

Fill in the last two columns yourself. The first three are given to get you started.

| Error | Category | Detected in | Who should handle it? | Mechanism? |
|---|---|---|---|---|
| No file argument | usage | `main` | | |
| File missing or unreadable | input | `main` | | |
| Empty program | input | `main` | | |
| Unmatched bracket | program (static) | `computerPos` | | |
| Tape pointer past the end | runtime | the execute loop | | |
| `<` at cell 0 (if you choose "error") | runtime | the execute loop | | |

Hints:

- A usage error is detected exactly where it is handled. Does it need to travel anywhere?
- An unmatched bracket is found in a helper but must be reported to the *user* by whoever called it. What carries the message and the **position** (CLEAN-2) back up?
- Does it make sense to use exceptions for ordinary control flow such as `[` and `]`? Think about how often those run versus how often a fatal error does.

### 5.5 What a good design looks like (questions, not code)

1. How many places should call `exit()`, or return from `main`, after you finish? (One is a good target.)
2. If helper functions only *report* problems, where do you print them, and to which stream?
3. How would different exception types let `main` choose different exit statuses for "bad usage", "bad program" and "runtime error"? (That revisits DEC-3.)
4. Where would a `catch (...)` safety net go, and why would you want one?
5. How would you test that a function *throws*, compared with how you would test one that calls `exit()`?

### 5.6 Two things to know before you commit to it

- **Don't throw in the hot path for normal things.** Throwing is much slower than a return, so use it for failures that stop the run, not for routine control flow inside your execute loop.
- **LLVM and MLIR do not use C++ exceptions.** LLVM's coding standards avoid them, and you will meet `llvm::Error`, `llvm::Expected<T>` and MLIR's `LogicalResult` instead. So the useful plan is to use exceptions here to learn the mechanism and RAII, then compare with the value-or-error style when you reach Stage 7. Neither choice is wrong; knowing both is the goal.

### 5.7 Suggested way to try it

- Do **T-1 (the trace function) first**, as you planned. Change one thing at a time, so each step can be tested and committed on its own.
- Then pick **one** error (for example, tape overflow) and convert it. Compare the two versions: the exit status, the stream, and what happens to the destructors.
- Decide per category (5.4) before converting the rest. Record it as DEC-10.

---

## 6. Before you start T-1 (the trace function)

The refactor should not change behavior, which makes it easy to check:

1. **Capture the current output first.** For a few programs, including a loop and a bad one:

   ```bash
   ./bf prog.bf > before_prog.txt
   ```

2. Do the refactor.
3. **Compare:** `./bf prog.bf > after_prog.txt && diff before_prog.txt after_prog.txt`. No difference means the trace is identical.
4. Decisions to make before typing: what goes in the state struct? Does `encWrt` belong to the trace too? Does `cnt` belong in the struct? Is `rdTxt` machine state or the program (DEC-7)? Should `dumpState` take `const&`, and why?
5. Fix BUG-7 **before** you start: its logic lives in `default:`, which is one of the blocks you are about to rewrite.

---

## 7. Where the decisions in your comments belong

| Your note | Where it goes |
|---|---|
| "are we handling multiple files?" | DEC-8 in the tracker |
| "will add verbose flag and other command line args" | T-2 in the tracker |
| "here I am breaking the loop but I need to exit" | Already captured in the Done log (lesson column) |
| "debug lines (remove after checks)" | Delete when the check is fixed |

---

## 8. Test checklist (additions)

| Input | What to check |
|---|---|
| `x+` | Which character does the message name? (BUG-7) |
| `+++ hello` | A program with a comment: should it run? |
| File with CRLF line endings | What character reaches `default:`? |
| `./bf` (no args) | Message, status 1 (fixed) |
| `./bf prog.bf extra.bf` | Extras ignored? Recorded? |
| 30000 `>` then `+` | Overflow, status 1 (verify) |
| Any bad input with `2>/dev/null` | Does the error vanish? (If so it is on `cerr`; if not, it is on `cout`.) |

---

## 9. Suggested order

1. **BUG-7:** reproduce, then decide DEC-9 (comments or strict), then fix it in the right layer.
2. **CLEAN-8:** delete the stale TODOs and leftover code.
3. **T-1:** the trace function, with the before/after `diff`.
4. **T-2:** verbose flag.
5. **Error design:** fill in the table in 5.4, then try exceptions on one error (DEC-10).
6. **T-3 / T-4:** the test script and step limit.

---

## 10. Tracker updates to make

**Done log:**

| ID | What | Root cause | Lesson (write your own) |
|---|---|---|---|
| D-13 | BUG-5 `argc` check | Used `argv[1]` unchecked | |
| D-14 | Overflow exits with status 1 | `break` ended the program with `return 0` | |

**Add as open:**

| ID | Item |
|---|---|
| BUG-7 | Unsupported-character check: always true, wrong index, contradicts the language, wrong layer |
| DEC-9 | Non-command characters: comments (standard) or errors (strict mode)? |
| DEC-10 | Error-handling mechanism per error category: `exit()`, return values or exceptions |
| CLEAN-8 | Remove stale TODOs and commented-out code; delete a TODO in the same commit that fixes it |
| Q-10 | De Morgan's laws; `||` vs `&&` in "none of these" conditions |
| Q-11 | Exceptions: stack unwinding, RAII, and how `exit()` differs |

**Update:**

- BUG-2: only the `cerr` part remains.
- BUG-5: closed; DEC-8 still needs a written decision.
- BUG-6: re-test after BUG-7 is fixed; a spaces-only file currently errors only because of BUG-7.
- T-1: in progress.