# Brainfuck Language Rules

Brainfuck is an esoteric programming language with only **eight
commands**. A program operates on a tape of memory cells and uses a
pointer to select the current cell.

## 1. The Eight Commands

  -----------------------------------------------------------------------
  Command                 Name                    Meaning
  ----------------------- ----------------------- -----------------------
  `>`                     Move right              Move the data pointer
                                                  one cell to the right.

  `<`                     Move left               Move the data pointer
                                                  one cell to the left.

  `+`                     Increment               Increase the value in
                                                  the current cell by 1.

  `-`                     Decrement               Decrease the value in
                                                  the current cell by 1.

  `.`                     Output                  Output the character
                                                  represented by the
                                                  current cell's value.

  `,`                     Input                   Read one character of
                                                  input and store its
                                                  value in the current
                                                  cell.

  `[`                     Loop start              If the current cell is
                                                  zero, jump to the
                                                  command after the
                                                  matching `]`;
                                                  otherwise, continue.

  `]`                     Loop end                If the current cell is
                                                  nonzero, jump back to
                                                  the command after the
                                                  matching `[`;
                                                  otherwise, continue.
  -----------------------------------------------------------------------

## 2. Memory and Pointer Rules

-   The program uses a tape made of cells.
-   At the start, the data pointer points to the first cell.
-   Cells are typically initialized to `0`.
-   `>` and `<` move the pointer between cells; they do not change cell
    values.
-   `+` and `-` change only the value in the current cell.
-   The tape's size and behavior at its boundaries depend on the
    interpreter.
-   Cell values are often 8-bit unsigned integers, but this is
    interpreter-dependent. They may wrap around (for example, `255 + 1`
    becomes `0`) or use another representation.

## 3. Loop Rules

-   Every `[` must have a matching `]`, and every `]` must have a
    matching `[`.
-   If a `[` is reached while the current cell is `0`, execution skips
    to the command immediately after its matching `]`.
-   If a `[` is reached while the current cell is nonzero, execution
    continues into the loop body.
-   When `]` is reached and the current cell is nonzero, execution
    returns to the command immediately after the matching `[`.
-   When `]` is reached and the current cell is `0`, execution continues
    after the `]`.
-   A loop must change the condition cell or otherwise reach zero if it
    is expected to terminate. An infinite loop is possible.

Example:

``` brainfuck
+++[-]
```

This sets the current cell to `3`, then repeatedly subtracts `1` until
the cell becomes `0`.

## 4. Input and Output

-   `.` outputs the current cell's value as a character, according to
    the interpreter's character encoding.
-   `,` reads one character and stores its numeric value in the current
    cell.
-   What happens when input reaches end-of-file is
    interpreter-dependent.
-   Brainfuck does not have built-in commands for printing numbers as
    decimal text; programs must implement that behavior themselves.

## 5. Comments and Whitespace

-   Only the eight command characters have Brainfuck meaning:
    `> < + - . , [ ]`.
-   In most interpreters, any other character is ignored and can be used
    for comments or formatting.
-   Whitespace is usually ignored, so commands can be split across lines
    for readability.
-   Some tools impose their own parsing rules, so check the interpreter
    if behavior differs.

## 6. Common Programming Patterns

### Clear the current cell

``` brainfuck
[-]
```

Repeatedly subtracts from the current cell until it reaches zero,
assuming the cell uses the usual wrapping or nonnegative-cell behavior.

### Increment a cell three times

``` brainfuck
+++
```

### Move to the next cell and set it to two

``` brainfuck
>++
```

### Output a character

``` brainfuck
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.
```

This example adds 65 to a zeroed cell and outputs `A` in an
ASCII-compatible interpreter.

## 7. Important Portability Notes

Brainfuck has no single universally enforced specification for
implementation details. Interpreters may differ in:

-   Tape length and what happens when the pointer moves beyond either
    end.
-   Cell width and whether arithmetic wraps around.
-   Input behavior at end-of-file.
-   Character encoding used for input and output.
-   Whether non-command characters are always ignored.

For portable programs, avoid relying on unspecified behavior unless you
know the target interpreter.

## Quick Reference

``` text
>  move pointer right
<  move pointer left
+  increment current cell
-  decrement current cell
.  output current cell as a character
,  input a character into current cell
[  skip loop if current cell is zero
]  repeat loop if current cell is nonzero
```
