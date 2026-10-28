# Brainfuck Interpreter: Code Review and Learning Roadmap

A review of your C++ brainfuck (BF) interpreter, followed by a detailed path from here to a real optimizing compiler with an LLVM backend. This is written as pointers and questions, not fixes, so the debugging and design work stays yours.

---

## 1. Summary

**Verdict:** a solid start. The core ideas are right: a tape, two pointers, a precomputed jump table, a `switch` dispatch loop, and a state trace to debug with. The bugs left are the kind every first interpreter has: input handling, bounds, and error handling.

| Area | Status |
|---|---|
| Bracket matching (stack) | Correct, handles nesting |
| Jump semantics | Correct and well reasoned |
| Tape cell type | Good choice (`unsigned char`) |
| File input | Likely buggy, check first |
| Bounds / error handling | Missing |
| Code structure | Everything in `main`, lots of duplication |
| Naming ("compiler") | It is an interpreter, see section 4 |

---

## 2. What you are doing well

1. **Stack-based bracket matching is correct**, including nested loops. Resolving jump targets *before* execution is your first compiler-style pre-pass.
2. **Your jump reasoning holds up.** Landing just after the matching bracket is equivalent to re-testing at the bracket, because the cell cannot change between the test and the landing point. One nit: the Wikipedia formulation of `]` already says "jump back to the command *after* the matching `[`". Older formulations jump to the `[` itself. Both exist, and yours is fine.
3. **You found real bugs by reasoning.** The `tpptr` vs `strptr` key mix-up and the `revpos[strptr]++` mutation were both caught and documented.
4. **`unsigned char` tape** gives you well-defined wraparound (`0 - 1` becomes `255`, `255 + 1` becomes `0`), which BF programs rely on.
5. **State-dump tracing** is the right instinct. Interpreters are miserable to debug without it.
6. **You write down what you learned** in comments. Keep doing that.

---

## 3. Issues to investigate

Each item is a question to answer by experiment. Fix them in roughly this order, since the first three are correctness bugs.

### 3.1 File reading (check this first)

- What does `rdTxt` contain **after** the `while (getline(file, rdTxt))` loop finishes?
- `std::getline` clears the string before it tries to extract anything. What does that imply for the last, failing call, especially if your file ends with a newline?
- What happens to earlier lines of a multi-line file?
- What happens if `input.txt` does not exist?

**Experiment:** print `rdTxt.size()` right after the loop, once with a trailing newline in the file and once without.

### 3.2 Unbalanced brackets

- What does `computerPos` do with a stray `]`? What does it do with a `[` that never closes?
- What does `operator[]` on an `unordered_map` do when the key is missing? What would the interpreter then do at runtime?
- How *should* an interpreter report a bad program? (Think: message, position, exit code.)

### 3.3 Tape pointer bounds

- What happens when `tpptr` reaches `-1`?
- You guard the upper end in the `while` condition. Should "the tape pointer left the tape" really be what ends your program loop? What should end it?
- Decide your policy and make it deliberate: error, wrap around, or grow the tape. Each is a legitimate design choice, and real BF implementations differ.

### 3.4 Do you need two maps?

Your note says `unordered_map` has no two-way lookup. Ask:

- Can one code position be both a `[` and a `]`?
- Are your keys arbitrary, or are they something denser and more regular?
- What would a more compact data structure look like?

### 3.5 Duplicated trace code

The same five-line dump appears about ten times, and you already left a `// void dumpState()` stub. Also check the `>` case: it casts to `(char)` while all the others use `(int)`, probably a leftover.

### 3.6 Debug output mixed with program output

`.` currently prints a label in the middle of trace noise. A real BF program's output is just its characters. How can you keep program output and trace output separable? Options to think about: a verbose flag, writing the trace to `stderr`, or a separate trace function.

### 3.7 Your `unsigned char` TODO

Ideas to chew on, not answers to copy:

- `cout << some_unsigned_char` picks the **character** overload. That is why your numeric dumps need a cast.
- Plain `char` may be **signed** on your platform. What does `(int)` of a signed `char` holding byte `0xC3` give you? Why does `(int)(unsigned char)` avoid that?
- Integer promotion: what type does `tape[i] + 1` have *before* it is stored back?

### 3.8 Smaller things

- `computerPos` is probably meant to be `computePos`.
- Pass the string as `const string&`, since it is only read.
- You compare `int` with `size_t` in a few places (compiler warnings will show this; compile with `-Wall -Wextra`).
- Add `#include <string>` instead of relying on a transitive include.
- Hard-coded `input.txt`: take the filename from `argv`.
- `,` is unimplemented (fine for phase 1, but plan for it).

---

## 4. Concept notes

### 4.1 Interpreter vs compiler

| | Interpreter | Compiler |
|---|---|---|
| What it does | Walks the program and *performs* it | *Translates* it into another form |
| Output | The program's behaviour | A new program (C, assembly, LLVM IR, machine code) |
| Yours today | Yes | Not yet |

Your banner says "compiler", but today this is an interpreter. That is not a criticism; it is the right first step, and the roadmap below turns it into a compiler.

### 4.2 Your interpreter is a tiny virtual machine

| In your code | Architecture name |
|---|---|
| `strptr` | Program counter (PC) / instruction pointer |
| `tpptr` | Data pointer (like an index register) |
| `tape[30000]` | Data memory |
| the `switch` | Fetch, decode, execute |
| `positions` / `revpos` | Resolved branch targets |
| the pre-pass | Jump resolution (like an assembler's label resolution) |

Keep this mapping in mind. It is the bridge between "interpreter project" and "how a CPU works".

### 4.3 BF grammar (for when you write a real parser)

```
program := ( command | loop )*
loop    := '[' program ']'
command := '+' | '-' | '<' | '>' | '.' | ','
```

Everything else is a comment. This tiny recursive grammar is a perfect first exposure to recursive-descent parsing.

---

## 5. Roadmap: from interpreter to optimizing compiler

Each stage has a **goal**, **what to build**, **what you will learn**, and **how to check you got it right**. Do them in order; every stage reuses the previous one.

| Stage | Title | Core idea |
|---|---|---|
| 0 | Harden | Fix section 3 |
| 1 | Separate phases | load, analyze, execute |
| 2 | Introduce an IR | text becomes instruction structs |
| 3 | Optimize the IR | peephole passes |
| 4 | Fast interpreter | dispatch and microarchitecture |
| 5 | Backend: emit C | first real compiler |
| 6 | Backend: assembly | registers, ISA, ABI |
| 7 | Backend: LLVM IR | SSA, basic blocks, `opt`, `llc` |
| 8 | JIT (optional) | run compiled code in-process |
| 9 | MLIR dialect (optional) | structured IR with regions |

### Stage 0: Harden the interpreter

**Goal:** a correct, robust interpreter before you build anything on top of it.

**Build**
- Fix everything in section 3.
- Create a `tests/` folder with a few BF programs and their expected output: a hello-world, a program that exercises nested loops, a cell-wraparound test, and a few deliberately **malformed** programs (unbalanced brackets, pointer underflow).
- Add a **step limit** option so a buggy infinite loop cannot hang your terminal.

**Learn:** error handling policy, test-driven development of a language implementation.

**Check:** a small shell script runs every test program and diffs the output against the expected file.

### Stage 1: Separate the phases

**Goal:** stop having `main` do everything.

**Build** three clearly separated pieces:

1. **Load**: read the source from a file named on the command line. Decide how multi-line files work.
2. **Analyze**: validate brackets and compute jump targets. Report errors with **line and column**.
3. **Execute**: run the program.

Also implement `,` (input), and make the trace a flag, not always-on.

**Learn:** the classic front-end / middle / back-end split in miniature. Good diagnostics (line and column) are what separate a toy from a tool.

**Check:** unbalanced-bracket tests now print a useful message pointing at the offending bracket.

### Stage 2: Introduce an intermediate representation (IR)

**Goal:** stop executing *text*; execute a *data structure* produced from the text.

**Build**
- Define an instruction type, roughly an opcode plus one integer argument.
- Write a **lowering** function: source text becomes `vector<Instr>`. Jump targets live in the argument field.
- Rewrite the execute loop to run the IR. Your `strptr` becomes a true program counter.
- Add a `--dump-ir` option that prints the IR as a readable listing (a disassembler).

**Stretch (strongly recommended):** also build a **tree** version using recursive descent over the grammar in 4.3, where a loop node owns its body. Now you have seen the two classic IR shapes:

| Shape | Control flow looks like | Compiler analogue |
|---|---|---|
| Flat bytecode | Jumps to indices | Assembly, bytecode, CFG after flattening |
| Tree (AST) | Nested loop nodes | AST, structured IR (like MLIR regions) |

**Learn:** lexing/parsing, why compilers use IRs, the difference between a tree and a linear IR.

**Check:** the IR interpreter gives byte-for-byte identical output to your Stage 0 interpreter on every test.

### Stage 3: Optimize the IR

**Goal:** make a pass pipeline and prove each pass preserves behaviour.

Build optimizations as separate passes over the IR, each runnable on its own, in roughly this order of difficulty:

1. **Run-length folding.** `+++++` becomes one `add 5`; `>>>` becomes one `move 3`. Handle `-` and `<` by folding them into negative arguments. Decide what happens to `+-` (cancel to nothing).
2. **Clear loop.** `[-]` (and `[+]`) sets the cell to zero. Replace the loop with a single `set 0`.
3. **Deferred pointer movement.** Instead of moving the data pointer on every `>`, track a running offset and give instructions an offset field, so `>+<` becomes `add 1 @offset+1` with no net pointer move.
4. **Scan loops.** `[>]` and `[<]` scan for the next zero cell. Replace them with a single scan instruction.
5. **Multiply / copy loops.** `[->+<]` adds the current cell to the next one, then zeroes it. `[->++>+++<<]` is a multiply-add. Recognize loops that (a) end with the pointer where they started, (b) decrement the loop cell by exactly 1 per iteration, and (c) contain only add instructions. Replace them with multiply-adds and a `set 0`.
6. **Known-zero tracking (stretch).** At program start, every cell is zero, so a loop that is entered when its cell is provably zero is dead code. This is a small taste of **abstract interpretation**.

**Learn:** peephole optimization, pattern matching on IR, pass pipelines, strength reduction, why optimizations must preserve semantics (including 8-bit wraparound and pointer behaviour).

**Check (important habit):** **differential testing**. For every test program, assert that unoptimized and optimized runs produce identical output. Add an `--opt-level` flag so you can switch passes off to bisect a bug. Keep a timing table, and watch how much each pass buys.

**Pitfall:** in the multiply-loop recognizer, be careful that the pointer is balanced and that the loop body has no I/O or nested loops.

### Stage 4: Make the interpreter fast (architecture lessons)

**Goal:** see why interpreters are slow on real hardware and how to fix it.

**Build**
- Benchmark with a long-running BF program (community Mandelbrot and similar benchmarks are widely available). Record the time at `-O0` and `-O2`.
- Look at the assembly of your dispatch loop (`-S`, or Compiler Explorer). Find the indirect jump of the `switch`.
- Try **direct-threaded dispatch** using computed goto (labels-as-values, a GCC/Clang extension), so each instruction ends with its own indirect jump.
- Measure with `perf stat` (Linux): instructions, cycles, branches, branch-misses.

**Learn:**

- **Branch prediction:** one shared indirect jump is hard to predict; replicating it per opcode gives the predictor more context.
- **Pipelining and CPI:** why more instructions can still be faster.
- **Memory hierarchy:** your 30 KB tape fits in L1 data cache, which is part of why BF runs reasonably. Try larger tapes and see what changes.

**Check:** the faster interpreter still passes all tests, and you can explain the speedup using counter numbers, not a guess.

### Stage 5: Backend 1, emit C

**Goal:** write your first real compiler: BF in, a C program out.

**Build** a code generator that walks your IR and prints C:

| IR | C |
|---|---|
| prologue | `unsigned char tape[30000]; unsigned char *p = tape;` |
| `add n` | `*p += n;` |
| `move n` | `p += n;` |
| `[` | `while (*p) {` |
| `]` | `}` |
| `.` / `,` | `putchar(*p);` / `*p = getchar();` |

Then compile the output with `gcc -O2` or `clang -O2` and time it against your interpreter.

**Learn:** code generation, the compile / assemble / link pipeline, and a humbling lesson: the C compiler will optimize your generated code further. Look at the assembly it produces and compare it with what *your* passes did.

**Check:** compile every test program through the C backend and diff against expected output, using the same test harness.

### Stage 6: Backend 2, emit assembly

**Goal:** go one level lower and see the machine.

Pick the ISA of your own machine (x86-64 or AArch64) and emit a `.s` file, then assemble and link it with `gcc` or `clang`.

**Sketch for x86-64** (Intel syntax):

| IR | Assembly idea |
|---|---|
| tape | a 30000-byte zeroed array in `.bss` |
| data pointer | kept in a callee-saved register such as `rbx` |
| `add n` | `add byte ptr [rbx], n` (8-bit wraparound is free) |
| `move n` | `add rbx, n` |
| `[` | `cmp byte ptr [rbx], 0` then `je` to a unique end label, with a unique begin label before it |
| `]` | `cmp byte ptr [rbx], 0` then `jne` back to the begin label, with the end label after it |
| `.` | zero-extend the cell into `edi` and `call putchar` |

Each loop needs **unique label names**. Use a counter and your bracket stack.

**Learn:** registers, addressing modes, flags and conditional jumps, the calling convention (which registers are preserved across calls, stack alignment before a `call`), and the difference between a section like `.bss` and `.text`.

**Check:** same test harness. When something crashes, learn to use `gdb` or `lldb` and step through instructions.

### Stage 7: Backend 3, LLVM IR

**Goal:** target LLVM and understand what it gives you.

This stage is the one most directly connected to your LLVM interest, so it has sub-steps.

**7a. Write LLVM IR by hand first.** Write the `.ll` for a few tiny BF programs yourself. Run them with `lli`, compile with `llc`, and optimize with `opt -O2 -S`. Reading and writing textual IR comes before using any API.

**7b. Understand the mapping.**

| BF concept | LLVM IR concept |
|---|---|
| tape | a global or `alloca` array of `i8` |
| data pointer | an `alloca`'d index or pointer variable (let `mem2reg` promote it later) |
| `+` | `load i8`, `add i8`, `store i8` through a `getelementptr` |
| `>` / `<` | add to the pointer variable (GEP or integer add) |
| `[ ... ]` | three basic blocks: loop **header** (load cell, `icmp eq 0`, conditional `br`), **body**, and **exit**; the end of the body branches back to the header |
| `.` / `,` | `call` to declared `putchar` / `getchar` |

Your bracket **stack** from the interpreter reappears here: when you see `[` you create blocks and push them; when you see `]` you pop them and wire up the branches. Remember that every basic block must end in exactly one terminator.

**7c. Emit textual IR from C++** (just print strings, as in Stage 5). Run it through `opt` and `llc`.

**7d. Use the LLVM C++ API** (`LLVMContext`, `Module`, `IRBuilder`). Same output, built programmatically.

**7e. Study what the optimizer does.**

- Compare `-O0` and `-O2` output for the same BF program.
- Run `opt -passes=mem2reg` and watch the `alloca`s turn into SSA values and **phi nodes**.
- Use `-print-after-all` to see which pass changed what.
- Check whether LLVM independently discovers your Stage 3 optimizations (clear loops, multiply loops). Where it does not, ask why: what does it know and not know about your 8-bit cells and your tape?

**Learn:** SSA form, basic blocks and terminators, `alloca` plus `mem2reg`, GEP, phi nodes, the `opt` pass pipeline, target selection with `llc`, and how a real compiler backend divides the work.

**Check:** the same test harness, now with a third backend. Compare running times across interpreter, C, assembly, and LLVM outputs.

### Stage 8 (optional): JIT

**Goal:** compile and run BF inside your own process.

Use LLVM's ORC JIT (`LLJIT`): build the module in memory, add it to the JIT, look up the entry symbol, and call it like a function pointer.

**Learn:** the difference between ahead-of-time and just-in-time compilation, symbol resolution, and executable memory management.

### Stage 9 (optional capstone): an MLIR dialect

**Goal:** model BF as a structured IR and lower it through a pipeline.

**Idea:** define a small BF dialect with operations for add, move, input, output, and a loop operation that **owns a region** (a structured analogue of Stage 2's tree IR). Write your Stage 3 optimizations as MLIR rewrite patterns, then lower the dialect step by step toward the LLVM dialect and finally to LLVM IR.

**Learn:** dialects, operations, regions, pattern rewriting, dialect conversion, and why multi-level IRs exist.

---

## 6. Parallel track: computer architecture

Do these alongside the stages above, not after them.

| Activity | Why |
|---|---|
| Compile small C functions with `-O0` and `-O2` in Compiler Explorer and read the assembly | Builds fluency in your ISA |
| Learn the calling convention of your platform | Needed in Stage 6 |
| Time the interpreter with different tape sizes and access patterns | Cache behaviour |
| Use `perf stat` before and after each optimization | Ties a source change to hardware counters |
| Design a tiny ISA for a BF-like machine and write an emulator for it | Fetch/decode/execute from the other side |
| Optional: emit RISC-V assembly and run it under an emulator such as QEMU | A clean, simple ISA to study |

---

## 7. Testing strategy (use it from Stage 0 onward)

1. **Golden tests:** each program has an expected output file.
2. **Differential testing:** the same program through every pipeline (interpreter, optimized interpreter, C, assembly, LLVM) must agree.
3. **Error tests:** malformed programs must fail with a specific message and a nonzero exit code.
4. **Step limits:** so non-terminating programs cannot hang the test run.
5. **Sanitizers:** build with `-fsanitize=address,undefined` during development; they will catch tape out-of-bounds and signed/unsigned mistakes immediately.

---

## 8. Reading list

| Resource | Use it for |
|---|---|
| *Crafting Interpreters* (Robert Nystrom, free online) | Parsing, bytecode VMs, interpreter design |
| LLVM "Kaleidoscope" tutorial | Your first IRBuilder-based compiler |
| LLVM Language Reference (LangRef) | The authoritative IR reference |
| Compiler Explorer (godbolt.org) | Seeing assembly and LLVM IR instantly |
| *Computer Organization and Design* (Patterson and Hennessy) | Architecture fundamentals |
| *Computer Systems: A Programmer's Perspective* (Bryant and O'Hallaron) | How C maps to machine code and memory |
| *Engineering a Compiler* (Cooper and Torczon) | IRs, SSA, optimization, register allocation |
| MLIR documentation and tutorials (Toy language) | Stage 9 |

---

## 9. Checklist

- [ ] Stage 0: file reading fixed, bounds policy decided, unbalanced brackets reported, test suite exists
- [ ] Stage 1: load / analyze / execute separated, `,` implemented, trace behind a flag
- [ ] Stage 2: IR defined, `--dump-ir` works, tree IR built (stretch)
- [ ] Stage 3: at least run-length folding and clear-loop passes, differential tests passing
- [ ] Stage 4: benchmarked, computed-goto dispatch tried, `perf stat` numbers explained
- [ ] Stage 5: C backend working
- [ ] Stage 6: assembly backend working
- [ ] Stage 7: hand-written `.ll`, textual IR emission, IRBuilder version, optimizer comparison
- [ ] Stage 8 / 9: JIT or MLIR dialect (optional)

Start with section 3.1. It is the quickest fix that will change how your program behaves.