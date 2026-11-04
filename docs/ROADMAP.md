# Roadmap

This project is deliberately following a compiler-learning path. The current implementation is a working interpreter, and the future direction is to grow it from a simple runtime to a more structured compiler pipeline.

> Current phase: interpreter hardening and design cleanup. The source in [bf.cpp](../bf.cpp) already includes the core execution loop, bracket validation, debugging output, and file-based CLI execution.

---

## Current stage

| Stage | Focus | Status |
|---|---|---|
| 0 | Interpreter hardening | In progress |
| 1 | Separate load / analyze / execute | Planned |
| 2 | Introduce an IR | Planned |
| 3 | Optimize the IR | Planned |
| 4 | Fast interpreter / architecture lessons | Planned |
| 5 | Backend: emit C | Planned |
| 6 | Backend: emit assembly | Planned |
| 7 | Backend: LLVM IR | Planned |
| 8 | JIT (optional) | Optional |
| 9 | MLIR dialect (optional) | Optional |

---

## Stage 0: harden the interpreter

This is the stage the project is in right now.

Goals:

- Make the interpreter more robust around runtime edge cases.
- Keep the logic easy to follow and explain.
- Separate normal program output from verbose debug traces.
- Improve the CLI and error reporting.
- Prepare the code for the next refactor instead of layering more features on top of a rough implementation.

The current code already does the core job: file reading, bracket validation, tape loop execution, and state dumps under `--verbose`.

---

## Stage 1: split the phases

The next refactor is to separate the interpreter into cleaner phases:

- file loading
- validation / analysis
- execution

This will make diagnostics, debugging, and future IR generation easier to manage.

Expected improvements:

- clearer ownership of state
- better CLI policies
- line-and-column error reporting
- more manageable code paths without repeated logic

---

## Stage 2: introduce an IR

Once the execution model is stable, the next structural step is a proper intermediate representation.

This will let the project move from raw text execution to a more compiler-like pipeline. The code already hints at this direction through comments about jump tables, validation, and structured execution.

Possible outcomes:

- bytecode-like instruction list
- loop-aware IR
- a disassembly or `--dump-ir` mode

---

## Stage 3: optimize the IR

Once an IR is present, the project can begin optimization passes such as:

- run-length folding
- loop simplification
- pointer movement coalescing
- pattern-based strength reductions

This gives the project a direct bridge from interpreter design to compiler optimization work.

---

## Stage 4: make the interpreter faster

The focus here is performance and architecture understanding. The project will start measuring behavior on real hardware and studying how fetch/decode/execute loops behave in practice.

This stage is meant to connect the code with:

- branch prediction
- memory layout
- assembly-level behavior
- interpreter dispatch performance

---

## Stage 5+: compiler backends

After the interpreter is stable and the IR exists, the project can move toward a real compiler pipeline:

- emit C
- emit assembly
- emit LLVM IR
- optionally JIT or MLIR work

This is the long-range direction from learning interpreter behavior toward compiler construction.

---

## Key design ideas to keep in view

### Interpreter vs compiler

| Concept | Current stage |
|---|---|
| Walk the program and execute it | Yes |
| Lower to a structured IR | Planned |
| Emit machine code / assembly / C | Planned |
| Optimize on a representation rather than raw source | Planned |

### Brainfuck execution model

The code in [bf.cpp](../bf.cpp) is already using a simple virtual machine model:

- program counter
- data pointer
- tape memory
- jump table resolution
- direct execution loop

That is the right foundation for later compiler work.

---

## Practical next steps

1. Stabilize runtime guarding and CLI behavior.
2. Add proper input support for `,`.
3. Split the interpreter into clearer phases.
4. Build a small IR abstraction.
5. Use optimization passes and backend generation as the next major milestone.

The roadmap remains deliberately honest: the project is still building the foundation before it becomes a full compiler toolkit.