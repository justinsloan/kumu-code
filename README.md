# Kumu

A small, BASIC-inspired programming language built for teaching — with a hand-written
interpreter in C++17, a reference manual, and a 17-chapter textbook.

Kumu keeps the friendly parts of BASIC (`PRINT`, `IF`, `FOR`, uppercase keywords) and
drops the parts that make it hard to learn from. There are no line numbers and no `GOTO`.
Every block in the language is written exactly one way — a header followed by `{ ... }` —
so there is only ever one shape to learn.

**📖 Read the manual and textbook at <https://justinsloan.github.io/kumu-code/>**

```basic
# Average a student's scores, and say how they did.

FUNCTION average(scores) {
    VAR total = 0
    FOR EACH s IN scores {
        total = total + s
    }
    RETURN total / scores.LEN()
}

VAR scores = [88, 92, 79, 95]
VAR mean = average(scores)

PRINT "Scores:  " + scores.JOIN(", ")
PRINT "Average: " + mean

IF mean >= 90 {
    PRINT "Grade:   A"
} ELSEIF mean >= 80 {
    PRINT "Grade:   B"
} ELSE {
    PRINT "Grade:   C or below"
}
```

```
Scores:  88, 92, 79, 95
Average: 88.5
Grade:   B
```

## Quick start

Requires `g++` with C++17 support and `make`. No external dependencies.

```sh
git clone https://github.com/justinsloan/kumu-code.git
cd kumu-code
make
./kumu collections.kumu
```

`make` produces a single `kumu` binary. `make clean` removes it along with the build
artifacts.

To run your own program, save it with a `.kumu` extension and pass it to the interpreter:

```sh
./kumu myprogram.kumu
```

## Error messages are part of the language

Most of the design effort here went somewhere unusual for a toy language: into what
happens when a program is *wrong*. A beginner spends more time reading error messages
than reading successful output, so Kumu tries to say what went wrong, where, and what you
probably meant.

Every uncaught error shows the offending line with a caret under the exact spot that
failed. An undefined name suggests the closest one that actually exists:

```
Runtime Error: Undefined variable: totl -- did you mean 'total'? (line 3)

 3 | PRINT totl / count
   |       ^
```

When the error happens inside a call, the chain of calls that led there follows it,
innermost first:

```
Runtime Error: Division by zero (line 2)

 2 |     RETURN x / 0
   |              ^
  in half() called at line 4
```

Other things Kumu notices rather than shrugging at: a keyword typed in lowercase
(`print` → "Kumu keywords are written in capitals"), `=` used where `==` was meant,
arithmetic on two strings, and a method name that doesn't exist (`.LENGTH()` → "did you
mean 'LEN'?"). Nothing fails silently — a stray `%` or `;` is reported, not skipped.

## The language at a glance

| Type | Description | Example |
| :--- | :--- | :--- |
| **Number** | Floating point, always finite | `10`, `3.14`, `-5.0` |
| **String** | Double-quoted text, no escape sequences | `"Hello World"` |
| **List** | Ordered sequence of any values, indexed from 1 | `[1, "two", [3]]` |
| **Matrix** | 2D table of text with a permanent header row | `(("Name", "Score"), ("Alice", "95"))` |
| **Function** | A named block, with recursion | `FUNCTION f(a) { ... }` |

**Control flow** — `IF` / `ELSEIF` / `ELSE`, `WHILE`, `FOR i = 1 TO n`, `FOR EACH x IN list`,
and `TRY` / `CATCH` for error handling.

**Built-ins** — string functions (`LEN`, `SUBSTRING`, `SPLIT`, `REPLACE`, `TRIM`,
`UPPER`/`LOWER`, `CONTAINS`, `INDEX_OF`, …), dates (`TODAY`, `NOW`, `DATE_DIFF`,
`DATE_ADD`, `WEEKDAY`, `YEAR`/`MONTH`/`DAY`, guarded by `IS_DATE`), console control
(`INPUT`, `CLEAR_SCREEN`, `SLEEP`), `RANDOM`, and a `TRACE` statement for watching
execution step by step.

**File I/O** — `READ_FILE` / `WRITE_FILE` for plain text, `READ_CSV` and `.SAVE()` for
CSV, and `FILE_EXISTS` for first-run setup.

Two design choices worth knowing before you read any code here:

- **Braces mean "block" and nothing else.** `( )` groups a sub-expression, carries a
  call's arguments or a function's parameters, and writes a Matrix literal. `[ ]` writes
  and indexes a List.
- **Collection methods never mutate.** Every List and Matrix method that changes data
  returns a *new* collection, so keeping a change always looks like
  `book = book.APPEND_ROW(...)`. This is deliberate: it makes aliasing bugs impossible in
  the one place beginners hit them hardest. Everything else in the language is ordinary
  mutable-by-default imperative code.

## Documentation

Both documents are published, rendered and searchable, at
**<https://justinsloan.github.io/kumu-code/>**. Their sources live in [`docs/`](docs/).

- **[Textbook](https://justinsloan.github.io/kumu-code/textbook/)** — a 17-chapter,
  example-driven course, written to be worked through in order by someone learning alone.
  Each chapter opens with the *problem* a feature solves before showing any syntax, then
  covers common mistakes and three kinds of exercise (*predict*, *fix*, *write*). Every
  answer is in Appendix A, and every example has been run through the interpreter — the
  output shown is real, not guessed.
- **[Manual](https://justinsloan.github.io/kumu-code/manual/)** — the reference manual.
  Ten sections covering the full syntax, every built-in, and precise semantics. Where the
  textbook answers "how do I use this?", the manual answers "what does this do, exactly?"
- **[Goals](https://justinsloan.github.io/kumu-code/goals/)** — what Kumu is trying to do
  and why it is shaped the way it is.

## Example programs

| Program | What it shows |
| :--- | :--- |
| [`todo.kumu`](todo.kumu) | A complete to-do manager — due dates, tags, priorities, completion and an archive, sorted any way you like and persisted to `todo.csv`. The largest program here. |
| [`address.kumu`](address.kumu) | The flagship example — an interactive address book using essentially every feature together. Contacts persist to `contacts.csv`, the book is a live Matrix with header-aware dot-methods, and `TRY`/`CATCH` turns end-of-input into a graceful save-and-exit. |
| [`collections.kumu`](collections.kumu) | Matrix vs. List — when you want a table and when you want a sequence, using a gradebook as the worked example. |
| [`test.kumu`](test.kumu) | A broad feature sweep, useful as a syntax reference by example. |
| [`hello.kumu`](hello.kumu) | A short tour that ends with a deliberate mistake, to show what an error looks like. |

The `.csv` and `.txt` files in the repository root are fixtures these programs read and
write. Their contents are entirely synthetic.

## Project layout

```
main.cpp                 entry point — reads a .kumu file and runs it
lexer.cpp / lexer.hpp    source text  → tokens
parser.cpp / parser.hpp  tokens       → AST
interpreter.cpp / .hpp   AST          → execution (tree-walking)
ast.hpp                  node definitions for the syntax tree
core.hpp                 Value type and shared primitives
Makefile                 builds ./kumu
```

The pipeline is a conventional three-stage one — lex, parse, walk the tree — with no
bytecode step and no external parser generator.

## Status

Kumu runs, and everything documented in the manual and textbook works. Two things it does
not have yet, stated plainly:

- **No automated test suite.** `test.kumu` is a demonstration program, not a test — it
  prints output that nothing compares against, so it catches no regressions on its own.
- **No package or install step.** Build it and run the binary out of the source directory.

## License

Released under the MIT License. See [`LICENSE`](LICENSE).
