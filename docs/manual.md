---
title: Manual
nav_order: 4
layout: doc
permalink: /manual/
---

# Manual: Kumu

This manual describes the syntax and capabilities of Kumu, the language implemented in this project. Kumu is BASIC-inspired but modern and structured, using explicit blocks instead of line numbers. Run a program with `./kumu program.kumu`.

---

## 1. Overview

Kumu is a procedural language that supports variable assignment, arithmetic, logical comparisons, control flow (conditionals and loops), error handling, user-defined functions, interactive console input and output (including clearing the screen and pausing), string and matrix utilities, and file I/O operations (including CSV support).

**Key structural rule:** Unlike classic BASIC, Kumu uses **blocks** for control structures, and every block is written the same way: `{ ... }`. There is no second spelling and no optional part -- `IF`, `WHILE`, `FOR`, `FOR EACH`, `FUNCTION` and `TRY` all read as a header followed by a braced body, so there is never a question of which style a construct wants. Braces do this job and nothing else: `( )` groups a sub-expression, holds a call's arguments, a function's parameters, or a Matrix literal, and `[ ]` belongs to Lists.

**A note on collection methods:** everywhere else in this language, assignment mutates -- `x = x + 1` changes what `x` holds. The methods on Matrix (§8) and List (§7) deliberately don't: every one that changes data (`.SET`, `.APPEND_ROW`, `.DELETE_ROW`, `.SORT`) returns a **new** Matrix -- and every List method that changes data (`.SET`, `.APPEND`, `.INSERT`, `.REMOVE`, `.SORT`, `.REVERSE`) likewise returns a **new** List -- leaving the one you called it on untouched, so "editing" a collection always looks like `book = book.SET(...)` rather than a method quietly changing `book` out from under you. It's a small, intentional bit of functional-style discipline sitting inside an otherwise fully imperative, mutable-by-default language.

---

## 2. Data Types

| Type | Description | Example |
| :--- | :--- | :--- |
| **Number** | Floating-point numbers | `10`, `3.14`, `-5.0` |
| **String** | Text enclosed in double quotes | `"Hello World"` |
| **List** | An ordered sequence of any Values, written in square brackets and indexed from 1 (§7) | `[1, "two", [3]]` |
| **Matrix** | A 2D table of text with a permanent header row, accessed with dot-methods like `.GET`/`.SUM` (§8) | `(("Name", "Score"), ("Alice", "95"))` |
| **Function** | A defined block of code | `FUNCTION my_func(a) { ... }` |

---

## 3. Expressions & Operators

Expressions are evaluated to produce a `Value`.

### Arithmetic Operators
- `+` : Addition
- `-` : Subtraction
- `*` : Multiplication
- `/` : Division
- `MOD` : Remainder after division (e.g. `7 MOD 2` is `1`). Works on decimals too, and dividing by zero is an error, exactly as with `/`.

`MOD` binds as tightly as `*` and `/`, so `1 + 6 MOD 4` is `3`, not `1`.

A result too large to represent (roughly 1.8e308) is a runtime error rather than a silent infinity, since an infinity would go on to poison every comparison and sum that touched it. A Kumu Number is always an ordinary, finite number.

### Unary Operators
- `-` : Negation (e.g., `-5`)
- `!` : Logical NOT (treats `0` as false, non-zero as true)
- `NOT` : A spelled-out synonym for `!`. `NOT x` and `!x` are the same thing; use whichever reads better.

### Comparison Operators
- `==` : Equal to
- `!=` : Not equal to
- `<`  : Less than
- `>`  : Greater than
- `<=` : Less than or equal to
- `>=` : Greater than or equal to

Comparing two values of different types (e.g. a Number to a String) is allowed for `==`/`!=` only, and they're simply never equal -- this matters for values read with `INPUT`, whose type depends on what the user typed. `<`, `>`, `<=`, and `>=` require both sides to be the same type (both Number or both String) and are a runtime error otherwise, as is comparing a Matrix with anything using any comparison operator.

Two Lists compare with `==` and `!=` element by element, all the way down through any nesting, so `[1, [2]] == [1, [2]]` is true. Ordering two Lists (`<`, `>`, ...) has no obvious meaning and is an error.

**`=` versus `==`:** `=` assigns, `==` compares. Writing `IF x = 5 {` still works -- it's read as a comparison -- but it prints a warning telling you what you probably meant:
<!-- check
VAR x = 5
VAR y = 1
VAR z = 2
IF x = 5 {
    PRINT "yes"
}
-->
```
Warning (line 4): '=' used as a comparison -- did you mean '=='?
```
Prefer `==` in every condition.

### Logical Operators
- `AND` : true when both sides are true
- `OR`  : true when either side is true
- `NOT` : flips true and false

They produce `1` or `0`, like the comparisons, and `AND` binds more tightly than `OR`, so `a AND b OR c` means `(a AND b) OR c`.

Both **short-circuit**: `AND` doesn't evaluate its right side when the left is false, and `OR` doesn't when the left is true. This is what makes a guard like the following safe rather than a division by zero:
```basic
IF count != 0 AND total / count > 5 {
    PRINT "above average"
}
```

### String Operations
- `+` : String concatenation (e.g., `"Hello " + "World"`)
- `==` : String equality
- `!=` : String inequality

**String literals have no escape sequences.** A `\` is an ordinary backslash and `\n` is a backslash followed by the letter n, not a newline. A literal may, however, span several lines -- the line break becomes part of the text:
```basic
PRINT "first
second"
```
Two consequences worth knowing: there is no way to put a `"` inside a string, and a newline has to be written as an actual line break.

### Grouping
Parentheses can wrap any sub-expression to control evaluation order, just like in arithmetic:
```basic
PRINT (1 + 2) * 3   # 9, not 7
```

### String Functions
All 1-indexed, matching this language's `FOR i = 1 TO n` convention.
- **LEN(str)**: Character count.
- **SUBSTRING(str, start, length)**: The substring starting at `start` and up to `length` characters long. `length` is clamped to however many characters are actually left, so `SUBSTRING(s, 5, LEN(s))` conveniently means "everything from position 5 onward." `start` beyond the end of the string is a runtime error, except `start == LEN(str) + 1`, which returns `""`.
- **INDEX_OF(str, search)**: The 1-based position of the first occurrence of `search` in `str`, or `0` if it isn't found.
- **UPPER(str)** / **LOWER(str)**: Case-converted copy.
- **TRIM(str)**: Copy with leading and trailing whitespace removed.
- **REPLACE(str, find, replacement)**: Copy with *every* occurrence of `find` replaced. `find` cannot be empty.
- **CONTAINS(str, text)**: `1` if `text` appears anywhere in `str`, else `0`.
- **STARTS_WITH(str, text)** / **ENDS_WITH(str, text)**: `1` or `0`.
- **SPLIT(str, separator)**: Breaks `str` apart at each `separator` and returns a **List** of the pieces (§7). An empty separator splits into single characters.

```basic
VAR s = "Hello, World"
PRINT LEN(s)                    # 12
PRINT SUBSTRING(s, 1, 5)        # "Hello"
PRINT INDEX_OF(s, "World")      # 8
PRINT UPPER(s)                  # "HELLO, WORLD"
PRINT TRIM("  hi  ")            # "hi"
PRINT REPLACE(s, "l", "L")      # "HeLLo, WorLd"
PRINT CONTAINS(s, "World")      # 1
PRINT SPLIT("a,b,c", ",")       # ["a", "b", "c"]
```

### Math Functions
- **ABS(n)**: Absolute value.
- **FLOOR(n)**: Rounds down to a whole number.
- **ROUND(n)**: Rounds to the nearest whole number.
- **SQRT(n)**: Square root. A negative `n` is a runtime error.
- **POW(base, exponent)**: `base` raised to `exponent`.

```basic
PRINT ABS(-7)        # 7
PRINT FLOOR(3.9)     # 3
PRINT ROUND(2.5)     # 3
PRINT SQRT(144)      # 12
PRINT POW(2, 10)     # 1024
```

### Type Conversion
Kumu figures out types on its own, with one place it can't: `INPUT` produces a Number when the user typed digits and a String otherwise, and every cell read from a CSV arrives as text. These three functions are how a program copes with that.

- **IS_NUMBER(value)**: `1` if `value` is a Number *or* is text that reads as one, else `0`. The words `nan` and `inf` count as text, not numbers -- they are not values Kumu arithmetic can produce, so text spelling them is left as text.
- **NUMBER(value)**: The value as a Number. A runtime error if it isn't one -- check with `IS_NUMBER` first.
- **STRING(value)**: The value as a String, formatted exactly as `PRINT` would show it.

```basic
INPUT age
IF IS_NUMBER(age) {
    PRINT "Next year you'll be " + (NUMBER(age) + 1)
} ELSE {
    PRINT "That wasn't a number."
}
```

### Random Numbers
- **RANDOM(min, max)**: A random integer in the inclusive range `[min, max]`. `min` must be `<= max`, and both must be whole numbers no larger than 1e15.

```basic
PRINT RANDOM(1, 6)   # roll a die
```

---

## 4. Statements

### Variables and Assignment
You can declare a variable with `VAR` or perform a direct (bare) assignment.
```basic
VAR x = 10
y = 20
```
At the top level of a program these behave identically. Inside a function they mean different things -- see Scope and Recursion (§5).

### Printing
Use `PRINT` to output a value to the console.
```basic
PRINT x + y
PRINT "Result is: " + x
```

### Clearing the Screen
`CLEAR_SCREEN()` clears the terminal and moves the cursor back to the top-left. It takes no arguments and is normally called on its own as a statement:
```basic
PRINT "This will disappear."
CLEAR_SCREEN()
PRINT "Fresh screen."
```
It works via the standard ANSI clear-and-home escape sequence, which essentially every modern terminal understands.

### Pausing (SLEEP)
`SLEEP(seconds)` pauses the program for the given number of seconds, which may be fractional (`SLEEP(0.5)` waits half a second). `seconds` must not be negative.
```basic
PRINT "Rolling..."
SLEEP(1)
PRINT RANDOM(1, 6)
```

### Input
Use `INPUT` to read one line of text from the console into a variable.
```basic
PRINT "What is your name?"
INPUT name
PRINT "Hello, " + name
```
There's no type keyword: a line that looks entirely like a number becomes a Number, and anything else is kept as a String. If there's no more input to read (end-of-input), `INPUT` raises a runtime error rather than continuing silently; wrap it in `TRY`/`CATCH` (§4) if you want to handle running out of input gracefully instead of stopping the whole program.

### Conditionals (IF-ELSEIF-ELSE)
```basic
IF x > 10 {
    PRINT "Large"
} ELSE {
    PRINT "Small"
}
```
For more than two branches, `ELSEIF` chains any number of extra conditions onto one `IF`, each branch closing its own brace:
```basic
IF x >= 90 {
    PRINT "A"
} ELSEIF x >= 80 {
    PRINT "B"
} ELSEIF x >= 70 {
    PRINT "C"
} ELSE {
    PRINT "F"
}
```
`ELSEIF` is exactly equivalent to writing `ELSE { IF ... }` and nesting one level deeper per branch -- it's parsed that way internally -- so it's purely a readability tool for multi-way branches, not a new capability. It must be written as one word: `ELSE IF` (with a space) tokenizes as two separate keywords and means something different -- an `ELSE` branch whose own body happens to start with a nested `IF` (which then needs its own braces).

### Loops
#### WHILE Loop
```basic
WHILE x < 10 {
    x = x + 1
}
```

#### FOR Loop
```basic
FOR i = 1 TO 5 {
    PRINT i
}
```
By default `FOR` counts upward in steps of 1.

#### FOR ... STEP
`STEP` sets how much the counter moves each time round. A negative step counts **down**:
```basic
FOR i = 1 TO 10 STEP 2 {
    PRINT i                     # 1 3 5 7 9
}

FOR i = 5 TO 1 STEP -1 {
    PRINT i                     # 5 4 3 2 1
}
```
A `STEP` of `0` is an error, since the loop could never finish.

Counting down **requires** `STEP`. Without it the counter only ever goes up, so `FOR i = 10 TO 1` runs zero times rather than counting backwards -- it has always behaved this way, and saying `STEP -1` when you mean "downwards" keeps the intent visible.

#### FOR EACH
`FOR EACH` walks a collection directly, without any counter:
```basic
FOR EACH name IN ["Ada", "Alan", "Grace"] {
    PRINT "Hello, " + name
}
```
It also accepts a Matrix, handing you each **data row** as a List (the header row is skipped, as everywhere else):
```basic
FOR EACH row IN book {
    PRINT row[1] + " - " + row[2]
}
```
The items are collected before the loop starts, so rebuilding the collection inside the body can't make the loop grow or shrink underneath itself.

#### BREAK and CONTINUE
`BREAK` leaves the innermost loop immediately; `CONTINUE` skips the rest of the current pass and starts the next one. Both work in `WHILE`, `FOR`, and `FOR EACH`:
```basic
FOR i = 1 TO 10 {
    IF i MOD 2 == 0 {
        CONTINUE
    }
    IF i > 7 {
        BREAK
    }
    PRINT i                     # 1 3 5 7
}
```
Used outside any loop -- including inside a function called from a loop, which would otherwise reach across the call and break the *caller's* loop -- either one is a runtime error.

### Error Handling (TRY-CATCH)
`TRY` runs a block; if any statement inside it raises a runtime error (like dividing by zero, an undefined variable, or `INPUT` hitting end-of-input), execution jumps to `CATCH`, binding the error message (as a String) to the variable you name there, and the program keeps going instead of stopping.
```basic
TRY {
    PRINT 10 / 0
} CATCH err {
    PRINT "Something went wrong: " + err
}
PRINT "Still running."
```
A `RETURN` inside a `TRY` (or its `CATCH`) still exits the enclosing function normally -- it isn't treated as an error.

The `CATCH` variable (`err` above) is bound the same way `VAR` would bind it: local to the enclosing function if `TRY` is inside one, global at the top level (see §5).

### Tracing (TRACE)
`TRACE 1` makes the interpreter print every statement as it runs, with its source line, which is useful for watching your own program's control flow while debugging it. `TRACE 0` turns it back off. Since `TRACE` takes any expression, you can also gate it on a condition, e.g. `TRACE debugMode`.

Two details make the output readable. Statements that change a variable -- `VAR`, bare assignment, and `INPUT` -- also print the value that landed, which is usually the thing you were trying to find out. And trace lines are indented one step per level of function call, so a recursive function reads as a shape rather than a flat list of repeats.
```basic
TRACE 1
VAR x = 5
PRINT x
TRACE 0
```
```
[TRACE line 2] VAR x = 5
[TRACE line 3] PRINT
5
[TRACE line 4] TRACE
```
(`TRACE 0` itself gets traced too -- tracing is still on when that statement *starts*, since it's the statement that turns it off.)

---

## 5. Functions

Functions allow you to encapsulate logic and reuse it. They support parameters and return values.

### Defining a Function
A function's body is a braced block, like every other block. The parameter list is required, even when it is empty: `FUNCTION tick() { ... }`.
```basic
FUNCTION multiply(a, b) {
    RETURN a * b
}

VAR result = multiply(5, 4)
PRINT result
```

A function's body may be left with a bare `RETURN`, which returns `0`. It's the natural way to bail out early:
```basic
FUNCTION greet(name) {
    IF name == "" {
        RETURN
    }
    PRINT "Hello, " + name
}
```
A function that simply runs off the end of its body also returns `0`.

### Calling a Function
```basic
PRINT multiply(10, 2)
```
Definitions are registered before the program runs, so a function can be called from **above** the line that defines it, and two functions can call each other:
```basic
PRINT double(21)                # 42, even though double is defined below

FUNCTION double(n) {
    RETURN n * 2
}
```
Defining the same function name twice is an error.
A function can also be called on its own as a statement, discarding whatever it returns -- useful for a call made only for a side effect:
```basic
logMessage("started")
```

### Scope and Recursion
Recursion is limited to 1000 nested calls. Going past that is an ordinary runtime error naming the likely cause, which a `TRY`/`CATCH` can catch like any other:
```
Maximum recursion depth (1000) exceeded -- is this function missing a base case?
```

Every function call gets its own scope: its parameters, and anything it declares with `VAR`, live there and disappear when the call returns. A recursive call gets a fresh scope of its own too, so a function calling itself never collides with its own earlier calls still in progress.

Bare assignment (no `VAR`) works differently on purpose: it updates the nearest existing binding of that name -- the current call's own scope if it has one, otherwise the global scope -- and creates a new *global* if neither has it yet. This is what lets a function deliberately read or change state it was never passed as a parameter, without declaring every such variable up front:
```basic
VAR total = 0

FUNCTION addToTotal(amount) {
    total = total + amount
}

addToTotal(10)
addToTotal(20)
PRINT total
```
```
30
```
`addToTotal` has no parameter or `VAR` named `total`, so its bare assignment reaches straight through to the global `total` and updates it. Had the function instead written `VAR total = amount` (declaring its own local `total`), that local would shadow the global for the rest of the call and disappear afterward, leaving the global's own `30` -- sorry, `0` -- untouched.

Scope in Kumu is per **function call**, not per block: a `VAR` declared inside an `IF`, `WHILE`, or `FOR` body is still visible for the rest of that same function call (or the rest of the top-level program, if not inside a function), not just within that block. At the top level, `VAR` and bare assignment are identical, because there's no enclosing call -- both simply set a global.

A `FOR` loop's counter, and a `CATCH` block's error variable, are both bound the way `VAR` binds a name: local to the enclosing call if there is one.

Functions can call themselves or each other, including recursively:
```basic
FUNCTION factorial(n) {
    IF n <= 1 {
        RETURN 1
    } ELSE {
        RETURN n * factorial(n - 1)
    }
}
```
If a function's body never runs a `RETURN`, calling it evaluates to `0`.

---

## 6. File I/O

The language provides built-in support for reading and writing files and CSV data.

### Plain Text Files
- **READ_FILE**: Reads an entire file as a single string.
- **WRITE_FILE**: Writes a string to a file.

```basic
VAR content = READ_FILE("notes.txt")
PRINT content

WRITE_FILE("output.txt", "This is a test.")
```

### CSV Files
- **READ_CSV(path)**: Reads a CSV file into a Matrix -- row 1 becomes the header. A plain function, not a method, since there's no existing Matrix to call it on yet.

To write a Matrix back to CSV, call its `.SAVE(path)` method (§8) rather than a bare function:

```basic
# Writing a CSV
VAR data = (
    ("Name", "Score"),
    ("Alice", "95"),
    ("Bob", "88")
)
data.SAVE("scores.csv")

# Reading a CSV
VAR my_data = READ_CSV("scores.csv")
PRINT my_data
```

#### Quoting
`.SAVE` and `READ_CSV` follow the usual CSV quoting rules, so a cell can hold
the characters that would otherwise break the format. A cell containing a
comma, a double quote, or a line break is written wrapped in double quotes,
and a double quote inside such a cell is written twice:

```
Name,Note
"Smith, Jr.","he said ""hi"""
"an address
over two lines",plain
```

`READ_CSV` understands the same convention, so anything `.SAVE` writes reads
back byte for byte identical -- including commas inside cells, which used to
be split into extra columns and silently lose data. Empty cells at the end of
a row are preserved too.

Line endings may be LF or CRLF, and this holds *inside* a quoted cell as well
as between rows: a cell containing a line break reads the same whether the file
was written on Windows or not, rather than carrying a stray carriage return
along in the value. (A lone carriage return that isn't a line ending is left
alone, since it is data.)

A file that ends in the middle of a quoted cell is a runtime error naming the
file, rather than a table quietly missing everything after the stray quote.
(A quote that is not the first character of its cell -- as in `a"b` -- is
ordinary text and needs no special treatment.)

### Checking Whether a File Exists
- **FILE_EXISTS**: Returns `1` if a path exists on disk, `0` otherwise. Useful for creating a file with default contents only the first time a program runs.

```basic
IF FILE_EXISTS("contacts.csv") {
    PRINT "Already there."
} ELSE {
    WRITE_FILE("contacts.csv", "")
}
```

---

## 7. List Operations

A **List** is an ordered sequence of Values, written in square brackets:
```basic
VAR scores = [95, 88, 72]
VAR mixed = [1, "two", [3, 4]]      # any Values, including other Lists
VAR empty = []
```
Elements are ordinary expressions, evaluated each time the literal is reached, so `[n, n * 2]` works.

Lists may be nested up to 100 deep. Past that is a runtime error -- the limit exists because anything that walks a nested list (printing it, comparing it, discarding it) walks it one level at a time, and an unbounded chain would exhaust the interpreter's own stack.

### Reading
Index with square brackets, counting from **1** -- the same convention as `SUBSTRING`, `INDEX_OF`, and Matrix row numbers:
```basic
PRINT scores[1]                     # 95
PRINT scores.LEN()                  # 3
PRINT LEN(scores)                   # 3, the same thing
```
A position of `0`, or past the end, is a runtime error naming the length.

### List vs Matrix
Both are collections, and they are good at different jobs.

| | List | Matrix |
| :--- | :--- | :--- |
| Shape | One dimension | Two dimensions, with a header row |
| Holds | Any Values, including Lists | Text only |
| Indexing | `myList[2]` | `.GET(row, column)` |
| Best for | A sequence of things | A table you'd otherwise keep in a spreadsheet |

`FOR EACH` over a Matrix hands you each data row as a List, which is how the two meet.

### Methods
As with Matrix, **no method changes the List it was called on** -- each returns a new one, so keeping a change means assigning it back: `scores = scores.APPEND(60)`.

- **.LEN()**: How many items.
- **.GET(position)**: The item at `position`. Same as `myList[position]`.
- **.SET(position, value)**: A copy with `position` replaced.
- **.APPEND(value, ...)**: A copy with one or more values added at the end.
- **.INSERT(position, value)**: A copy with `value` inserted *before* `position`. A position of `.LEN() + 1` appends.
- **.REMOVE(position)**: A copy without the item at `position`.
- **.CONTAINS(value)**: `1` if the list holds `value`, else `0`.
- **.INDEX_OF(value)**: The 1-based position of the first match, or `0` if there is none -- the same "0 means not found" convention as the string `INDEX_OF`.
- **.SORT()**: A sorted copy. Numeric order when every item reads as a number, otherwise text order -- the same auto-detection `Matrix.SORT` uses.
- **.REVERSE()**: A reversed copy.
- **.SLICE(start, length)**: A copy of `length` items starting at `start`. `length` is clamped to what's actually left.
- **.JOIN(separator)**: All the items glued into one String. The inverse of `SPLIT`.
- **.SUM()** / **.AVERAGE()** / **.MIN()** / **.MAX()**: Over the numbers in the list. Text that reads as a number counts (so a list straight from `SPLIT` or a CSV column works); anything else is an error naming the offending item. `.AVERAGE`, `.MIN` and `.MAX` need at least one item.

```basic
VAR scores = [95, 88, 72]
PRINT scores.SUM()                  # 255
PRINT scores.AVERAGE()              # 85
PRINT scores.SORT()                 # [72, 88, 95]
PRINT scores.APPEND(60)             # [95, 88, 72, 60] -- a new list; .SORT() above
                                    #                    didn't change `scores` either
PRINT scores                        # [95, 88, 72]     -- the original, unchanged
```

### Printing a List
`PRINT` shows a List in bracket form with its **strings quoted**, because inside a list that's the only thing separating the number `1` from the text `"1"`:
```basic
PRINT [1, "two", [3]]               # [1, "two", [3]]
```
`.JOIN` is what you want for output meant for a person:
```basic
PRINT ["a", "b", "c"].JOIN(", ")    # a, b, c
```

### Lists Are Immutable
There is no `myList[1] = 99`; trying it is a syntax error that tells you what to write instead. Use `.SET`:
```basic
VAR xs = [1, 2, 3]
VAR ys = xs
xs = xs.SET(1, 99)
PRINT xs                            # [99, 2, 3]
PRINT ys                            # [1, 2, 3] -- untouched
```

---

## 8. Matrix Operations

A Matrix is a genuine object in Kumu: once you have one, you call methods on it with `.` instead of passing it as the first argument to a function (`book.GET(...)`, not `MATRIX_GET(book, ...)`). **Row 1 is always the header.** Every method treats it that way, so row numbers everywhere else always mean *data* rows -- `.GET(1, ...)` is always the first real row, never the header text, and `.ROWS()` never counts the header either.

A Matrix literal's cells must be literal strings or numbers, not variables or expressions -- there is no way to build a Matrix from other values directly:
```basic
VAR book = (
    ("Name", "Phone"),
    ("Alice", "555-0101"),
    ("Bob", "555-0142")
)
```
A single-row literal (just a header, no data rows) is a valid, "empty" Matrix, ready to have rows appended to it.

Every column argument below can be a **Number** (1-based position) or a **String** (the exact header text) -- `book.GET(1, "Phone")` and `book.GET(1, 2)` are the same call. Every method that changes data (`.SET`, `.APPEND_ROW`, `.DELETE_ROW`, `.SORT`) returns a **new** Matrix rather than modifying the one you called it on -- reassign if you want the change to stick: `book = book.SET(1, "Phone", "555-9999")`.

### Reading and Modifying
- **.GET(row, col)**: The string in that cell. A runtime error if `row`/`col` is out of range, or `col` names a column that doesn't exist.
- **.SET(row, col, value)**: A copy with that cell replaced by `value` (converted to a string if it isn't one already).
- **.ROWS()**: Number of data rows (the header is never counted).
- **.COLS()**: Number of columns.
- **.APPEND_ROW(cell, cell, ...)**: A copy with a new row added at the end. The number of cells given must match the existing row width, unless the matrix is empty (just a header).
- **.DELETE_ROW(row)**: A copy with that data row removed.

```basic
PRINT book.GET(1, "Phone")          # "555-0101"
VAR updated = book.SET(1, "Phone", "555-9999")
VAR withCarol = updated.APPEND_ROW("Carol", "555-0199")
VAR withoutAlice = withCarol.DELETE_ROW(1)
```

### Aggregates: SUM, AVERAGE, MIN, MAX, COUNT
These all take one column and read down every data row in it.

- **.SUM(col)** / **.AVERAGE(col)** / **.MIN(col)** / **.MAX(col)**: Strict -- every cell in the column must parse as a number, or the call throws a runtime error naming the offending row and value.
- **.COUNT(col)**: Lenient by design -- counts only the cells that parse as numbers, silently skipping blanks and non-numeric text like `"N/A"`. (This is Excel's actual `COUNT`, not `COUNTA`.)

```basic
VAR scores = (
    ("Name", "Score"),
    ("Alice", "95"),
    ("Bob", "88")
)
PRINT scores.SUM("Score")        # 183
PRINT scores.AVERAGE("Score")    # 91.5

VAR prices = (
    ("Item", "Price"),
    ("Widget", "9.99"),
    ("Gadget", "N/A")
)
PRINT prices.COUNT("Price")      # 1 -- "N/A" is skipped, not an error
```

### Searching and Sorting
- **.FIND(col, value, ignoreCase)**: The 1-based data-row index of the first match in `col`, or `0` if none. Matching compares as text, so `.FIND("Score", 95)` matches a cell holding `"95"`. `ignoreCase` is optional and defaults to off; pass `1` to match regardless of capitalisation, which is usually what you want for names typed by a person.
- **.SORT(col)**: A new Matrix with its data rows sorted ascending by `col`. Auto-detects the comparison: numeric if every cell in that column parses as a number, lexicographic (string) otherwise -- there's no flag to choose, and no descending option.

```basic
PRINT scores.FIND("Name", "Bob")     # 2
PRINT scores.FIND("Name", "bob")     # 0 -- case matters by default
PRINT scores.FIND("Name", "bob", 1)  # 2 -- unless you say otherwise
VAR sorted = scores.SORT("Score")    # ascending by Score
```

### Saving to CSV
- **.SAVE(path)**: Writes the Matrix to `path` as CSV, header row included. See §6 for `READ_CSV`, its counterpart.

### Printing a Matrix
`PRINT`ing (or concatenating with `+`) a Matrix renders it as a table, columns aligned and separated by ` | `, with a `-`-filled rule under the header:
```basic
PRINT scores
```
```
Name  | Score
----- | -----
Alice | 95
Bob   | 88
```

---

## 9. Comments

Anything from a `#` to the end of the line is a comment and is ignored. Comments can sit on their own line or trail after a statement.

```basic
# This line is entirely a comment
VAR x = 10   # so is everything after this '#'
PRINT x
```

---

## 10. Summary of Syntax Rules

1. **Indentation**: Not required for logic, but recommended for readability.
2. **Blocks**:
   - Every block is `{ ... }`, with no alternative spelling. `IF`, `ELSEIF`, `ELSE`, `WHILE`, `FOR`, `FOR EACH`, `FUNCTION`, `TRY` and `CATCH` each take one.
   - Braces mean "block" and nothing else. `( )` groups a sub-expression, carries a call's arguments or a function's parameters, and writes a Matrix literal; `[ ]` writes and indexes a List.
3. **Case Sensitivity**: Keywords (like `IF`, `PRINT`), built-in functions (`LEN`, `SPLIT`) and methods (`.SORT`, `.GET`) are case-**sensitive** and must be written in uppercase. Names you choose yourself are case-sensitive too, but may be written however you like -- `myTotal` and `mytotal` are two different variables.

   A keyword written in the wrong case is a valid identifier, so it would otherwise fail somewhere other than where it was written. Kumu recognises the mistake and says so:

<!-- check
print "hi"
-->
   ```
   Syntax Error: 'print' is not a keyword -- Kumu keywords are written in capitals. Did you mean 'PRINT'? (line 1)
   ```

   This only applies where a statement was expected. `VAR print = 5` and `print = 5` still work, because there a lowercase keyword really is being used as an ordinary name.
4. **Comments**: A `#` starts a comment that runs to the end of the line; see [Section 9](#9-comments).
5. **Error messages**: Kumu tries to say what went wrong, where, and what you probably meant. Every uncaught error -- syntax or runtime -- shows the offending line with a caret under the exact spot that failed:

<!-- check
VAR a = 1
VAR b = 2
PRINT 5 +
-->
   ```
   Syntax Error: Unexpected end of file -- a value was expected here (line 3)

    3 | PRINT 5 +
      |          ^
   ```

   For a runtime error the caret points at the operator or name at fault, not merely at the start of the statement, and any calls in progress follow it:

<!-- check
FUNCTION c(x) {
    RETURN x / 0
}
FUNCTION b(x) {
    RETURN c(x)
}
PRINT b(5)
-->
   ```
   Runtime Error: Division by zero (line 2)

    2 |     RETURN x / 0
      |              ^
     in c() called at line 5
     in b() called at line 7
   ```

   An undefined name suggests the closest one that does exist, and a type error names the types involved:

<!-- check
VAR total = 0
PRINT totl
-->
   ```
   Runtime Error: Undefined variable: totl -- did you mean 'total'? (line 2)
   ```

<!-- check
PRINT "a" - "b"
-->
   ```
   Runtime Error: Cannot use '-' on a String and a String -- only '+' joins text (line 1)
   ```

   A `TRY`/`CATCH` binds only the message text -- no caret, no call stack -- so a program handling an error deliberately isn't handed a debugging report to strip out.

6. **Errors**: Problems in the *text* of a program -- an unexpected character, an unterminated string, a malformed number, a misplaced keyword -- are reported as `Syntax Error` before anything runs. Problems that happen while running -- division by zero, an undefined variable, `INPUT` past end-of-input, recursion past 1000 deep -- are reported as `Runtime Error` with the source line, and stop the program unless caught by a `TRY`/`CATCH` (§4).
7. **Nothing fails silently**: every character in a program is either meaningful or an error. A stray `%` or `;` is reported, not skipped.
8. **Uncaught errors show the call stack**: when a runtime error reaches the top level without being caught, the message is followed by the chain of calls that led to it, innermost first:

<!-- check
FUNCTION c(x) {
    RETURN x / 0
}
FUNCTION b(x) {
    RETURN c(x)
}
FUNCTION a(x) {
    RETURN b(x)
}
PRINT a(5)
-->
   ```
   Runtime Error: Division by zero (line 2)

    2 |     RETURN x / 0
      |              ^
     in c() called at line 5
     in b() called at line 8
     in a() called at line 10
   ```

   A `TRY`/`CATCH` binds only the message, without the call stack -- a program that is handling an error deliberately shouldn't have to strip a debugging trail out of it.
