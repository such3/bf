# tests/

One test = one program file plus the answer files that describe what it must do.

```
tests/
├── run_tests.sh
├── programs/    NAME.bf        the program to run
└── expected/    NAME.out       the exact bytes it must print   (missing file = prints nothing)
                 NAME.status    the exit status it must return  (missing file = 0)
                 NAME.args      extra arguments, e.g. --verbose (optional)
```

## Run everything

```bash
chmod +x tests/run_tests.sh      # once; downloads lose the executable bit
make test                         # or: ./tests/run_tests.sh ./bf
```

Output is one line per test (`PASS name` or `FAIL name (exit X, wanted Y)`), then a summary.
The script exits 0 only if every test passed.

## Add a test

1. Create `tests/programs/my_test.bf`.
2. Decide, **by reasoning, not by running the program**, what it must print and which exit status it must return.
3. Create `tests/expected/my_test.out` (only if it prints something) and `tests/expected/my_test.status` (only if the status is not 0).

Use `printf`, not `echo`, and octal escapes for special bytes:

```bash
printf '+++.' > tests/programs/three.bf
printf '\003' > tests/expected/three.out        # the byte with value 3
printf '1'    > tests/expected/three.status     # must exit with status 1
printf -- '-.' > tests/programs/minus.bf        # -- because the text starts with a dash
```

## Rules

- Never copy the program's output into the expected file. That records what it does today, not what it should do.
- A test that fails for a known reason is useful: it describes the bug. Do not change the expected value to make it pass.
- Tests compare **stdout and the exit status** only. The trace and error messages go to stderr and are not compared.
