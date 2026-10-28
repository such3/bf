# Roadmap: from interpreter to optimizing compiler

The long-term plan. Day-to-day bugs and to-dos live in `TRACKER.md`; dated code reviews live in `reviews/`.

**Current stage:** 0 (hardening)

| Stage | Title | Core idea | State |
|---|---|---|---|
| 0 | Harden | Fix known bugs, add tests | In progress |
| 1 | Separate phases | Load, analyze, execute | |
| 2 | Introduce an IR | Text becomes instruction structs | |
| 3 | Optimize the IR | Peephole passes | |
| 4 | Fast interpreter | Dispatch and microarchitecture | |
| 5 | Backend: emit C | First real compiler | |
| 6 | Backend: assembly | Registers, ISA, ABI | |
| 7 | Backend: LLVM IR | SSA, basic blocks, `opt`, `llc` | |
| 8 | JIT (optional) | Run compiled code in-process | |
| 9 | MLIR dialect (optional) | Structured IR with regions | |

Estimated effort for the core path (stages 0 to 7): roughly 65 to 130 focused hours. A shorter path to LLVM IR (stages 0, 1, 2, the first two passes of 3, 5, then 7a to 7c) is about 40 to 70 hours.

---

## Concept notes

### Interpreter vs compiler

| | Interpreter | Compiler |
|---|---|---|
| What it does | Walks the program and performs it | Translates it into another form |
| Output | The program's behaviour | A new program (C, assembly, LLVM IR) |
| This project today | Yes | Not yet |

### The interpreter is a tiny virtual machine

| In the code | Architecture name |
|---|---|
| `strptr` | Program counter (PC) |
| `tpptr` | Data pointer (like an index register) |
| `tape[30000]` | Data memory |
| the `switch` | Fetch, decode, execute |
| `positions` / `revpos` | Resolved branch targets |
| the bracket pre-pass | Jump resolution, like an assembler resolving labels |

### BF grammar

```
program := ( command | loop )*
loop    := '[' program ']'
command := '+' | '-' | '<' | '>' | '.' | ','
```

Everything else is a comment. A natural first exercise in recursive-descent parsing.

---

## Stage 0: Harden the interpreter

- **Goal:** a correct, robust interpreter before building on it.
- **Build:**
  - Fix the open bugs in `TRACKER.md`.
  - Create `tests/` with a hello-world, nested loops, a cell-wraparound test and several deliberately malformed programs.
  - Add a step limit so a runaway loop cannot hang the terminal.
- **Learn:** error-handling policy, exit codes, test-driven development of a language implementation.
- **Check:** a script runs every test program and diffs output **and exit status** against expectations.

## Stage 1: Separate the phases

- **Goal:** stop having `main` do everything.
- **Build:** three separate pieces: **load** (read from a file named on the command line), **analyze** (validate brackets, compute jump targets, report errors with line and column), **execute**. Also implement `,` and put the trace behind a flag.
- **Learn:** the front end / middle / back end split in miniature; good diagnostics.
- **Check:** unbalanced-bracket tests print a useful message pointing at the offending bracket.

## Stage 2: Introduce an IR

- **Goal:** execute a data structure produced from the text, not the text itself.
- **Build:**
  - An instruction type (opcode plus one integer argument).
  - A lowering function from source text to `vector<Instr>`, with jump targets in the argument.
  - An execute loop over the IR (`strptr` becomes a true program counter).
  - A `--dump-ir` option (a disassembler).
  - **Stretch:** also build a tree version with recursive descent, where a loop node owns its body.

| Shape | Control flow looks like | Compiler analogue |
|---|---|---|
| Flat bytecode | Jumps to indices | Assembly, bytecode, a flattened CFG |
| Tree | Nested loop nodes | AST, structured IR like MLIR regions |

- **Learn:** lexing and parsing, why compilers use IRs, tree vs linear IR.
- **Check:** identical output to the Stage 0 interpreter on every test.

## Stage 3: Optimize the IR

- **Goal:** build a pass pipeline and show each pass preserves behaviour.
- **Build** separate passes, roughly in this order:
  1. **Run-length folding:** `+++++` becomes `add 5`, `>>>` becomes `move 3`.
  2. **Clear loop:** `[-]` becomes `set 0`.
  3. **Deferred pointer movement:** track a running offset and give instructions an offset field.
  4. **Scan loops:** `[>]` and `[<]` become one scan instruction.
  5. **Multiply / copy loops:** `[->+<]` and variants become multiply-adds plus `set 0`. The loop must end where it started, decrement its cell by exactly 1, and contain only adds (no I/O, no nested loops).
  6. **Known-zero tracking (stretch):** every cell starts at zero, so a loop entered on a provably zero cell is dead code.
- **Learn:** peephole optimization, pattern matching on IR, pass pipelines, strength reduction.
- **Check:** **differential testing**: optimized and unoptimized runs must agree on every program. Add an `--opt-level` flag to bisect bugs, and keep a timing table.

## Stage 4: Make the interpreter fast

- **Goal:** see why interpreters are slow on real hardware.
- **Build:** benchmark with a long-running program (for example a Mandelbrot); compare `-O0` and `-O2`; read the assembly of the dispatch loop and find the indirect jump; try direct-threaded dispatch with computed `goto` (a GCC/Clang extension); measure with `perf stat` (instructions, cycles, branches, branch misses).
- **Learn:** branch prediction, pipelining, memory hierarchy (the tape fits in L1 cache; try larger tapes).
- **Check:** all tests still pass, and you can explain the speedup with counter numbers.

## Stage 5: Backend 1, emit C

- **Goal:** the first real compiler: BF in, C out.
- **Build:** a code generator that walks the IR and prints C.

| IR | C |
|---|---|
| prologue | `unsigned char tape[30000]; unsigned char *p = tape;` |
| `add n` | `*p += n;` |
| `move n` | `p += n;` |
| `[` | `while (*p) {` |
| `]` | `}` |
| `.` / `,` | `putchar(*p);` / `*p = getchar();` |

- **Learn:** code generation, the compile / assemble / link pipeline, and how much the C compiler optimizes your output further.
- **Check:** every test program through the C backend, same harness.

## Stage 6: Backend 2, emit assembly

- **Goal:** go one level lower and see the machine.
- **Build:** emit a `.s` file for your machine's ISA (x86-64 or AArch64), assemble and link it. On x86-64 the data pointer can live in a callee-saved register such as `rbx`, `+` becomes an 8-bit add to `[rbx]`, and each loop needs unique labels with a compare and conditional jump at each end. Mind stack alignment before calling `putchar`.
- **Learn:** registers, addressing modes, flags, conditional jumps, the calling convention, sections (`.bss` vs `.text`).
- **Check:** same harness. Learn `gdb` or `lldb` for when it crashes.

## Stage 7: Backend 3, LLVM IR

- **Goal:** target LLVM and understand what it gives you.
- **Steps:**
  - **7a.** Write `.ll` files by hand for tiny programs; run with `lli`, compile with `llc`.
  - **7b.** Understand the mapping:

| BF concept | LLVM IR concept |
|---|---|
| tape | global or `alloca` array of `i8` |
| data pointer | an `alloca`'d index or pointer (let `mem2reg` promote it) |
| `+` | `load i8`, `add i8`, `store i8` through a `getelementptr` |
| `[ ... ]` | three basic blocks: header (load, `icmp eq 0`, conditional `br`), body, exit; the body branches back to the header |
| `.` / `,` | `call` to `putchar` / `getchar` |

  - **7c.** Emit textual IR from C++ (print strings).
  - **7d.** Use the LLVM C++ API (`LLVMContext`, `Module`, `IRBuilder`). Your bracket stack reappears here as a stack of loop blocks.
  - **7e.** Study the optimizer: `opt -passes=mem2reg`, `-print-after-all`, compare `-O0` and `-O2`, and check whether LLVM finds your Stage 3 optimizations.
- **Learn:** SSA, basic blocks and terminators, GEP, phi nodes, the pass pipeline.
- **Check:** same harness; compare running times across interpreter, C, assembly and LLVM outputs.

## Stage 8 (optional): JIT

Use LLVM's ORC JIT (`LLJIT`): build the module in memory, look up the entry symbol, call it as a function pointer. Learn ahead-of-time vs just-in-time compilation and executable memory.

## Stage 9 (optional capstone): MLIR dialect

A small BF dialect with a loop operation that owns a region. Write the Stage 3 optimizations as rewrite patterns, then lower through the LLVM dialect to LLVM IR. Learn dialects, regions, pattern rewriting and conversion.

---

## Parallel track: computer architecture

| Activity | Why |
|---|---|
| Compile small C functions at `-O0` and `-O2` in Compiler Explorer and read the assembly | Fluency in your ISA |
| Learn your platform's calling convention | Needed for Stage 6 |
| Time the interpreter with different tape sizes | Cache behaviour |
| `perf stat` before and after each optimization | Ties source changes to hardware counters |
| Design a tiny ISA and write an emulator for it | Fetch/decode/execute from the other side |
| Emit RISC-V and run it under QEMU (optional) | A clean, simple ISA |

## Testing strategy (from Stage 0 onward)

1. **Golden tests:** each program has an expected output file.
2. **Differential testing:** every pipeline must agree on every program.
3. **Error tests:** malformed programs fail with a specific message and a nonzero exit status.
4. **Step limits:** infinite loops must not hang the suite.
5. **Sanitizers:** build with `-fsanitize=address,undefined` during development.

## Reading list

| Resource | Use it for |
|---|---|
| *Crafting Interpreters* (Nystrom, free online) | Parsing, bytecode VMs, interpreter design |
| LLVM "Kaleidoscope" tutorial | Your first IRBuilder-based compiler |
| LLVM Language Reference (LangRef) | The authoritative IR reference |
| Compiler Explorer (godbolt.org) | Seeing assembly and LLVM IR instantly |
| *Computer Organization and Design* (Patterson and Hennessy) | Architecture fundamentals |
| *Computer Systems: A Programmer's Perspective* (Bryant and O'Hallaron) | How C maps to machine code and memory |
| *Engineering a Compiler* (Cooper and Torczon) | IRs, SSA, optimization, register allocation |
| MLIR documentation and the Toy tutorial | Stage 9 |

## Stage checklist

- [ ] **Stage 0:** known bugs fixed, bounds policy decided, errors reported with proper exit status, test suite exists
- [ ] **Stage 1:** load / analyze / execute separated, `,` implemented, trace behind a flag
- [ ] **Stage 2:** IR defined, `--dump-ir` works, tree IR built (stretch)
- [ ] **Stage 3:** at least run-length folding and clear-loop passes, differential tests passing
- [ ] **Stage 4:** benchmarked, computed-goto dispatch tried, `perf stat` numbers explained
- [ ] **Stage 5:** C backend working
- [ ] **Stage 6:** assembly backend working
- [ ] **Stage 7:** hand-written `.ll`, textual IR emission, IRBuilder version, optimizer comparison
- [ ] **Stage 8 / 9:** JIT or MLIR dialect (optional)