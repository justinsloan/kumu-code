---
title: Textbook
nav_order: 3
layout: doc
permalink: /textbook/
---

# The Kumu Textbook

*A short, example-driven introduction to the Kumu programming language.*

This textbook is a companion to the [Manual]({{ site.baseurl }}/manual/), Kumu's reference. Where the manual answers "what does this do, precisely?", this book answers "how do I use it, and why does it work this way?" — with a runnable example for nearly every idea. Every example in this book has actually been run through the interpreter; the output shown is real, not guessed.

Kumu is BASIC-inspired: familiar keywords like `PRINT`, `IF`, and `FOR`, but built around **blocks** written in braces — `{ ... }` — instead of classic BASIC's line numbers. Every block in the language looks the same, so there is only ever one shape to learn.

To run any program in this book, save it to a file ending in `.kumu` and run:

```
./kumu program.kumu
```

If `./kumu` doesn't exist yet, run `make` in this directory first.

### How to use this book

It's written to be worked through in order, and it's built for someone learning
on their own. Each chapter follows the same shape:

- **What you'll be able to do** — the goals, up front, so you can tell whether
  you got there.
- **The problem** — why the feature exists, before any syntax. If you can't yet
  see what a chapter is *for*, read this part twice.
- **The teaching**, with a runnable example for every idea.
- **Common mistakes** — the things that actually go wrong, and what Kumu says
  when they do. Reading these before you hit them saves a lot of time.
- **Exercises**, of three kinds. *Predict* asks what a program prints before you
  run it. *Fix* gives you a broken program to repair. *Write* asks you to build
  something from scratch.
- **Recap** — one paragraph, to check against the goals.

Every answer is in [Appendix A](#appendix-a-answers-to-exercises), and every
example and answer in this book has been run through the interpreter — the
output shown is real, not guessed.

Two pieces of advice. **Run the code**: type the examples in and change them,
because reading a program is not the same as watching it behave. And **attempt
the exercises before reading the answers** — especially the *Predict* ones,
where being wrong is far more informative than being right.

---

## Contents

1. [Hello, World](#chapter-1-hello-world)
2. [Variables](#chapter-2-variables)
3. [Expressions and Operators](#chapter-3-expressions-and-operators)
4. [Making Decisions with IF](#chapter-4-making-decisions-with-if)
5. [Getting Input from the User](#chapter-5-getting-input-from-the-user)
6. [Loops](#chapter-6-loops)
7. [Functions](#chapter-7-functions)
8. [Text Processing with String Functions](#chapter-8-text-processing-with-string-functions)
9. [Handling Errors with TRY/CATCH](#chapter-9-handling-errors-with-trycatch)
10. [Debugging with TRACE](#chapter-10-debugging-with-trace)
11. [Random Numbers](#chapter-11-random-numbers)
12. [Console Control: CLEAR_SCREEN and SLEEP](#chapter-12-console-control-clear_screen-and-sleep)
13. [Lists](#chapter-13-lists)
14. [Files and Matrices](#chapter-14-files-and-matrices)
15. [Working with Dates](#chapter-15-working-with-dates)
16. [Comments](#chapter-16-comments)
17. [Putting It Together — A Guessing Game](#chapter-17-putting-it-together--a-guessing-game)

[Where to Go From Here](#where-to-go-from-here)

[Appendix A: Answers to Exercises](#appendix-a-answers-to-exercises)

---

## Chapter 1: Hello, World

**By the end of this chapter you'll be able to:**

- write a Kumu program and run it
- print text and numbers
- recognise an error message and work out what it's telling you

### The problem

A program that can't tell you anything is no use. Before you can do anything
else, you need a way to get something *out* — onto the screen where you can see
it. That's where every language starts, and Kumu is no exception.

### Your first program

A Kumu program is just a sequence of statements, run top to bottom. There's no
`main` function to write, no imports, no semicolons. The simplest statement is
`PRINT`, which evaluates whatever follows it and writes the result to the
screen:

```kumu
PRINT "Hello, World!"
```

Output:
```
Hello, World!
```

That's a complete program. Save it in a file called `hello.kumu` and run it:

```
./kumu hello.kumu
```

If you haven't built the interpreter yet, run `make` first — that produces the
`./kumu` program you just used.

### PRINT does more than text

Anything you can write a value for, `PRINT` can show. Text goes in double
quotes; numbers don't need them, and arithmetic is worked out before printing:

```kumu
PRINT "The answer is:"
PRINT 6 * 7
```

Output:
```
The answer is:
42
```

The quotes matter more than they look. `PRINT "6 * 7"` would print the
characters `6 * 7`, because anything inside quotes is text to be shown exactly
as written. Without quotes it's a sum to be worked out.

### Your first error

Make a mistake on purpose. It's better to meet one now, deliberately, than at
midnight when you're stuck:

```kumu-broken
print "Hello, World!"
```

```
Syntax Error: 'print' is not a keyword -- Kumu keywords are written in capitals. Did you mean 'PRINT'? (line 1)

 1 | print "Hello, World!"
   | ^
```

Kumu keywords are written in capitals, always. Lowercase `print` isn't the
`PRINT` statement — to Kumu it's just a name you happen to have used, the same
as `total` or `score`.

Notice what the message gives you: the problem, the line it's on, and a caret
under the exact spot. Every error does this — not just this one — so you never
have to count lines in an editor to work out what it's talking about.

### Nothing runs until the whole file is read

One thing surprises people the first time. A syntax error stops the program
*before it starts*: Kumu reads the whole file first, so nothing runs at all —
not even the parts above the mistake.

```kumu-broken
PRINT "This line is fine."
PRINT "So is this one."
print "But this one is wrong."
```

```
Syntax Error: 'print' is not a keyword -- Kumu keywords are written in capitals. Did you mean 'PRINT'? (line 3)

 3 | print "But this one is wrong."
   | ^
```

Neither of the first two lines printed anything. They were never reached.

That's deliberate, and it's on your side: a program that got halfway through —
writing a file, say, or updating a total — before discovering it couldn't
understand the rest would leave you worse off than one that refused to start.
The trade-off is that a single typo makes the whole program silent, so if you
run something and see nothing but an error, don't assume the rest of your code
is broken too. Fix the one line and try again.

Two habits worth forming right now:

- **Read the whole message.** It names the thing it choked on, the line it was
  on, and here even what you probably meant.
- **Errors are normal.** Every programmer produces dozens a day. They're the
  language telling you something useful, not a scolding.

### Common mistakes

**Lowercase keywords.** `print`, `Print` and `PRINT` are three different things
to Kumu, and only the last one prints.

**A missing closing quote.** Easy to do and caught clearly:

<!-- check
PRINT "Hello
-->
```
Syntax Error: Unterminated string -- it is missing a closing '"' (line 1)

 1 | PRINT "Hello
   |       ^
```

**Forgetting to rebuild.** If you change the interpreter's C++ source rather
than your `.kumu` file, run `make` again. Editing a `.kumu` program needs no
rebuild — just run it.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What do these two lines print? They are not the same.

```kumu
PRINT "6 * 7"
PRINT 6 * 7
```

**2. (Fix)** What's wrong with this, and what does Kumu say about it?

```kumu-broken
print "Kumu"
```

**3. (Fix)** And this one? Read the error it produces before deciding.

```kumu-broken
PRINT "Hello
```

**4. (Write)** Write a program that prints your name on one line and your
favourite number on the next. Use two `PRINT` statements.

### Recap

`PRINT` shows a value on the screen. Text goes in double quotes and appears
exactly as written; numbers and arithmetic don't, and get worked out first.
Keywords are always capitals. Run a program with `./kumu yourfile.kumu`, and
when an error appears, read it — it's information, not failure.

---

## Chapter 2: Variables

**By the end of this chapter you'll be able to:**

- store a value under a name and use it later
- change what a name holds
- name the four kinds of value Kumu has
- explain why a misspelled variable name is dangerous in Kumu

### The problem

Everything in Chapter 1 was forgotten the instant it was printed. That's fine
for one line of output, but useless the moment you want to compute something and
*use it again* — a total you keep adding to, a name you asked the user for, a
price you need twice.

You need somewhere to put a value and a way to refer to it. That's a
**variable**: a name that holds a value.

### Naming a value

`VAR` introduces a new name and gives it a value:

```kumu
VAR name = "Kumu"
VAR version = 1
PRINT name
PRINT version
```

Output:
```
Kumu
1
```

From then on, writing `name` means "whatever value is in `name` right now".
That last phrase is the important one — a variable is not a fixed label, it's a
box whose contents can change:

```kumu
VAR score = 10
PRINT score
score = score + 5
PRINT score
```

Output:
```
10
15
```

Read `score = score + 5` right to left: work out `score + 5` using the current
value, then put the answer back into `score`. It isn't a claim that `score`
equals `score + 5` — that would be nonsense. It's an instruction.

### VAR is a courtesy, not a requirement

You can assign to a name without `VAR`, and it works:

```kumu
VAR declared = 1
undeclared = 2
PRINT declared + undeclared
```

Output:
```
3
```

`VAR` and plain assignment do exactly the same thing. `VAR` is a signal to
whoever reads your program that *this is the first time* the name is used.
Kumu doesn't enforce it.

Use `VAR` the first time anyway. Chapter 10 shows a bug that is very hard to
find precisely because Kumu doesn't insist on it.

### The four kinds of value

| Kind | Looks like | Covered in |
| :--- | :--- | :--- |
| **Number** | `10`, `3.14`, `-5` | this chapter |
| **String** | `"like this"` | Chapter 8 |
| **List** | `[1, 2, 3]` | Chapter 13 |
| **Matrix** | a table of text | Chapter 14 |

There's no separate true/false type: `1` means true and `0` means false
throughout the language.

A name isn't tied to one kind. This is legal, and Kumu won't complain:

```kumu
VAR thing = 42
PRINT thing
thing = "now I'm text"
PRINT thing
```

Output:
```
42
now I'm text
```

That flexibility is convenient and occasionally dangerous — nothing stops you
putting a String where the rest of your program expects a Number.

### Common mistakes

**Using a name before giving it a value.** Kumu catches this:

<!-- check
PRINT total
-->
```
Runtime Error: Undefined variable: total (line 1)
```

**Misspelling a name you're reading.** Also caught — and Kumu looks for a name
you did define that's close to what you typed:

<!-- check
VAR total = 0
PRINT totl
-->
```
Runtime Error: Undefined variable: totl -- did you mean 'total'? (line 2)
```

**Misspelling a name you're *writing*.** This one is *not* caught:

```kumu
VAR total = 0
totl = total + 10
PRINT total
```

Output:
```
0
```

No error. `totl` is simply a new variable that now holds `10`, while `total` sits
untouched at `0`. Kumu has no way to know you meant the other one. This is the
single most dangerous thing in this chapter, and Chapter 10 is largely about
finding it when it happens to you.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does this print?

```kumu
VAR x = 5
VAR y = x
x = 20
PRINT x
PRINT y
```

**2. (Predict)** And this?

```kumu
VAR count = 1
count = count + count
count = count + count
PRINT count
```

**3. (Fix)** This should print `30`, but it prints `10`. Why?

```kumu
VAR price = 10
prise = price * 3
PRINT price
```

**4. (Write)** Store a rectangle's width (`8`) and height (`5`) in two
variables, then print its area and its perimeter, each on its own line, using
those variables rather than the numbers.

**5. (Write)** Put `"first"` in a variable `a` and `"second"` in a variable `b`,
then swap them so `a` holds `"second"` and `b` holds `"first"`, and print both.
You'll need a third variable — work out why.

### Recap

A variable is a name holding a value you can read back and change later.
`VAR` marks the first use and is a courtesy to the reader, not a rule Kumu
enforces. Values come in four kinds and a name can hold any of them, even a
different one later. Reading a name that was never set is an error; *writing* a
misspelled name silently creates a new variable instead — which is why spelling
matters more here than in languages that make you declare things.

---

## Chapter 3: Expressions and Operators

**By the end of this chapter you'll be able to:**

- combine values with arithmetic and join text with `+`
- compare values, and combine those comparisons with `AND`, `OR` and `NOT`
- predict what precedence does, and reach for parentheses when it isn't what you want
- avoid the `=` / `==` trap that catches every beginner

### The problem

Storing values isn't much use on its own. The work happens when you combine
them — add a tax to a price, check whether a score passed, join a greeting to a
name. That's what **operators** are for, and you'll use them inside nearly every
statement you write from here on.

### Arithmetic

`+`, `-`, `*` and `/` work the way you'd expect, including precedence: `*` and
`/` bind more tightly than `+` and `-`. Parentheses override that:

```kumu
PRINT 2 + 3 * 4
PRINT (2 + 3) * 4
```

Output:
```
14
20
```

When operators bind equally tightly, they're applied left to right, which
matters more often than people expect:

```kumu
PRINT 10 - 2 - 3
PRINT 20 / 2 * 5
```

Output:
```
5
50
```

### Joining text

`+` also joins strings together. Mix a string with a number and the number is
converted to text automatically, which is how most output gets built:

```kumu
VAR name = "Ada"
VAR age = 36
PRINT "Hi " + name
PRINT name + " is " + age + " years old"
```

Output:
```
Hi Ada
Ada is 36 years old
```

### Comparisons

`==`, `!=`, `<`, `>`, `<=` and `>=` all produce `1` for true or `0` for false —
Kumu has no separate true/false type:

```kumu
PRINT 5 > 3
PRINT 5 == 3
PRINT "abc" == "abc"
```

Output:
```
1
0
1
```

Unary `-` negates a number, and unary `!` flips truthiness — `0` becomes `1`,
anything else becomes `0`:

```kumu
PRINT -5
PRINT !0
PRINT !7
```

Output:
```
-5
1
0
```

"Truthiness" is worth a moment. Anywhere a condition is expected, Kumu treats
`0` and the empty string `""` as false, and everything else as true.

One rule worth knowing early: comparing two *different* types with `==`/`!=` is
allowed, and simply always says "not equal" — it doesn't crash. That matters
once you start reading user input in Chapter 5, since you won't always know
whether what you're comparing is a Number or a String.

### `=` is not `==`

`=` assigns a value; `==` compares two. They're easy to mix up, and mixing them
up is the single most common beginner mistake in any language. Kumu still
accepts `IF x = 5 {` and reads it as a comparison, but it warns you:

<!-- check
IF 1 = 1 {
    PRINT "x"
}
-->
```
Warning (line 1): '=' used as a comparison -- did you mean '=='?
```

Take the warning seriously. Write `==` in every condition.

### MOD: the remainder

`MOD` gives what's left over after a division. `7 MOD 2` is `1`, because 2 goes
into 7 three times with 1 left over. It binds as tightly as `*` and `/`.

That one operator answers a surprising number of questions. "Is this number
even?" is `n MOD 2 == 0`. "Is it a multiple of 10?" is `n MOD 10 == 0`. You'll
use it constantly once you reach loops.

### AND, OR, NOT

Comparisons answer one question at a time. `AND` and `OR` combine those answers,
and `NOT` flips one:

```kumu
VAR age = 25
VAR hasTicket = 1

IF age >= 18 AND hasTicket {
    PRINT "Come in"
}

IF age < 5 OR age > 65 {
    PRINT "Discount applies"
}

IF NOT hasTicket {
    PRINT "Please buy a ticket"
}
```

Output:
```
Come in
```

`AND` binds more tightly than `OR`, so `a AND b OR c` means `(a AND b) OR c`.
Use parentheses when you want the other grouping.

There's a second, less obvious thing these two do, and it's genuinely useful.
They **short-circuit**: `AND` never looks at its right-hand side once the left is
false, and `OR` never looks once the left is true. Why that matters:

```kumu
VAR count = 0
VAR total = 0

IF count != 0 AND total / count > 5 {
    PRINT "above average"
} ELSE {
    PRINT "no data yet"
}
```

Output:
```
no data yet
```

`total / count` would be a division by zero — but it never runs, because
`count != 0` was already false and `AND` stopped there. Putting the guard on the
left of an `AND` is a standard and very worthwhile habit.

### Common mistakes

**`=` where you meant `==`.** You get a warning, not an error, so it's easy to
scroll past. Don't.

**Dividing by zero.** Caught, both ways:

<!-- check
PRINT 1 / 0
-->
```
Runtime Error: Division by zero (line 1)
```

<!-- check
PRINT 1 MOD 0
-->
```
Runtime Error: Division by zero in MOD (line 1)
```

**Expecting `-` to work on text.** `+` joins strings, but nothing else does:

<!-- check
PRINT "a" - "b"
-->
```
Runtime Error: Cannot use '-' on a String and a String -- only '+' joins text (line 1)
```

**Ordering two different types.** `==` and `!=` tolerate a mismatch and answer
"not equal". `<`, `>`, `<=` and `>=` do not:

<!-- check
PRINT 5 < "abc"
-->
```
Runtime Error: Cannot use '<' on a Number and a String -- both sides must be the same type, and Matrices cannot be ordered (line 1)
```

**Expecting `"5" == 5`.** It's `0`. The text `"5"` and the number `5` are
different values. Chapter 8 shows how to convert between them.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does each line print?

```kumu
PRINT 1 + 2 * 3
PRINT 100 / 10 / 2
PRINT 7 MOD 3
PRINT 1 + 6 MOD 4
```

**2. (Predict)** And these?

```kumu
PRINT 5 == "5"
PRINT 5 != "5"
PRINT !""
PRINT 0 OR 3
```

**3. (Fix)** This prints a warning before it prints anything else. What's the
warning, and what should the line be?

```kumu
VAR total = 10
IF total = 10 {
    PRINT "ten"
}
```

**4. (Fix)** This crashes with `Division by zero`. Rewrite the condition so it
prints `no items` instead, without adding a second `IF`.

```kumu
VAR items = 0
VAR cost = 100
IF cost / items > 10 {
    PRINT "expensive"
} ELSE {
    PRINT "no items"
}
```

**5. (Write)** Given `VAR n = 7`, print `even` or `odd` using `MOD`.

**6. (Write)** A year is a leap year if it's divisible by 4, **except** that
years divisible by 100 are not — **unless** they're also divisible by 400. So
2024 and 2000 are leap years; 1900 is not. Given `VAR year = 2000`, print
`leap` or `not leap`. You'll want `MOD`, `AND`, `OR` and some parentheses.

### Recap

Operators combine and compare values. Arithmetic follows normal precedence and
applies left to right within a tier; `+` doubles as text-joining. Comparisons
give `1` or `0`, and `AND`/`OR`/`NOT` combine them — short-circuiting, which
lets you put a guard on the left and rely on it. `MOD` gives remainders and
answers more questions than it looks like it should. And `=` assigns while `==`
compares: if you see the warning, fix the line.

---

## Chapter 4: Making Decisions with IF

**By the end of this chapter you'll be able to:**

- run one piece of code or another depending on a condition
- chain several conditions with `ELSEIF` and get the order right
- explain what counts as true in Kumu
- avoid the two mistakes that make an `IF` chain give the wrong answer

### The problem

Everything so far has run straight through, top to bottom, doing the same thing
every time. Real programs don't. A grade depends on a score; a discount depends
on an age; a message depends on whether the user typed anything. The program has
to *choose*.

`IF` is that choice.

### IF and ELSE

`IF` evaluates a condition and runs one of two blocks:

```kumu
VAR temperature = 75
IF temperature > 80 {
    PRINT "It's hot!"
} ELSE {
    PRINT "It's pleasant."
}
```

Output:
```
It's pleasant.
```

The `ELSE` is optional — leave it off when there's nothing to do in the other
case:

```kumu
VAR errors = 3
IF errors > 0 {
    PRINT "There were problems."
}
PRINT "Done."
```

Output:
```
There were problems.
Done.
```

Notice that `PRINT "Done."` is outside the braces, so it runs either way. What's
inside the braces is the only thing the condition controls.

### What counts as true

The condition doesn't have to be a comparison. Kumu treats `0` and the empty
string `""` as false, and everything else as true — so a count, a name, or a
flag can be tested directly:

```kumu
VAR name = ""
IF name {
    PRINT "Hello, " + name
} ELSE {
    PRINT "I don't know your name."
}
```

Output:
```
I don't know your name.
```

### Braces are never optional

Each branch is a braced block, even for a single statement. Some languages let
you leave the braces off when the body is one line; Kumu doesn't, because that
shortcut is a classic source of bugs. Add a second line later, forget to add the
braces, and it silently isn't part of the branch any more.

### More than two outcomes

Chain as many `ELSEIF`s as you need onto one `IF`:

```kumu
VAR score = 72
IF score >= 90 {
    PRINT "A"
} ELSEIF score >= 80 {
    PRINT "B"
} ELSEIF score >= 70 {
    PRINT "C"
} ELSE {
    PRINT "F"
}
```

Output:
```
C
```

The chain stops at the **first** branch whose condition is true — the rest are
never even checked. That's what makes the ordering above work: by the time
Kumu reaches `score >= 70`, it already knows the score is under 80.

Get that order wrong and the chain quietly gives wrong answers. If the branches
were listed lowest-first, every score above 70 would print `C`, because `C`'s
test would match first and nothing after it would run. **In a chain of
overlapping conditions, put the most specific first.**

Under the hood, `ELSEIF` is just `ELSE { IF ... }` written for you — each
`ELSEIF` is really an `IF` nested one level inside the previous branch's `ELSE`,
which is exactly how you'd have had to write a multi-way decision without it.

### Common mistakes

**`=` instead of `==`.** From Chapter 3, and it bites hardest here:

<!-- check
VAR total = 10
IF total = 10 {
    PRINT "ten"
}
-->
```
Warning (line 2): '=' used as a comparison -- did you mean '=='?
```

**Overlapping conditions in the wrong order.** No error, no warning — just wrong
answers. See above.

**`ELSE IF` with a space.** It has to be one word. `ELSE IF` is two separate
keywords: an `ELSE` branch whose body happens to begin with its own, separately
braced `IF`. That usually still works, but it nests a level deeper with each
branch and needs its own closing brace, so a long chain becomes a staircase.

**Testing a String against a Number.** `IF answer == 5` is false when the user
typed text. Chapter 5 explains why that matters for input.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does this print?

```kumu
VAR n = 0
IF n {
    PRINT "truthy"
} ELSE {
    PRINT "falsy"
}
```

**2. (Predict)** And this? Read the branch order carefully.

```kumu
VAR score = 95
IF score >= 70 {
    PRINT "C"
} ELSEIF score >= 80 {
    PRINT "B"
} ELSEIF score >= 90 {
    PRINT "A"
}
```

**3. (Fix)** Exercise 2 is meant to print `A`. Rewrite the chain so it does,
without changing the score.

**4. (Write)** Given `VAR age = 12`, print `child` for under 13, `teen` for 13
to 19, and `adult` for 20 and over. Use one `IF`/`ELSEIF`/`ELSE` chain.

**5. (Write)** Given `VAR hour = 14` (a 24-hour clock), print `Good morning` for
hours under 12, `Good afternoon` for 12 to 17, and `Good evening` otherwise.

### Recap

`IF` chooses between blocks based on a condition, where `0` and `""` are false
and everything else is true. `ELSE` is optional; braces are not. An
`IF`/`ELSEIF` chain runs the first matching branch and skips the rest, so when
conditions overlap, the most specific one has to come first — get that backwards
and you get wrong answers with no error to warn you.

---

## Chapter 5: Getting Input from the User

**By the end of this chapter you'll be able to:**

- read what someone types into a variable
- work out whether you were given a Number or a String
- explain why `INPUT` is the place type confusion usually starts

### The problem

Every program so far has had its values written into it. Change the temperature,
edit the file. That's fine for learning, but a program nobody can talk to isn't
much of a program — you want to ask a question and use the answer.

### INPUT

`INPUT` reads one line of what the user types and stores it in a variable:

```kumu
PRINT "What is your name?"
INPUT userName
PRINT "Nice to meet you, " + userName + "!"
```

A sample run, typing `Ada` at the prompt:
```
What is your name?
Ada
Nice to meet you, Ada!
```

`INPUT` on its own prints nothing, so always `PRINT` a question first. A program
that sits silently waiting looks broken.

### What type did you get?

This is the part that catches people. `INPUT` has no type keyword — it decides
for you. If the whole line looks like a number, you get a **Number**. Anything
else, including an empty line, stays a **String**.

That means the same program behaves differently depending on what was typed:

```kumu
PRINT "Enter a number:"
INPUT n
PRINT n + 1
```

Type `42` and it prints `43` — number plus number. Type `abc` and it prints
`abc1`, because `+` joins text when either side is a String. No error, just a
surprising answer.

When it matters, ask:

```kumu
PRINT "Enter your age:"
INPUT age
IF IS_NUMBER(age) {
    PRINT "Next year you will be " + (NUMBER(age) + 1)
} ELSE {
    PRINT "That was not a number."
}
```

`IS_NUMBER` answers "could this be used as a number?", and `NUMBER` converts it.
Both are covered properly in Chapter 8; this is the pattern worth copying until
then.

### When the input runs out

If there's nothing left to read — the user closed the terminal, or a script
piped in fewer lines than the program asked for — `INPUT` stops the program with
an error:

```
Runtime Error: INPUT: no more input available (end of input) (line 2)
```

That's deliberate. The alternative would be handing your program an endless
stream of empty lines, and a loop reading input would spin forever. Chapter 9
shows how to catch this and exit gracefully instead.

### Common mistakes

**No prompt.** `INPUT` prints nothing by itself. Always ask first.

**Assuming you got a Number.** Everything above. If the user might type
anything, check with `IS_NUMBER` before doing arithmetic.

**Comparing a typed value to a Number.** If the user typed `yes`, then
`answer == 1` is `0` — different types are never equal. Compare against what
you'll actually receive: `answer == "yes"`.

**Reading more lines than exist.** Ends the program. Chapter 9 has the fix.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** This program is run twice — once typing `10`, once typing
`ten`. What does it print each time?

```kumu
INPUT value
PRINT value + 5
```

**2. (Predict)** The user types `yes`. What does this print, and why?

```kumu
INPUT answer
IF answer == 1 {
    PRINT "confirmed"
} ELSE {
    PRINT "not confirmed"
}
```

**3. (Write)** Ask for someone's name, then their town, then print
`<name> is from <town>` on one line.

**4. (Write)** Ask for a number and print whether it's even or odd. Guard
against the user typing something that isn't a number — print `not a number` in
that case. (You'll want `IS_NUMBER`, `NUMBER` and `MOD`.)

### Recap

`INPUT` reads one line into a variable, and decides its type by looking at it: a
line that reads as a number becomes a Number, everything else stays a String.
Always print a prompt first, and never assume what you got — `IS_NUMBER` and
`NUMBER` are how you check and convert. Running out of input is an error rather
than silence, which Chapter 9 shows how to handle.

---

## Chapter 6: Loops

**By the end of this chapter you'll be able to:**

- recognise a problem that calls for repetition
- write a `WHILE` loop and a `FOR` loop, and say which one a problem wants
- explain why a loop runs forever, and fix one that does
- leave a loop early with `BREAK`, or skip one pass with `CONTINUE`

### The problem

Print the numbers 1 to 100.

You could write a hundred `PRINT` statements. Nobody does that — not because it's
slow to type, but because the moment you want 1 to 1000 instead, or want to skip
the even ones, you'd start again from scratch. What you actually want is to say
"do this, repeatedly, until you're done", once.

That's a **loop**: a block of statements that runs more than once.

### Every loop answers three questions

This is the idea to hold on to, because it's true in every programming language,
not just Kumu:

1. **What's true before it starts?** — the counter's starting value, the total
   set to zero
2. **What keeps it going?** — the condition that's checked before each pass
3. **What changes each time?** — the thing that eventually makes the condition
   false

Get all three right and the loop works. Almost every loop bug is one of the
three being missing or wrong — and the worst of them is a missing number 3,
which we'll come to shortly.

### WHILE

`WHILE` runs its body for as long as its condition holds, checking fresh before
every pass:

```kumu
VAR count = 5
WHILE count > 0 {
    PRINT count
    count = count - 1
}
PRINT "Liftoff!"
```

Output:
```
5
4
3
2
1
Liftoff!
```

Find the three questions in it. Before the loop, `count` is `5`. The condition
`count > 0` keeps it going. And `count = count - 1` is what changes — without
that line, `count` would stay at `5`, the condition would stay true, and the
loop would never stop.

Notice that `PRINT "Liftoff!"` runs once, after the loop. The loop ends when the
condition is checked and found false, so by the time you reach that line `count`
is `0`.

### When a loop never ends

This is the mistake every programmer makes, usually more than once:

```kumu
VAR count = 5
WHILE count > 0 {
    PRINT count
}
```

The update is missing. `count` is `5` forever, `count > 0` is true forever, and
the program prints `5` until you stop it.

Kumu will not warn you about this. There's no error message, because there's
nothing detectably wrong — you asked it to repeat while a condition holds, and
the condition holds. The program simply never finishes.

If a program of yours hangs and prints the same thing over and over, look at
your loop and ask question 3: *what changes each time?* Press `Ctrl-C` to stop
it.

### FOR

Counting is so common that it gets its own form. `FOR` counts a variable from a
start value to an end value:

```kumu
VAR total = 0
FOR i = 1 TO 10 {
    total = total + i
}
PRINT total
```

Output:
```
55
```

Here's what's worth noticing: `FOR i = 1 TO 10` states all three answers on one
line. Start at 1. Keep going while `i` is at most 10. Add 1 each time. The
update you can forget in a `WHILE` is built in, which is exactly why `FOR`
exists — it's a `WHILE` loop with the parts you're most likely to get wrong
written down for you.

So the choice between them is simple:

- **Use `FOR`** when you know how many times, or are counting through a range.
- **Use `WHILE`** when you don't — when you're waiting for something to become
  true and can't say in advance how long that takes.

### STEP

By default `FOR` counts up by exactly 1. `STEP` changes that, and a negative
step counts **down**:

```kumu
FOR i = 1 TO 10 STEP 3 {
    PRINT i
}

FOR i = 3 TO 1 STEP -1 {
    PRINT i
}
```

Output:
```
1
4
7
10
3
2
1
```

Counting down *requires* `STEP -1`. Plain `FOR i = 10 TO 1` does not count
backwards — it runs zero times, because the counter only ever moves up and it
starts out already past the end. Nothing is printed and no error appears, which
makes it a genuinely confusing thing to get wrong. If you mean downwards, say
`STEP -1`.

### BREAK and CONTINUE

Sometimes you want out of a loop before its condition says so, or want to skip
the rest of one pass. `BREAK` leaves the loop entirely; `CONTINUE` jumps ahead
to the next pass:

```kumu
FOR i = 1 TO 10 {
    IF i MOD 2 == 0 {
        CONTINUE
    }
    IF i > 7 {
        BREAK
    }
    PRINT i
}
```

Output:
```
1
3
5
7
```

Walk it one pass at a time. Even numbers hit `CONTINUE` and print nothing. Odd
numbers up to 7 reach the `PRINT`. At `i` of 9 the second `IF` fires, `BREAK`
ends the loop, and `i` never reaches 10.

`BREAK` is what you want for a search: stop as soon as you've found the thing,
rather than uselessly checking the rest.

Both only make sense inside a loop. Using either outside one is an error, and so
is using one inside a function that a loop happens to call — a `BREAK` reaching
out of a function to end its caller's loop would be very hard to reason about.

### Putting it together: FizzBuzz

A loop, `MOD` from Chapter 3, and an `IF`/`ELSEIF` chain from Chapter 4 are
enough for the classic first-programming exercise. Count to 15, but say "Fizz"
for multiples of 3, "Buzz" for multiples of 5, and "FizzBuzz" for both:

```kumu
FOR i = 1 TO 15 {
    IF i MOD 15 == 0 {
        PRINT "FizzBuzz"
    } ELSEIF i MOD 3 == 0 {
        PRINT "Fizz"
    } ELSEIF i MOD 5 == 0 {
        PRINT "Buzz"
    } ELSE {
        PRINT i
    }
}
```

Output:
```
1
2
Fizz
4
Buzz
Fizz
7
8
Fizz
Buzz
11
Fizz
13
14
FizzBuzz
```

The order of the branches is the whole trick. `15` is a multiple of 3 *and* of
5, so its test has to come first — an `IF`/`ELSEIF` chain stops at the first
branch that matches, so if `i MOD 3` were checked first, 15 would print "Fizz"
and never reach the "FizzBuzz" case.

### Common mistakes

**The loop never ends.** Something in the body has to make the condition
eventually false. No error, no message — the program just runs forever. This is
question 3.

**A countdown prints nothing.** `FOR i = 10 TO 1` runs zero times. You want
`FOR i = 10 TO 1 STEP -1`. Again, no error — silence is the symptom.

**`STEP` of zero.** This one *is* caught:

<!-- check
FOR i = 1 TO 5 STEP 0 {
    PRINT i
}
-->
```
Runtime Error: FOR STEP cannot be 0 -- the loop would never end (line 1)
```

**`BREAK` outside a loop.** Also caught:

<!-- check
BREAK
-->
```
Runtime Error: BREAK used outside of a loop
```

Notice which mistakes Kumu can catch and which it can't. A `STEP` of zero is
impossible to mean, so it's an error. A loop that never ends might be exactly
what you wanted — a program that runs until you stop it is a real thing — so
Kumu can't judge it for you. When a language stays silent, the check has to be
yours.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does this print? All of it — don't forget the last line.

```kumu
VAR n = 3
WHILE n > 0 {
    PRINT n
    n = n - 1
}
PRINT n
```

**2. (Predict)** And this one?

```kumu
FOR i = 1 TO 5 {
    IF i == 2 {
        CONTINUE
    }
    IF i == 4 {
        BREAK
    }
    PRINT i
}
```

**3. (Fix)** This is meant to print 1, 2, 3. It prints `1` forever instead. Why,
and what line is missing?

```kumu
VAR i = 1
WHILE i <= 3 {
    PRINT i
}
```

**4. (Fix)** This is meant to count down from 5. It prints nothing at all. Why?

```kumu
FOR i = 5 TO 1 {
    PRINT i
}
```

**5. (Write)** Print the 7 times table, from `7 x 1 = 7` through `7 x 12 = 84`.
One line each. (Hint: `PRINT` joins text and numbers with `+`.)

**6. (Write)** Starting from 1 and doubling each time (1, 2, 4, 8, …), print
every value that is 1000 or less. Use a `WHILE` loop — you don't know in advance
how many doublings that takes, which is exactly when `WHILE` is the right
choice.

### Recap

A loop repeats a block. Every one of them answers three questions — what's true
at the start, what keeps it going, what changes each pass — and `FOR` exists
because it writes all three on one line so you can't forget the third. Use `FOR`
when you know the range, `WHILE` when you're waiting for a condition. `BREAK`
leaves early; `CONTINUE` skips a pass. And when a loop misbehaves silently,
suspect question 3 first.

---

## Chapter 7: Functions

**By the end of this chapter you'll be able to:**

- give a piece of work a name and call it from anywhere
- pass values in with parameters and get one back with `RETURN`
- explain what a function's own variables can and can't see
- write a recursive function, and recognise one that will never stop

### The problem

Chapter 6 let you repeat a block. But repetition isn't the only kind of
duplication — often the same *idea* appears in several places that aren't next
to each other. Converting a temperature here, and again there. Checking whether
a name is valid in three different menus.

Copying those lines around is how programs rot: when the rule changes, you have
to find every copy. What you want is to write it once, give it a name, and refer
to the name.

That's a **function**. It's the single most important tool in this book for
keeping a program from collapsing under its own weight as it grows.

### Defining and calling

`FUNCTION` defines one; `RETURN` sends a value back:

```kumu
FUNCTION square(n) {
    RETURN n * n
}
PRINT square(7)
```

Output:
```
49
```

`n` is a **parameter** — a name for whatever value the caller passes in. Calling
`square(7)` runs the body with `n` holding `7`.

The parameter list is required even when it's empty, and a function that never
runs `RETURN` evaluates to `0`:

```kumu
FUNCTION shout() {
    PRINT "HEY"
}
shout()
```

Output:
```
HEY
```

A bare `RETURN` with no value also gives `0`, and is the tidy way to leave
early:

```kumu
FUNCTION greet(name) {
    IF name == "" {
        RETURN
    }
    PRINT "Hello, " + name
}

greet("Ada")
greet("")
PRINT "done"
```

Output:
```
Hello, Ada
done
```

### Every call gets its own variables

**This is the rule worth learning carefully.** Every call gets its own scope:
its parameters, and anything it declares with `VAR`, live there and vanish when
the call returns.

```kumu
VAR n = "outer"
FUNCTION show(n) {
    PRINT n
}
show("inner")
PRINT n
```

Output:
```
inner
outer
```

The function's `n` and the global `n` are different variables that happen to
share a name. Nothing the function does to its own can reach the outer one.

### Reaching out on purpose

Bare assignment — without `VAR` — breaks that isolation deliberately. It doesn't
create a local; it updates an existing variable, walking out to the global scope
if the current call doesn't have one of its own:

```kumu
VAR total = 0
FUNCTION addToTotal(amount) {
    total = total + amount
}
addToTotal(10)
addToTotal(20)
PRINT total
```

Output:
```
30
```

`addToTotal` never declares its own `total`, so its bare assignment reaches
straight through to the global one. This is used constantly later in this book —
the guessing game in Chapter 17, and `address.kumu`'s menu handlers, are full of
zero-parameter functions that change shared state this way. It's a deliberate
design, not a workaround.

It's also the thing that makes a misspelling dangerous, exactly as in Chapter 2.
Write `totl = totl + amount` and you don't get an error; you get a brand new
global that nothing else reads. Chapter 10 is about finding that.

### Order doesn't matter

Kumu reads all your function definitions before running the first line, so a
function can be called from above the line that defines it:

```kumu
PRINT double(21)

FUNCTION double(n) {
    RETURN n * 2
}
```

Output:
```
42
```

That's also what lets two functions call *each other* — otherwise whichever you
defined first would refer to one that didn't exist yet.

### Recursion

A function may call itself. `factorial(6)` calls `factorial(5)`, which calls
`factorial(4)`, down to a **base case** that returns without recursing:

```kumu
FUNCTION factorial(n) {
    IF n <= 1 {
        RETURN 1
    } ELSE {
        RETURN n * factorial(n - 1)
    }
}
PRINT factorial(6)
```

Output:
```
720
```

This only works because of the scope rule above: every nested call gets its
*own* `n`, even though they're all written as the same name. Six calls are alive
at once, each holding a different value.

Every recursive function needs two things — a base case that stops, and a
recursive step that moves *towards* it. Miss either and it never ends.

### Common mistakes

**A missing or unreachable base case.** Kumu stops it at 1000 nested calls:

<!-- check
FUNCTION f(n) {
    RETURN f(n + 1)
}
PRINT f(1)
-->
```
Runtime Error: Maximum recursion depth (1000) exceeded -- is this function missing a base case?
```

When you see that, look at the base case first: is the condition right, and does
each call actually get closer to it?

**Forgetting the parameter parens.** They're required even when empty:

<!-- check
FUNCTION greet {
    PRINT "hi"
}
-->
```
Syntax Error: Expected '(' to start the parameter list -- a function with no parameters is written `FUNCTION name()` (line 1)
```

**Calling with the wrong number of arguments.** Caught, and it names both counts:

<!-- check
FUNCTION square(n) {
    RETURN n * n
}
PRINT square(1, 2)
-->
```
Runtime Error: Function 'square' expects 1 argument(s), got 2 (line 4)
```

**Expecting a value when there's no `RETURN`.** You get `0`, not an error.

**Defining the same name twice.** An error, so a typo'd duplicate is caught
rather than one silently winning.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does this print? The second line is the interesting one.

```kumu
FUNCTION bump(x) {
    VAR x = x + 10
    RETURN x
}
VAR x = 10
PRINT bump(x)
PRINT x
```

**2. (Predict)** And this one?

```kumu
FUNCTION mystery(n) {
    IF n <= 0 {
        RETURN 0
    }
    RETURN n + mystery(n - 1)
}
PRINT mystery(4)
```

**3. (Fix)** This never finishes and ends with a recursion-depth error. What's
missing?

```kumu
FUNCTION countdown(n) {
    PRINT n
    RETURN countdown(n - 1)
}
countdown(3)
```

**4. (Write)** Write `celsius(f)` that converts Fahrenheit to Celsius using
`(f - 32) * 5 / 9`, and print the result for `212`, `98.6` and `32`.

**5. (Write)** Write `isEven(n)` that returns `1` or `0`, then use it in an `IF`
to print whether `18` is even.

**6. (Write)** Write `power(base, exponent)` using recursion — no `POW`, no
loop. `power(2, 10)` should give `1024`. (Base case: anything to the power `0`
is `1`.)

### Recap

A function names a piece of work so it can be written once and used anywhere.
Parameters carry values in, `RETURN` carries one back, and a function with no
`RETURN` gives `0`. Every call gets its own variables, which is what makes
recursion work; bare assignment deliberately reaches outward to shared state,
which is powerful and is also why spelling matters. Definitions can appear in
any order. And every recursive function needs a base case it actually reaches.

---

## Chapter 8: Text Processing with String Functions

**By the end of this chapter you'll be able to:**

- measure, slice and search text
- clean up what a user typed before relying on it
- convert safely between text and numbers
- reach for the arithmetic that `+ - * /` doesn't cover

Once you're past `+` for concatenation, Kumu gives you a set of string tools, all 1-indexed. Five for inspecting and converting — `LEN`, `SUBSTRING`, `INDEX_OF`, `UPPER`, `LOWER`:

```kumu
VAR sentence = "The quick brown fox"
PRINT LEN(sentence)
PRINT UPPER(sentence)
PRINT SUBSTRING(sentence, 5, 5)
PRINT INDEX_OF(sentence, "brown")
```

Output:
```
19
THE QUICK BROWN FOX
quick
11
```

`SUBSTRING(sentence, 5, 5)` means "starting at character 5, take up to 5 characters" — and `INDEX_OF` returns `0` if the search text isn't found at all, the same "0 means not found" convention you'll see again with Lists in Chapter 13 and Matrix lookups in Chapter 14.

### Cleaning up and asking questions

Five more handle the jobs that come up constantly once real text is involved — especially text a person typed:

```kumu
VAR reply = "  Yes, please  "
PRINT "[" + TRIM(reply) + "]"
PRINT CONTAINS(reply, "Yes")
PRINT STARTS_WITH(TRIM(reply), "Yes")
PRINT ENDS_WITH(TRIM(reply), "please")
PRINT REPLACE("2026-09-17", "-", "/")
```

Output:
```
[Yes, please]
1
1
1
2026/09/17
```

`TRIM` is the one you'll reach for most: text from `INPUT` very often arrives with stray spaces, and `"yes "` is not equal to `"yes"`. `REPLACE` swaps *every* occurrence, not just the first.

### Turning text into numbers

`INPUT` gives you a Number when the user typed digits and a String otherwise — which means a program can't simply assume. Three functions settle the question:

```kumu
VAR typed = "42"
PRINT IS_NUMBER(typed)
PRINT IS_NUMBER("hello")
PRINT NUMBER(typed) + 8
PRINT STRING(3.5) + "!"
```

Output:
```
1
0
50
3.5!
```

Always check with `IS_NUMBER` before calling `NUMBER`, because `NUMBER("hello")` is a runtime error. (`"nan"` and `"inf"` count as text too, not numbers — Kumu arithmetic can't produce those values, so text spelling them stays text.) The pattern is worth memorising:

```kumu
INPUT age
IF IS_NUMBER(age) {
    PRINT "Next year you'll be " + (NUMBER(age) + 1)
} ELSE {
    PRINT "That wasn't a number."
}
```

### Doing arithmetic

Rounding out the toolkit, `ABS`, `FLOOR`, `ROUND`, `SQRT`, and `POW` cover the arithmetic that `+ - * /` and `MOD` don't:

```kumu
PRINT ABS(-7)
PRINT FLOOR(3.9)
PRINT ROUND(2.5)
PRINT SQRT(144)
PRINT POW(2, 10)
```

Output:
```
7
3
3
12
1024
```


### Common mistakes

**Counting from 0.** Kumu counts characters from **1**. `SUBSTRING(s, 1, 3)` is
the first three characters, not the second through fourth.

**Expecting `-1` for "not found".** `INDEX_OF` returns `0`, because position `0`
doesn't exist in a 1-indexed language. Test with `== 0`.

**A start position past the end.** `SUBSTRING` is forgiving about *length* — it
clamps — but not about where you start:

<!-- check
PRINT SUBSTRING("abc", 9, 2)
-->
```
Runtime Error: SUBSTRING: start 9 is out of range (line 1)
```

**Calling `NUMBER` without checking.** It's an error, not a `0`:

<!-- check
VAR typed = "abc"
PRINT NUMBER(typed) + 1
-->
```
Runtime Error: NUMBER: 'abc' is not a number -- check it with IS_NUMBER first (line 2)
```

**Forgetting `TRIM`.** `"yes "` and `"yes"` are different strings, and a user who
typed a trailing space has given you the first one.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does each line print?

```kumu
VAR s = "Kumu"
PRINT LEN(s)
PRINT SUBSTRING(s, 1, 2)
PRINT INDEX_OF(s, "mu")
PRINT INDEX_OF(s, "z")
```

**2. (Predict)** And these?

```kumu
PRINT IS_NUMBER("42")
PRINT IS_NUMBER("4.2")
PRINT IS_NUMBER("42abc")
PRINT IS_NUMBER("")
```

**3. (Fix)** This stops with an error. Make it print `not a number` instead,
without using `TRY`.

```kumu
VAR typed = "abc"
PRINT NUMBER(typed) + 1
```

**4. (Write)** Given `VAR full = "Ada Lovelace"`, print the initials `A.L.`.
(Find the space with `INDEX_OF`, then take one character from each side of it.)

**5. (Write)** Given `VAR w = "kUMU"`, print it in title case — `Kumu`. Upper
the first character, lower the rest.

### Recap

Text functions in Kumu are all 1-indexed, and "not found" is `0`. `LEN`,
`SUBSTRING` and `INDEX_OF` inspect; `UPPER`, `LOWER`, `TRIM` and `REPLACE`
reshape; `CONTAINS`, `STARTS_WITH` and `ENDS_WITH` ask yes-or-no questions.
`IS_NUMBER` and `NUMBER` are the pair that make user input safe to compute with
— always check before you convert.

---

## Chapter 9: Handling Errors with TRY/CATCH

**By the end of this chapter you'll be able to:**

- stop a runtime error from ending your program
- use the message `CATCH` gives you
- recover from input running out, instead of crashing
- say when catching an error is the wrong thing to do

Dividing by zero, reading an undefined variable, or `INPUT` running out of lines to read are all **runtime errors** — by default, they stop the program immediately and print a message. `TRY`/`CATCH` lets you intercept one instead:

```kumu
FUNCTION safeDivide(a, b) {
    TRY {
        RETURN a / b
    } CATCH err {
        PRINT "Error: " + err
        RETURN 0
    }
}
PRINT safeDivide(10, 2)
PRINT safeDivide(10, 0)
```

Output:
```
5
Error: Division by zero (line 3)
0
```

`err` is bound to the error's message as a String, including the line where it happened — scoped the same way a `VAR` would be (Chapter 7): local to `safeDivide` here, so it disappears once the call returns rather than lingering as a global. Notice that `RETURN 0` inside the `CATCH` still works exactly like a normal `RETURN` — it exits `safeDivide` itself, not just the `TRY` block. This is the tool that finally makes Chapter 5's `INPUT`-can-crash problem solvable: wrap the `INPUT` in a `TRY`, and a `CATCH` can respond to "no more input" without the whole program going down. The number-guessing game in Chapter 17 does exactly that.


### Common mistakes

**Catching errors you should have prevented.** `TRY` around a division is fine;
`TRY` around a whole program because "something in there breaks" hides the bug
instead of fixing it. Catch what you can actually respond to.

**Expecting the rest of the `TRY` block to run.** It doesn't. The moment an
error happens, control jumps to `CATCH` — everything after it in the block is
skipped.

**Expecting `TRY` to catch a typo.** It can't. Syntax errors are found before
the program runs at all, so there's nothing running yet to catch them.

**Forgetting the error message is just text.** `err` holds a String. You can
print it, join it, or search it with `INDEX_OF` — which is how `address.kumu`
tells "no more input" apart from every other error.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does this print, in order?

```kumu
TRY {
    PRINT "before"
    PRINT 1 / 0
    PRINT "after"
} CATCH e {
    PRINT "caught"
}
PRINT "done"
```

**2. (Fix)** This ends with `INPUT: no more input available` when there's
nothing to read. Wrap it so it prints `no input` and continues instead.

```kumu
INPUT name
PRINT "Hello, " + name
```

**3. (Write)** Write `safeNumber(text)` that returns the number when `text` is
one, and `0` when it isn't — using `TRY`/`CATCH` rather than `IS_NUMBER`. Test
it with `"42"` and `"abc"`.

**4. (Think)** `safeNumber` above returns `0` for bad input. Why might that be a
bad idea, and what could it return instead? (No code needed.)

### Recap

`TRY` runs a block; if a runtime error happens, control jumps to `CATCH` with
the message bound as a String, and the program carries on. The rest of the `TRY`
block is skipped. `RETURN` inside either block still returns from the enclosing
function. Catch errors you can respond to — not ones you should have prevented.

---

## Chapter 10: Debugging with TRACE

**By the end of this chapter you'll be able to:**

- watch a program run, one statement at a time
- turn tracing on for just the part you're suspicious of
- find a bug that produces no error at all

`TRACE 1` makes the interpreter print every statement as it runs, with its source line — useful for watching a program's actual control flow instead of guessing at it. `TRACE 0` turns it back off.

Statements that change a variable also show the value that landed, which is usually the thing you actually wanted to know:

```kumu
TRACE 1
VAR x = 3
PRINT x * 2
TRACE 0
```

Output:
```
[TRACE line 2] VAR x = 3
[TRACE line 3] PRINT
6
[TRACE line 4] TRACE
```

`VAR` and bare assignment trace differently — `VAR x = 3` shows as `VAR x`, while a later bare `x = x + 1` would show as `ASSIGN x`. This distinction matters more than it looks like here; Chapter 7 covers why.

Inside a function call, trace lines are indented one step per level of nesting. That's what makes a recursive function readable — without it, six passes through `factorial` look like one repeated list.

`TRACE 0` gets traced too — tracing is still on when that statement *begins*, since it's the statement that turns it off.

### Using TRACE to Find a Real Bug

That's the syntax, but it doesn't yet show *why* `TRACE` earns its own chapter. Here's a program with a genuine bug in it — one that Kumu itself has no way to warn you about:

```kumu
VAR runningTotal = 0

FUNCTION addToTotal(amount) {
    runingTotal = runningTotal + amount
}

addToTotal(10)
addToTotal(20)
addToTotal(5)
PRINT "Running total: " + runningTotal
```

The intent is obvious: add 10, then 20, then 5, and print 35. What it actually prints is:

```
Running total: 0
```

There's no error, no crash, nothing pointing at a line number — just a wrong answer. That's because `runingTotal` inside the function is missing an "n", and — importantly — it's written as a **bare** assignment, not `VAR runingTotal = ...`. Recall from Chapter 7: a bare assignment to a name that doesn't already exist anywhere doesn't fail, it just creates a brand new *global* variable on the spot. Since the misspelled `runingTotal` had never been assigned before, that's exactly what happens: a new global quietly springs into existence, and `addToTotal` faithfully keeps writing to it three times, while the real `runningTotal` sits untouched at `0` forever.

This is precisely the kind of bug where staring harder at the code doesn't help, because the code has no error in its *syntax*, only in one character of one name. Turning `TRACE` on shows what's actually happening, statement by statement:

```kumu
VAR runningTotal = 0
TRACE 1

FUNCTION addToTotal(amount) {
    runingTotal = runningTotal + amount
}

addToTotal(10)
addToTotal(20)
addToTotal(5)
TRACE 0
PRINT "Running total: " + runningTotal
```

Output:
```
[TRACE line 4] FUNCTION addToTotal
[TRACE line 8] EXPRESSION
  [TRACE line 5] ASSIGN runingTotal = 10
[TRACE line 9] EXPRESSION
  [TRACE line 5] ASSIGN runingTotal = 20
[TRACE line 10] EXPRESSION
  [TRACE line 5] ASSIGN runingTotal = 5
[TRACE line 11] TRACE
Running total: 0
```

Look at what's assigned on line 5, three times in a row: `ASSIGN runingTotal`. Not `runningTotal`. And look at the values — `10`, `20`, `5`, never accumulating, because each call is writing to a variable that nothing ever reads. Once you're watching every assignment go by, a name that doesn't match what you meant to write stands out immediately — you're no longer trusting that the code says what you think it says, you're reading what the interpreter actually did. Fix the spelling:

```kumu
VAR runningTotal = 0

FUNCTION addToTotal(amount) {
    runningTotal = runningTotal + amount
}

addToTotal(10)
addToTotal(20)
addToTotal(5)
PRINT "Running total: " + runningTotal
```

Output:
```
Running total: 35
```

The general technique behind this example generalizes well beyond misspelled names: whenever a program runs without crashing but produces a wrong answer, `TRACE` turns "which of these statements actually did what I expected?" from a guess into something you can read directly off the screen.


### Common mistakes

**Leaving `TRACE 1` on.** The output is enormous for anything with a loop. Turn
it on immediately before the suspicious part and off immediately after.

**Expecting `TRACE` to show every value.** Assignments and `INPUT` show what
landed; other statements show only that they ran. `PRINT` is still how you
inspect an arbitrary expression.

**Reaching for `TRACE` first.** Read the code and the error message first;
`TRACE` earns its keep on bugs that produce no error at all — like the
misspelling in this chapter.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does this print? Include the trace lines.

```kumu
TRACE 1
VAR x = 2
x = x + 1
PRINT x
TRACE 0
```

**2. (Write)** This prints `0` when it should print `6`. Add `TRACE 1` and
`TRACE 0` around the loop, run it, and use what you see to find the bug — then
fix it.

```kumu
VAR total = 0
FOR i = 1 TO 3 {
    totl = total + i
}
PRINT total
```

### Recap

`TRACE 1` turns on a running commentary of every statement as it executes, with
its line number; `TRACE 0` turns it off. It shows what ran and in what order,
not what the values were. It's the tool for bugs that produce no error — which
in Kumu most often means a misspelled name that quietly became a new variable.

---

## Chapter 11: Random Numbers

**By the end of this chapter you'll be able to:**

- produce a random whole number in a range you choose
- build a dice roll, a coin flip, or a random choice

`RANDOM(min, max)` returns a random whole number in the inclusive range `[min, max]`:

```kumu
PRINT RANDOM(1, 6)
```

Output (this one is different every time you run it):
```
4
```

`RANDOM` requires `min <= max` — reversing them is a runtime error, which is exactly the kind of thing Chapter 9's `TRY`/`CATCH` is for.


### Common mistakes

**Backwards bounds.** `min` must not exceed `max`:

<!-- check
PRINT RANDOM(10, 1)
-->
```
Runtime Error: RANDOM: min must be <= max (line 1)
```

**Expecting a fraction.** `RANDOM` gives whole numbers only. For a random
decimal, divide: `RANDOM(1, 100) / 100`.

**Expecting `max` to be excluded.** The range includes both ends, so
`RANDOM(1, 6)` really can return `6`. `RANDOM(5, 5)` always returns `5`.

**Enormous bounds.** Both must be whole numbers no larger than 1e15.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does `PRINT RANDOM(5, 5)` print, and why is it the only
`RANDOM` call in this book with a predictable answer?

**2. (Fix)** This stops with an error. What's wrong, and what did the author
probably mean?

```kumu
PRINT RANDOM(10, 1)
```

**3. (Write)** Roll two dice and print their total.

**4. (Write)** Flip a coin: print `heads` or `tails` with an even chance.

### Recap

`RANDOM(min, max)` gives a random whole number, with both ends included and
`min <= max` required. It's what makes games and simulations possible, and
Chapter 17 builds a whole one on it.

---

## Chapter 12: Console Control: CLEAR_SCREEN and SLEEP

**By the end of this chapter you'll be able to:**

- clear the screen so a program can redraw itself
- pause deliberately, so output can be read before it's replaced

Two small tools round out what a program can do with the console itself, beyond just `PRINT` and `INPUT`: clearing it, and pausing before doing anything else. Neither one takes a variable to write into or produces a value worth keeping, so both are normally called as bare statements, the same way `logMessage("started")` was in Chapter 7.

`CLEAR_SCREEN()` wipes the terminal and puts the cursor back at the top-left — useful any time you want a program to feel like it's showing a fresh screen rather than an endless scroll of old output:

```kumu
PRINT "This will disappear."
CLEAR_SCREEN()
PRINT "Fresh screen."
```

`SLEEP(seconds)` pauses the program for a given number of seconds, which can be fractional. A classic use is a countdown:

```kumu
VAR count = 3
WHILE count > 0 {
    PRINT count
    SLEEP(1)
    count = count - 1
}
PRINT "Go!"
```

Output (with a one-second pause between each line):
```
3
2
1
Go!
```

Combine `CLEAR_SCREEN`, `SLEEP`, and Chapter 11's `RANDOM`, and even a one-line result can get a little drama:

```kumu
CLEAR_SCREEN()
PRINT "Rolling the dice..."
SLEEP(1)
PRINT RANDOM(1, 6)
```


### Common mistakes

**A negative `SLEEP`.** An error — you can't pause for less than no time.

**Clearing before the user has read anything.** `CLEAR_SCREEN` erases what was
there. Pair it with a `SLEEP`, or a "press Enter" `INPUT`, or the user sees a
flash of something they can't read. `address.kumu` wraps exactly this pattern in
a `pauseForReading()` helper.

**Expecting `SLEEP` to be exact.** It's "at least this long", not a guarantee.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Write)** Count down from 3 to 1 with a short pause between each number,
then print `Go!`.

**2. (Write)** Print a two-line menu, pause for a second, clear the screen, then
print `Cleared.`. Run it to watch it happen — the output alone won't show you
the timing.

### Recap

`CLEAR_SCREEN()` wipes the screen and `SLEEP(seconds)` pauses. Neither produces
a value; both are called as statements for their effect. Together they're what
turns a scrolling wall of text into something that behaves like an application.

---

## Chapter 13: Lists

**By the end of this chapter you'll be able to:**

- hold several values under one name and get them back out
- walk a list with `FOR EACH`
- explain why a list method never changes the list you called it on
- turn text into a list and back again

Every variable so far has held exactly one thing: one number, one piece of text. That works until you want *several* — five test scores, a shopping list, the words in a sentence. A **List** holds a sequence of values in order, written in square brackets:

```kumu
VAR scores = [95, 88, 72]
PRINT scores
PRINT scores.LEN()
```

Output:
```
[95, 88, 72]
3
```

### Getting items out

Square brackets again, with a position. Kumu counts from **1**, the same as `SUBSTRING` and `INDEX_OF` back in Chapter 8:

```kumu
VAR scores = [95, 88, 72]
PRINT scores[1]
PRINT scores[3]
```

Output:
```
95
72
```

If you've seen another language before, note that many of them count from 0. Kumu doesn't, anywhere. Asking for `scores[0]` is an error, and so is asking for `scores[4]` when there are only three items — in both cases the message tells you how long the list actually is.

### Walking a list

You could count through the positions with a `FOR` loop, but there's a better way — `FOR EACH` hands you each item directly, and you never have to think about the length at all:

```kumu
VAR names = ["Ada", "Alan", "Grace"]

FOR EACH name IN names {
    PRINT "Hello, " + name
}
```

Output:
```
Hello, Ada
Hello, Alan
Hello, Grace
```

`BREAK` and `CONTINUE` from Chapter 6 work here exactly as they do in any other loop.

### Lists don't change

Here is the one rule that surprises people, and it's worth slowing down for. A list method never modifies the list you called it on. It builds a **new** list and hands that back:

```kumu
VAR scores = [95, 88]
VAR more = scores.APPEND(72)
PRINT scores
PRINT more
```

Output:
```
[95, 88]
[95, 88, 72]
```

`scores` is untouched. If you want the change to stick, assign it back:

```kumu
VAR scores = [95, 88]
scores = scores.APPEND(72)
PRINT scores
```

Output:
```
[95, 88, 72]
```

This is exactly the rule Matrices follow in the next chapter, so learning it once covers both. It also means there is no `scores[1] = 99` — trying it is a syntax error that points you at `.SET`:

```kumu
VAR scores = [95, 88]
scores = scores.SET(1, 99)
PRINT scores
```

Output:
```
[99, 88]
```

### What you can do with a list

Building and reshaping: `.APPEND(value)`, `.INSERT(position, value)`, `.REMOVE(position)`, `.SET(position, value)`, `.REVERSE()`, `.SLICE(start, length)`.

Searching: `.CONTAINS(value)` answers yes-or-no, `.INDEX_OF(value)` gives the position — or `0` if it isn't there, that same convention again.

Arithmetic: `.SUM()`, `.AVERAGE()`, `.MIN()`, `.MAX()`.

Ordering: `.SORT()`, which works out on its own whether you meant numbers or text.

```kumu
VAR scores = [72, 95, 88]
PRINT scores.SORT()
PRINT scores.SUM()
PRINT scores.AVERAGE()
PRINT scores.MAX()
PRINT scores.CONTAINS(88)
PRINT scores.INDEX_OF(95)
PRINT ["pear", "apple", "fig"].SORT()
```

Output:
```
[72, 88, 95]
255
85
95
1
2
["apple", "fig", "pear"]
```

Notice that strings print with quotes when they're inside a list. That's deliberate: inside a list, quotes are the only thing separating the number `1` from the text `"1"`.

`.SORT()` goes smallest-first. Pass it a `1` to go the other way:

```kumu
VAR scores = [72, 95, 88]
PRINT scores.SORT()
PRINT scores.SORT(1)
```

Output:
```
[72, 88, 95]
[95, 88, 72]
```

`.SORT().REVERSE()` gets you the same answer the long way round, and reads
just as well — use whichever says more clearly what you meant.

### Lists and text

`SPLIT` breaks a string apart into a list, and `.JOIN` glues a list back into a string. They're opposites, and together they're how most text-handling work actually gets done:

```kumu
VAR csv = "Ada,Alan,Grace"
VAR names = SPLIT(csv, ",")
PRINT names
PRINT names.LEN()
PRINT names.SORT().JOIN(" and ")
```

Output:
```
["Ada", "Alan", "Grace"]
3
Ada and Alan and Grace
```

Numbers that arrive as text still add up, because the arithmetic methods accept anything that reads as a number:

```kumu
VAR raw = SPLIT("10,20,30", ",")
PRINT raw.SUM()
PRINT raw.AVERAGE()
```

Output:
```
60
20
```

### Lists inside lists

A list can hold anything, including other lists — which gives you a grid:

```kumu
VAR grid = [[1, 2], [3, 4]]
PRINT grid[2][1]
```

Output:
```
3
```

Read `grid[2][1]` left to right: take item 2 of `grid` (which is `[3, 4]`), then item 1 of that.

### A worked example

Averaging test scores typed in by a person, with bad input rejected rather than crashing:

```kumu
VAR raw = "88, 92, oops, 79"
VAR scores = []

FOR EACH piece IN SPLIT(raw, ",") {
    VAR cleaned = TRIM(piece)
    IF IS_NUMBER(cleaned) {
        scores = scores.APPEND(NUMBER(cleaned))
    } ELSE {
        PRINT "Skipping '" + cleaned + "' - not a number"
    }
}

PRINT "Kept " + scores.LEN() + " scores: " + scores.JOIN(", ")
PRINT "Average: " + scores.AVERAGE()
PRINT "Best: " + scores.MAX()
```

Output:
```
Skipping 'oops' - not a number
Kept 3 scores: 88, 92, 79
Average: 86.3333
Best: 92
```

Every piece of this chapter is in there: `SPLIT` to break the text up, `FOR EACH` to walk it, `TRIM` and `IS_NUMBER` from Chapter 8 to clean and check, `.APPEND` assigned back because lists don't change in place, and `.JOIN`, `.AVERAGE` and `.MAX` to report.


### Common mistakes

**Forgetting to assign the result back.** This is the big one, and it's silent:

```kumu
VAR xs = [1, 2, 3]
xs.APPEND(4)
PRINT xs
```

Output:
```
[1, 2, 3]
```

The new list was built and then thrown away. You wanted `xs = xs.APPEND(4)`.

**Counting from 0.** Lists are 1-indexed, like everything else in Kumu.
`xs[0]` is an error naming the length.

**Expecting `[1,2] == [1,2]` to be false.** It's true — lists compare by
contents, all the way down through nesting.

**Sorting text that looks numeric.** `.SORT()` looks at the contents: if every
item reads as a number it sorts numerically, even when they're strings.

**`.AVERAGE()` on an empty list.** An error, not `0` — there's no meaningful
answer.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What do these two lines print, and why are they different?

```kumu
VAR xs = [1, 2, 3]
xs.APPEND(4)
PRINT xs
PRINT xs.APPEND(4)
```

**2. (Predict)** All three of these sort. What does each produce?

```kumu
PRINT [10, 9, 100].SORT()
PRINT ["10", "9", "100"].SORT()
PRINT ["banana", "apple", "cherry"].SORT()
```

**3. (Write)** Given `VAR raw = "4,8,15,16"`, print the average of those four
numbers. (One `SPLIT`, one method call.)

**4. (Write)** Reverse the word `"stressed"` a letter at a time and print the
result. (`SPLIT` with an empty separator gives you the letters.)

**5. (Write)** Given a list of names, print only the ones longer than four
characters, one per line. Use `FOR EACH` and `LEN`.

### Recap

A List holds any values in order, indexed from 1, and can hold other lists. No
method ever changes the list you called it on — each returns a new one, so
`xs = xs.APPEND(...)` is how a change sticks. `FOR EACH` walks a list without a
counter, `SPLIT` turns text into one and `.JOIN` turns one back into text.

---

## Chapter 14: Files and Matrices

**By the end of this chapter you'll be able to:**

- save text to a file and read it back
- hold a table of data in a Matrix and ask questions about it
- move data between a Matrix and a CSV file

This chapter covers two ways Kumu programs keep data around after they exit: plain text files, and Matrices — a table type with its own dot-methods, built for tabular data like CSV. The two connect directly: a Matrix is what `READ_CSV` gives you back, and what `.SAVE` writes out.

### Plain text
The simplest case is a whole file read or written as one string:

```kumu
WRITE_FILE("diary.txt", "Today I learned Kumu.")
IF FILE_EXISTS("diary.txt") {
    VAR entry = READ_FILE("diary.txt")
    PRINT entry
}
```

Output:
```
Today I learned Kumu.
```

### Matrices
A Matrix is a genuine object in Kumu — once you have one, you call methods on it with `.` instead of passing it to a function. It's written as nested parentheses, and row 1 is always its header:

```kumu
VAR grades = (
    ("Name", "Grade"),
    ("Ann", "A"),
    ("Ben", "B")
)
PRINT grades.GET(1, "Name")
```

Output:
```
Ann
```

Every method treats row 1 as the permanent header, so row numbers everywhere else always mean data rows — `.GET(1, ...)` is the first real row, never "Name"/"Grade" itself. Columns work by name (`"Name"`) or by position (`1`) interchangeably: `grades.GET(1, "Name")` and `grades.GET(1, 1)` are the same call.

Here's the one design choice in Kumu worth calling out on its own: every method that changes data returns a **new** Matrix rather than changing the one you called it on.

```kumu
VAR grades = (
    ("Name", "Grade"),
    ("Ann", "A"),
    ("Ben", "B")
)
VAR updated = grades.SET(2, "Grade", "A+")
PRINT updated.GET(2, "Grade")
PRINT grades.GET(2, "Grade")
```

Output:
```
A+
B
```

`grades` itself never changed — `updated` is a separate value. Everywhere else in Kumu, assignment mutates (`x = x + 1` really does change `x`); Matrix's mutating methods are the one deliberately functional-style corner of an otherwise fully imperative language. If you want a change to stick, you reassign: `grades = grades.SET(2, "Grade", "A+")`.

Matrices round-trip straight to CSV files — `.SAVE` writes one, `READ_CSV` reads one back (a plain function, not a method, since there's no existing Matrix to call it on yet):

```kumu
VAR grades = (
    ("Name", "Grade"),
    ("Ann", "A"),
    ("Ben", "B")
)
grades.SAVE("grades.csv")
VAR loaded = READ_CSV("grades.csv")
PRINT loaded
```

Output:
```
Name | Grade
---- | -----
Ann  | A    
Ben  | B    
```

The round trip is faithful even when the data itself contains the characters CSV uses as punctuation. A cell holding a comma — `"Smith, Jr."` — is written wrapped in quotes so that reading it back gives you one cell again, not two. The same goes for a cell containing a quote or a line break. You don't have to do anything to get this; it's worth knowing only so you can trust that what you save is what you load, and so the file you open in a spreadsheet looks the way you expect.

Matrices also come with built-in spreadsheet-style aggregates. `.SUM`/`.AVERAGE`/`.MIN`/`.MAX` all read down one column, and are strict — every cell must parse as a number, or they throw:

```kumu
VAR scores = (
    ("Name", "Score"),
    ("Ann", "72"),
    ("Ben", "95"),
    ("Cy", "88")
)
PRINT scores.SUM("Score")
PRINT scores.AVERAGE("Score")
```

Output:
```
255
85
```

`.COUNT` is the deliberate exception to that strictness: it counts only the cells that *do* parse as numbers, silently skipping blanks and text like `"N/A"` — so it's the one aggregate that's safe to run over messy, real-world data:

```kumu
VAR prices = (
    ("Item", "Price"),
    ("Widget", "9.99"),
    ("Gadget", "N/A"),
    ("Gizmo", "15.50")
)
PRINT prices.COUNT("Price")
TRY {
    PRINT prices.SUM("Price")
} CATCH err {
    PRINT "Caught: " + err
}
```

Output:
```
2
Caught: SUM: row 2, column 2 ('N/A') is not a number (line 9)
```

Two more round out the set: `.FIND(col, value)` returns the 1-based row of the first exact match, or `0` if there isn't one; `.SORT(col)` returns a new Matrix sorted ascending by that column, auto-detecting whether to compare numerically or as text (numeric only if *every* cell in that column parses as a number — a column with even one `"N/A"` in it sorts as text instead). As with a List, a `1` on the end sorts the other way: `.SORT(col, 1)`.

```kumu
VAR scores = (
    ("Name", "Score"),
    ("Ann", "72"),
    ("Ben", "95"),
    ("Cy", "88")
)
PRINT scores.FIND("Name", "Ben")
PRINT scores.SORT("Score")
```

Output:
```
2
Name | Score
---- | -----
Ann  | 72   
Cy   | 88   
Ben  | 95   
```


### Common mistakes

**Forgetting to assign the result back.** Matrix methods behave exactly like
list methods — `d.APPEND_ROW(...)` builds a new Matrix and discards it unless
you write `d = d.APPEND_ROW(...)`.

**Counting the header as a row.** `.ROWS()` never counts it, and `.GET(1, ...)`
is the first *data* row. The header exists to be looked up by name, not read as
data.

**Expecting `.SUM` to skip bad cells.** It won't — it stops and names the
offending cell. `.COUNT` is the lenient one that skips non-numbers.

**Expecting cells to remember they were numbers.** Every Matrix cell is text.
That's why `.SUM` has to parse, and why a List is the better home for a sequence
of numbers.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What do these three lines print?

```kumu
VAR d = (("A","B"),("1","2"),("3","4"))
PRINT d.ROWS()
PRINT d.COLS()
PRINT LEN(d)
```

**2. (Fix)** This should print `2`, but prints `1`. Why?

```kumu
VAR d = (("A"),("1"))
d.APPEND_ROW("2")
PRINT d.ROWS()
```

**3. (Write)** Build a Matrix of three items with a `Cost` column, and print the
total cost.

**4. (Write)** Take that Matrix, add a fourth row, save it to a CSV file, read
it back with `READ_CSV`, and print how many rows came back.

### Recap

`WRITE_FILE` and `READ_FILE` handle plain text; a Matrix holds a table with a
permanent header row, and `.SAVE`/`READ_CSV` move it to and from a CSV file.
Every cell is text, row 1 is always the header, and — as with lists — methods
return a new Matrix rather than changing the one you have.

---

## Chapter 15: Working with Dates

**By the end of this chapter you'll be able to:**

- write a date so that sorting and comparing it do the right thing
- find today's date, and count days forwards and backwards
- check that something really is a date before trusting it

### The problem

Suppose you're keeping a to-do list, and every item needs a due date. Before you
can write a single line of code you have to decide how a date gets written down.
`23/9/2026`? `Sept 23`? `9-23-26`?

Pick one and try sorting a few:

```kumu
PRINT ["23/9/2026", "5/1/2026", "14/12/2026"].SORT()
```

Output:
```
["14/12/2026", "23/9/2026", "5/1/2026"]
```

December first, January last. Sorting text compares character by character, and
the first character of each is the *day*, so the day is what got sorted. The
result is in no useful order at all.

Kumu's answer isn't a new kind of value. It's a rule about how you write one
down. **A date is an ordinary String written `YYYY-MM-DD`** — a four-digit year,
then the month, then the day, each padded with a zero if it needs one:
`2026-09-23`.

Put the biggest unit first and the problem disappears:

```kumu
PRINT ["2026-09-23", "2026-01-05", "2026-12-14"].SORT()
```

Output:
```
["2026-01-05", "2026-09-23", "2026-12-14"]
```

That's the whole trick, and it's worth understanding rather than memorising:
sorting dates as *text* now puts them in *time* order, because the characters
happen to line up with the calendar.

### Comparing dates

The same rule means the comparison operators from Chapter 3 already work:

```kumu
VAR dueDate = "2026-09-23"
VAR deadline = "2026-10-01"

IF dueDate < deadline {
    PRINT "There's still time."
}
```

Output:
```
There's still time.
```

Nothing clever is going on. `<` is comparing two ordinary strings, exactly the
way it would compare `"apple"` and `"banana"`. It gives the right answer purely
because of how the dates are written.

### Today

`TODAY()` gives today's date, already in the right shape. `NOW()` adds the time.
Neither takes any arguments.

```kumu
PRINT TODAY()
PRINT NOW()
```

Output (yours will show the day you run it):
```
2026-09-24
2026-09-24 20:42:11
```

Like `RANDOM` in Chapter 11, these make a program's output different every time,
so a program that prints them can't be checked against a fixed expected answer.

### Counting days

`DATE_DIFF(from, to)` counts whole days from the first date to the second:

```kumu
PRINT DATE_DIFF("2026-09-23", "2026-09-30")
PRINT DATE_DIFF("2026-09-30", "2026-09-23")
```

Output:
```
7
-7
```

The order of the two dates matters, and the sign is the useful part: a negative
answer means the second date is the earlier one. That turns "is this overdue?"
into a single comparison:

```kumu
VAR due = "2026-01-01"
IF DATE_DIFF(TODAY(), due) < 0 {
    PRINT "Overdue."
}
```

`DATE_ADD(date, days)` moves a date forwards, or backwards if you give it a
negative number:

```kumu
PRINT DATE_ADD("2026-09-23", 7)
PRINT DATE_ADD("2026-09-23", -1)
PRINT DATE_ADD("2026-12-31", 1)
```

Output:
```
2026-09-30
2026-09-22
2027-01-01
```

The last one rolled into a new year without being asked to. Both functions know
how long each month is, and which years are leap years:

```kumu
PRINT DATE_ADD("2024-02-28", 1)
PRINT DATE_ADD("2026-02-28", 1)
```

Output:
```
2024-02-29
2026-03-01
```

Same sum, different answers, because 2024 is a leap year and 2026 isn't. This is
the part you'd least enjoy working out by hand, and you never have to.

### Taking a date apart

```kumu
PRINT WEEKDAY("2026-09-23")
PRINT YEAR("2026-09-23")
PRINT MONTH("2026-09-23")
PRINT DAY("2026-09-23")
```

Output:
```
Wednesday
2026
9
23
```

`WEEKDAY` gives a String; `YEAR`, `MONTH` and `DAY` give Numbers, so you can do
arithmetic with them:

```kumu
VAR birthday = "1990-11-23"
PRINT "Born in " + YEAR(birthday) + ", on a " + WEEKDAY(birthday) + "."
```

Output:
```
Born in 1990, on a Friday.
```

### Checking before you trust

Every function in this chapter is strict. Hand one something that isn't a real
date and it stops rather than guessing:

<!-- check
PRINT DATE_ADD("2026-02-30", 1)
-->
```
Runtime Error: DATE_ADD date: '2026-02-30' is not a valid date -- dates are written as YYYY-MM-DD (e.g. 2026-09-23) (line 1)
```

That's deliberate. There is no February the 30th, and there's no sensible guess
to make — is it March the 2nd, or February the 28th, or a typo for the 3rd? A
program that quietly picked one would give you a wrong answer weeks later with
nothing to trace it back to.

`IS_DATE` is how you check first. It's the same pattern as `IS_NUMBER` in
Chapter 5: ask, then act.

```kumu
PRINT IS_DATE("2026-02-28")
PRINT IS_DATE("2024-02-29")
PRINT IS_DATE("2026-02-29")
PRINT IS_DATE("2026-1-1")
PRINT IS_DATE("tomorrow")
```

Output:
```
1
1
0
0
0
```

The middle two are worth a second look. `2024-02-29` is real because 2024 is a
leap year; `2026-02-29` isn't, because 2026 is not. `IS_DATE` doesn't just check
the shape of the text, it checks that the day actually exists.

Put together with `INPUT`, that's the shape of every date a user types:

```kumu
PRINT "When is it due? (YYYY-MM-DD)"
INPUT typed
IF IS_DATE(typed) {
    PRINT "That's " + DATE_DIFF(TODAY(), typed) + " day(s) away."
} ELSE {
    PRINT "That doesn't look like a date."
}
```

### Sorting by date

Because a date is just text in a well-chosen format, everything you already know
from Chapters 13 and 14 works on dates with nothing added:

```kumu
VAR tasks = (
    ("Task", "Due"),
    ("write", "2026-10-01"),
    ("call", "2026-09-25"),
    ("ship", "2026-12-31")
)
PRINT "Soonest first:"
PRINT tasks.SORT("Due")
PRINT "Latest first:"
PRINT tasks.SORT("Due", 1)
```

Output:
```
Soonest first:
Task  | Due       
----- | ----------
call  | 2026-09-25
write | 2026-10-01
ship  | 2026-12-31
Latest first:
Task  | Due       
----- | ----------
ship  | 2026-12-31
write | 2026-10-01
call  | 2026-09-25
```

The `1` on the end is the same descending flag you met in Chapter 14. Nothing in
either call knows it's looking at dates — it's sorting text, and the format is
what makes that the right thing to do.

### Common mistakes

**Writing the date some other way.** `09/23/2026` is not a Kumu date, and no
function here will accept it. If your data arrives in another format you have to
rearrange it yourself — `SPLIT` from Chapter 8 is usually how.

**Dropping the padding zero.** `2026-9-1` looks like a date to a person but is
rejected, because the whole scheme depends on every date being exactly ten
characters. `2026-09-01` is the same day, written correctly.

**Getting `DATE_DIFF`'s arguments the wrong way round.** `DATE_DIFF(from, to)`
counts *from* the first *to* the second. If your overdue check is reporting
everything backwards, this is almost always why.

**Expecting dates to be their own type.** They aren't, and that's the design.
`LEN("2026-09-23")` is `10`, `SUBSTRING` works on it, and a date read from a CSV
arrives as ordinary text needing no conversion at all.

**Comparing a date with a number.** `"2026-09-23" < 2026` is a runtime error:
`<` needs both sides to be the same type (Chapter 3). Use `YEAR()` to get a
number out first.

**Forgetting that a blank isn't a date.** An empty cell is `""`, which
`IS_DATE` rejects and which sorts *before* every real date. In a list of items
where the due date is optional, the undated ones come first ascending and last
descending.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does this print, and what does the sign tell you?

```kumu
PRINT DATE_DIFF("2026-03-01", "2026-02-28")
```

**2. (Predict)** What does `PRINT IS_DATE("2026-4-15")` print? There is nothing
wrong with that day — so why?

**3. (Fix)** This is meant to report how many days are left, but it stops with
an error. What's wrong?

```kumu
VAR deadline = "2026/12/25"
PRINT DATE_DIFF(TODAY(), deadline)
```

**4. (Write)** Ask the user for a date, and print what day of the week it falls
on — or a polite message if what they typed isn't a date.

**5. (Write)** Given `VAR dates = ["2026-12-14", "2026-01-05", "2026-09-23"]`,
print the earliest one without writing a loop.

**6. (Write)** Print the date a fortnight from today, and the day of the week it
lands on.

### Recap

A Kumu date is a String written `YYYY-MM-DD`, and that format is doing real
work: it makes sorting text the same as sorting time, and makes `<` and `>`
compare dates correctly. `TODAY()` and `NOW()` tell you when it is. `DATE_DIFF`
counts days between two dates and `DATE_ADD` moves one along, both handling month
lengths and leap years for you. `WEEKDAY`, `YEAR`, `MONTH` and `DAY` take a date
apart. And because every one of them is strict about what it accepts, `IS_DATE`
is how you check anything that came from a user or a file before you rely on it.

---

## Chapter 16: Comments

**By the end of this chapter you'll be able to:**

- leave notes in a program that Kumu ignores
- tell the difference between a comment worth writing and one that isn't

Anything from a `#` to the end of the line is ignored:

```kumu
# This program prints a greeting
PRINT "Hi!"   # inline comment
```

Output:
```
Hi!
```


### Why bother

A comment can't make a program work. What it can do is tell the next reader —
usually you, months later — something the code itself can't say: *why* the code
is the way it is.

```kumu
# Case-insensitive on purpose: people type "bob smith" for "Bob Smith".
VAR search = LOWER(typed)
```

That comment is worth writing. This one isn't:

```kumu
VAR count = 0   # set count to 0
```

It repeats what the line already says, and it's one more thing to go stale when
the line changes. Comment the reasons, not the mechanics.

### Common mistakes

**Expecting `#` to work inside a string.** It doesn't — inside quotes it's an
ordinary character. `PRINT "# not a comment"` prints the `#`.

**Comments that disagree with the code.** A wrong comment is worse than none,
because it's believed. When you change a line, check the comment above it.

### Exercises

Answers are in [Appendix A](#appendix-a-answers-to-exercises).

**1. (Predict)** What does this print?

```kumu
PRINT "# hello"   # a real comment
```

**2. (Write)** Take any program you've written for an earlier chapter and add a
comment at the top saying what it does and one comment explaining a decision you
made. Then read it back and delete any comment that only repeats the code.

### Recap

`#` starts a comment that runs to the end of the line, and Kumu ignores it —
unless it's inside a string, where it's just a character. Write comments that
explain why, not what.

---

## Chapter 17: Putting It Together — A Guessing Game

**By the end of this chapter you'll be able to:**

- read a complete program and see every chapter of this book in it
- extend a working program rather than starting from a blank file

This chapter's example uses seven different things from this book at once: `RANDOM`, `INPUT`, `WHILE`, `IF`, string concatenation, comparison, and — the one that makes it robust — `TRY`/`CATCH`.

```kumu
PRINT "I'm thinking of a number between 1 and 10."
VAR secret = RANDOM(1, 10)
VAR guess = 0
VAR ranOut = 0
WHILE guess != secret {
    PRINT "Your guess?"
    TRY {
        INPUT guess
    } CATCH err {
        PRINT "No more guesses available - the number was " + secret
        ranOut = 1
        guess = secret
    }
}
IF ranOut == 0 {
    PRINT "You got it!"
}
```

A sample run where the secret number was 4 and the player found it on the fourth guess:
```
I'm thinking of a number between 1 and 10.
Your guess?
1
Your guess?
2
Your guess?
3
Your guess?
4
You got it!
```

And what happens if the player's input runs out before they guess correctly:
```
I'm thinking of a number between 1 and 10.
Your guess?
No more guesses available - the number was 7
```

Notice the `ranOut` flag: the `WHILE` loop's condition (`guess != secret`) can become false two different ways — a real correct guess, or the `CATCH` forcing `guess = secret` to escape an input that's run dry. Without `ranOut` to tell those two cases apart, the program would falsely print "You got it!" even when the player never actually won. It's a small example of a real lesson: when a loop can end for more than one reason, make sure your code afterward can tell *which* reason it was.


### Extending it

The best way to finish a book like this is to change something in a program that
already works. Each of these is a small edit to the guessing game, and each one
exercises a different chapter.

**1.** Count the guesses and report the total when the player wins. (Chapter 2)

**2.** Tell the player how many guesses they have left, counting down from a
limit you choose, and end the game when they run out. (Chapters 4 and 6)

**3.** Keep every guess in a list and print them all, sorted, at the end.
(Chapter 13)

**4.** Move the "too high / too low" decision into its own function that returns
a message, so the main loop just prints what it returns. (Chapter 7)

**5.** Save the number of guesses to a file each time, and print the best score
so far when the game starts. (Chapter 14 — and you'll need `FILE_EXISTS`.)

### Where this leaves you

You've now met every keyword and built-in function Kumu has. `address.kumu` in
this directory is the largest program written in it, and — having read this far
— you should be able to read it end to end. Doing that is the real final
exercise: it's one thing to write a program from a chapter's worth of ideas, and
another to walk into someone else's and work out what it's doing.

---

## Where to Go From Here

This book covers every keyword and built-in function Kumu has, but a few larger, complete programs are worth reading in full once you're comfortable with the basics:

- **`test.kumu`** exercises every language feature in one file — a good file to skim when you want a reminder of exact syntax.
- **`collections.kumu`** is a guided tour of the two collection types working together: a gradebook Matrix for storage, Lists for the arithmetic, `FOR EACH` as the bridge between them, and a CSV round trip at the end. Read it after Chapters 13 and 14.
- **`address.kumu`** is the primary, flagship example of what Kumu programs look like in practice: a real, persistent, menu-driven address book with a two-level menu (Manage Contacts / Tools, both using `ELSEIF` for a flat, readable dispatch), a clean `CLEAR_SCREEN`-refreshed screen with `SLEEP`-paced pauses so results stay readable, an exported text report alongside its CSV persistence, and a `.COLS()` sanity check on the data it loads. It has a "sort contacts alphabetically" tool built on `.SORT`, recovers gracefully from running out of input at any menu depth, and is the largest example in this book of everything working together at once. One piece worth studying: its contact search is case-insensitive, and hand-rolls the lookup to get that. `.FIND` can now do it directly with its optional third argument — `book.FIND("Name", typed, 1)` — so rewriting that helper is a good first exercise in reading someone else's code and simplifying it.
- **`todo.kumu`** is the largest program in the project, and the one that uses this chapter's dates in anger: a to-do list with optional due dates and tags, priorities, completion that archives an item in place rather than deleting it, and views that work out what's overdue or due this week every time you look. Two decisions in it are worth the read. Items are addressed by a stable `Id` rather than by row number, because sorting rewrites the row order and "complete number 3" would otherwise mean different things before and after a sort. And priority is stored as `1`, `2`, `3` rather than `"high"`, `"med"`, `"low"`, so that `.SORT` sees a numeric column — sorting the words would give you `high, low, med`, which is alphabetical and useless. Read it after this chapter.

A few exercises to try on your own:
1. Write a function `isPalindrome(s)` that returns `1` if a string reads the same backward as forward. Try it twice: once with `LEN` and `SUBSTRING`, and once with `SPLIT(s, "")`, `.REVERSE()` and `.JOIN("")` — comparing the two is a good way to feel what lists are for.
2. Extend the guessing game to track and print how many guesses it took, and to keep the list of guesses so it can show them all at the end.
3. Add a numeric "Age" column to a copy of the address book's starter data, and a Tools option that reports the average age with `.AVERAGE`.
4. Rewrite `address.kumu`'s `indexOfContact()` to use `.FIND(column, value, 1)` instead of its hand-rolled loop, and check the behaviour is unchanged.
5. Write a word-frequency counter: `SPLIT` a sentence on spaces, then for each word report how many times it appears using `.CONTAINS` and `.INDEX_OF`.
6. Write `daysUntilBirthday(birthday)`: given a date in any year, work out how many days until the next one. `MONTH` and `DAY` will get you the parts; the year needs a decision about what happens when the day has already gone past this year.
7. Add a "due this week" view to `todo.kumu` that groups items by `WEEKDAY` rather than listing them flat.

For the complete, precise behavior of every keyword and function — including edge cases this book doesn't dwell on — see the [Manual]({{ site.baseurl }}/manual/).

---

## Appendix A: Answers to Exercises

Try each exercise before reading the answer. Getting it wrong and finding out
why is most of where the learning happens.

### Chapter 1: Hello, World

**1.** `6 * 7` on the first line, then `42` on the second.

Anything inside double quotes is text and is shown exactly as written. Without
quotes it's a sum, and Kumu works it out before printing.

**2.** `print` is lowercase, and Kumu keywords are always capitals — so `print`
is just a name, not the `PRINT` statement. The message says so directly:

<!-- check
print "Kumu"
-->
```
Syntax Error: 'print' is not a keyword -- Kumu keywords are written in capitals. Did you mean 'PRINT'? (line 1)
```

Fixing it is a matter of the shift key:

```kumu
PRINT "Kumu"
```

Output:
```
Kumu
```

**3.** The string is never closed — there's no `"` at the end of the line:

<!-- check
PRINT "Hello
-->
```
Syntax Error: Unterminated string -- it is missing a closing '"' (line 1)

 1 | PRINT "Hello
   |       ^
```

**4.** Two statements, one per line:

```kumu
PRINT "Ada"
PRINT 42
```

Output:
```
Ada
42
```

### Chapter 2: Variables

**1.** `20`, then `5`.

`VAR y = x` copies the *value* that `x` held at that moment. Changing `x`
afterwards doesn't reach back and change `y` — they're two separate boxes.

**2.** `4`.

`count` starts at 1. The first line doubles it to 2, the second doubles that to
4. Each line uses the current value on the right before storing the new one.

**3.** `prise` is a misspelling of `price`. Because writing to an unknown name
creates a new variable rather than failing, `prise` quietly comes into existence
holding `30` while `price` stays at `10`. Fix the spelling:

```kumu
VAR price = 10
price = price * 3
PRINT price
```

Output:
```
30
```

**4.** Area is `width * height`, perimeter is twice the sum of the sides:

```kumu
VAR width = 8
VAR height = 5
PRINT width * height
PRINT 2 * (width + height)
```

Output:
```
40
26
```

**5.** You need the third variable because the first assignment destroys what
you're about to need. If you wrote `a = b` first, the original `"first"` would
be gone before you could put it into `b`. So stash it:

```kumu
VAR a = "first"
VAR b = "second"
VAR temp = a
a = b
b = temp
PRINT a
PRINT b
```

Output:
```
second
first
```

### Chapter 3: Expressions and Operators

**1.** `7`, `5`, `1`, `3`.

`1 + 2 * 3` is `1 + 6` because `*` binds tighter. `100 / 10 / 2` applies left to
right: `10 / 2`. `7 MOD 3` is the remainder, `1`. And `MOD` binds as tightly as
`*`, so `1 + 6 MOD 4` is `1 + 2`.

**2.** `0`, `1`, `1`, `1`.

The number `5` and the text `"5"` are different values, so `==` says no and `!=`
says yes. `""` is the empty string, which counts as false, so `!""` is `1`.
`0 OR 3` is true because the right side is non-zero.

**3.** The warning is:

<!-- check
VAR total = 10
IF total = 10 {
    PRINT "ten"
}
-->
```
Warning (line 2): '=' used as a comparison -- did you mean '=='?
```

It still runs, because Kumu reads a lone `=` in a condition as a comparison. But
write what you mean:

```kumu
VAR total = 10
IF total == 10 {
    PRINT "ten"
}
```

Output:
```
ten
```

**4.** Put a guard on the left of an `AND`. Because `AND` short-circuits, the
division never happens when `items` is `0`:

```kumu
VAR items = 0
VAR cost = 100
IF items != 0 AND cost / items > 10 {
    PRINT "expensive"
} ELSE {
    PRINT "no items"
}
```

Output:
```
no items
```

**5.** A number is even when dividing by 2 leaves no remainder:

```kumu
VAR n = 7
IF n MOD 2 == 0 {
    PRINT "even"
} ELSE {
    PRINT "odd"
}
```

Output:
```
odd
```

**6.** Read the rule as one sentence: divisible by 4, *and* (not divisible by
100 *or* divisible by 400). The parentheses are essential — without them `AND`
would bind tighter than `OR` and the meaning would change:

```kumu
VAR year = 2000
IF year MOD 4 == 0 AND (year MOD 100 != 0 OR year MOD 400 == 0) {
    PRINT "leap"
} ELSE {
    PRINT "not leap"
}
```

Output:
```
leap
```

Change `year` to `1900` and it prints `not leap`; to `2024` and it prints `leap`.

### Chapter 4: Making Decisions with IF

**1.** `falsy`. `n` holds `0`, and `0` is the one Number Kumu treats as false.

**2.** `C`.

The chain runs the first branch that matches and skips everything after it. `95`
is indeed `>= 70`, so `C` prints and the `>= 80` and `>= 90` tests are never
reached. This is the ordering bug in the chapter, and note that nothing warns
you — the program runs happily and gives the wrong grade.

**3.** Put the most specific condition first:

```kumu
VAR score = 95
IF score >= 90 {
    PRINT "A"
} ELSEIF score >= 80 {
    PRINT "B"
} ELSEIF score >= 70 {
    PRINT "C"
}
```

Output:
```
A
```

**4.** Because the chain stops at the first match, each later test only has to
rule out its own upper bound:

```kumu
VAR age = 12
IF age < 13 {
    PRINT "child"
} ELSEIF age < 20 {
    PRINT "teen"
} ELSE {
    PRINT "adult"
}
```

Output:
```
child
```

**5.** Same shape:

```kumu
VAR hour = 14
IF hour < 12 {
    PRINT "Good morning"
} ELSEIF hour < 18 {
    PRINT "Good afternoon"
} ELSE {
    PRINT "Good evening"
}
```

Output:
```
Good afternoon
```

### Chapter 5: Getting Input from the User

**1.** Typing `10` prints `15`. Typing `ten` prints `ten5`.

`10` looks like a number, so `value` is a Number and `+` adds. `ten` doesn't, so
`value` is a String and `+` joins — giving `ten5` with no error at all. This is
exactly why you check before doing arithmetic on input.

**2.** `not confirmed`.

The user typed text, so `answer` holds the String `"yes"`, and a String is never
equal to the Number `1`. Comparing different types is allowed and always answers
"not equal". To test it properly, compare against what you'll actually get:
`IF answer == "yes"`.

**3.** Two prompts, two reads, one line of output:

```kumu
PRINT "Name?"
INPUT name
PRINT "Town?"
INPUT town
PRINT name + " is from " + town
```

A sample run, typing `Ada` then `London`:
```
Name?
Town?
Ada is from London
```

**4.** Check first, convert, then test the remainder:

```kumu
PRINT "Number?"
INPUT v
IF IS_NUMBER(v) {
    IF NUMBER(v) MOD 2 == 0 {
        PRINT "even"
    } ELSE {
        PRINT "odd"
    }
} ELSE {
    PRINT "not a number"
}
```

Typing `7` prints `odd`; typing `cat` prints `not a number`.

### Chapter 6: Loops

**1.** `3`, `2`, `1`, `0`.

The loop prints 3, 2 and 1. When `n` reaches `0` the condition `n > 0` is false,
so the loop ends — but `n` still exists afterwards, holding `0`, and the final
`PRINT n` shows it. A loop variable doesn't disappear when the loop finishes.

**2.** `1`, `3`.

Pass by pass: `i` of 1 prints. `i` of 2 hits `CONTINUE`, so the `PRINT` is
skipped. `i` of 3 prints. `i` of 4 hits `BREAK`, which ends the loop — so 5 is
never reached at all.

**3.** Nothing changes `i`, so `i <= 3` is true forever. It needs the update —
question 3 from the start of the chapter:

```kumu
VAR i = 1
WHILE i <= 3 {
    PRINT i
    i = i + 1
}
```

Output:
```
1
2
3
```

**4.** `FOR` only counts upward unless you tell it otherwise, and `i` starts at
5, which is already past the end value of 1 — so the loop body never runs even
once. Add `STEP -1`:

```kumu
FOR i = 5 TO 1 STEP -1 {
    PRINT i
}
```

Output:
```
5
4
3
2
1
```

**5.** A `FOR` loop over 1 to 12, building each line with `+`:

```kumu
FOR i = 1 TO 12 {
    PRINT "7 x " + i + " = " + (7 * i)
}
```

Output:
```
7 x 1 = 7
7 x 2 = 14
7 x 3 = 21
7 x 4 = 28
7 x 5 = 35
7 x 6 = 42
7 x 7 = 49
7 x 8 = 56
7 x 9 = 63
7 x 10 = 70
7 x 11 = 77
7 x 12 = 84
```

The parentheses around `7 * i` aren't strictly needed — `*` binds tighter than
`+` — but they make the intent obvious at a glance.

**6.** You can't say up front how many doublings fit under 1000, so this is a
`WHILE`:

```kumu
VAR n = 1
WHILE n <= 1000 {
    PRINT n
    n = n * 2
}
```

Output:
```
1
2
4
8
16
32
64
128
256
512
```

`n` does reach 1024, but the condition is checked *before* each pass, so 1024 is
never printed.

### Chapter 7: Functions

**1.** `20`, then `10`.

Inside `bump`, `VAR x = x + 10` reads the parameter (`10`), adds ten, and stores
`20` in the call's *own* `x`. The global `x` is a different variable that shares
a name, so it's still `10` when the function returns. Nothing a function does to
its own variables can reach outward — unless you use bare assignment, which is
the next section of the chapter.

**2.** `10`.

`mystery` adds `n` to the result of calling itself with `n - 1`, stopping at
zero: `4 + 3 + 2 + 1 + 0`. It sums the numbers from `n` down to 1.

**3.** There's no base case. `countdown` calls itself forever, sailing straight
past zero into negative numbers, until Kumu stops it:

<!-- check
FUNCTION f(n) {
    RETURN f(n + 1)
}
PRINT f(1)
-->
```
Runtime Error: Maximum recursion depth (1000) exceeded -- is this function missing a base case?
```

Add a condition that returns without recursing:

```kumu
FUNCTION countdown(n) {
    IF n <= 0 {
        RETURN 0
    }
    PRINT n
    RETURN countdown(n - 1)
}
countdown(3)
```

Output:
```
3
2
1
```

**4.** Straight translation of the formula:

```kumu
FUNCTION celsius(f) {
    RETURN (f - 32) * 5 / 9
}
PRINT celsius(212)
PRINT celsius(98.6)
PRINT celsius(32)
```

Output:
```
100
37
0
```

**5.** A comparison already produces `1` or `0`, so you can return it directly
rather than writing an `IF` that returns `1` or `0` itself:

```kumu
FUNCTION isEven(n) {
    RETURN n MOD 2 == 0
}
IF isEven(18) {
    PRINT "18 is even"
} ELSE {
    PRINT "18 is odd"
}
```

Output:
```
18 is even
```

**6.** The base case is exponent `0`; each step multiplies by `base` and moves
the exponent one closer to it:

```kumu
FUNCTION power(base, exponent) {
    IF exponent == 0 {
        RETURN 1
    }
    RETURN base * power(base, exponent - 1)
}
PRINT power(2, 10)
```

Output:
```
1024
```

### Chapter 8: Text Processing with String Functions

**1.** `4`, `Ku`, `3`, `0`.

`SUBSTRING(s, 1, 2)` starts at character **1** and takes two. `"mu"` begins at
character 3. And `"z"` isn't there, so `INDEX_OF` gives `0` — not `-1`, because
position `0` doesn't exist in a language that counts from 1.

**2.** `1`, `1`, `0`, `0`.

`"4.2"` is a perfectly good number. `"42abc"` is not — the *whole* string has to
read as one. An empty string isn't a number either.

**3.** Check before converting:

```kumu
VAR typed = "abc"
IF IS_NUMBER(typed) {
    PRINT NUMBER(typed) + 1
} ELSE {
    PRINT "not a number"
}
```

Output:
```
not a number
```

**4.** Find the space, then take one character from each side of it:

```kumu
VAR full = "Ada Lovelace"
VAR space = INDEX_OF(full, " ")
PRINT SUBSTRING(full, 1, 1) + "." + SUBSTRING(full, space + 1, 1) + "."
```

Output:
```
A.L.
```

**5.** Upper the first character, lower everything from the second on. Passing
`LEN(w)` as the length is the idiom for "all the rest" — it's clamped to
whatever is actually left:

```kumu
VAR w = "kUMU"
PRINT UPPER(SUBSTRING(w, 1, 1)) + LOWER(SUBSTRING(w, 2, LEN(w)))
```

Output:
```
Kumu
```

### Chapter 9: Handling Errors with TRY/CATCH

**1.** `before`, `caught`, `done`.

`after` never prints. The moment the division fails, control leaves the `TRY`
block for the `CATCH` — the rest of the block is abandoned, not resumed.

**2.** Wrap the part that can fail:

```kumu
TRY {
    INPUT name
    PRINT "Hello, " + name
} CATCH e {
    PRINT "no input"
}
PRINT "continuing"
```

With nothing to read, that prints `no input` then `continuing` instead of
stopping.

**3.** Let the failure happen and catch it:

```kumu
FUNCTION safeNumber(t) {
    TRY {
        RETURN NUMBER(t)
    } CATCH e {
        RETURN 0
    }
}
PRINT safeNumber("42")
PRINT safeNumber("abc")
```

Output:
```
42
0
```

**4.** Because `0` is a perfectly good number, the caller can't tell "the text
was `0`" from "the text was nonsense". Any sentinel you pick has the same
problem — some input could legitimately produce it. Better options: return the
text unchanged and let the caller check with `IS_NUMBER`, or don't catch at all
and let the caller decide. In general, a function that swallows an error and
returns something plausible is harder to debug than one that lets it through.

### Chapter 10: Debugging with TRACE

**1.**

```
[TRACE line 2] VAR x = 2
[TRACE line 3] ASSIGN x = 3
[TRACE line 4] PRINT
3
[TRACE line 5] TRACE
```

Two things to notice. `VAR x` and `ASSIGN x` are labelled differently, so you
can see which line declared and which reassigned, and each shows the value that
landed. The `PRINT` line shows no value — only statements that change a variable
do — and its trace appears before the `3`, because the commentary is printed as
the statement runs.

**2.** The trace shows `ASSIGN totl = 1`, then `= 2`, then `= 3` — three
assignments to a name that isn't `total`, and values that restart rather than
accumulate. The bug is `totl`, a misspelling of `total`. Because writing to an unknown name creates a new variable rather than
failing, `totl` quietly absorbed every update while `total` stayed at `0`:

```kumu
VAR total = 0
FOR i = 1 TO 3 {
    total = total + i
}
PRINT total
```

Output:
```
6
```

### Chapter 11: Random Numbers

**1.** `5`, every time. The range includes both ends, so a range of `5` to `5`
has exactly one possible answer.

**2.** `min` must not be greater than `max`:

<!-- check
PRINT RANDOM(10, 1)
-->
```
Runtime Error: RANDOM: min must be <= max (line 1)
```

The author almost certainly meant `RANDOM(1, 10)`.

**3.** Two separate calls — one call can't roll two dice:

```kumu
VAR first = RANDOM(1, 6)
VAR second = RANDOM(1, 6)
PRINT "You rolled " + (first + second)
```

**4.** A range of two values, one for each face:

```kumu
IF RANDOM(0, 1) == 1 {
    PRINT "heads"
} ELSE {
    PRINT "tails"
}
```

### Chapter 12: Console Control

**1.** A `FOR` loop counting down, with a pause each pass:

```kumu
FOR i = 3 TO 1 STEP -1 {
    PRINT i
    SLEEP(0.5)
}
PRINT "Go!"
```

Output:
```
3
2
1
Go!
```

The output looks the same as it would without the `SLEEP` — the difference is
entirely in the timing, which is why you have to run it to see the point.

**2.**

```kumu
PRINT "1. Start"
PRINT "2. Quit"
SLEEP(1)
CLEAR_SCREEN()
PRINT "Cleared."
```

### Chapter 13: Lists

**1.** `[1, 2, 3]`, then `[1, 2, 3, 4]`.

The first `xs.APPEND(4)` builds a new list and immediately throws it away,
because nothing is done with the result — `xs` is untouched. The second prints
the returned list directly. To keep the change you'd write `xs = xs.APPEND(4)`.

**2.** `[9, 10, 100]`, `["9", "10", "100"]`, `["apple", "banana", "cherry"]`.

The second is the interesting one. Those are strings, but every one of them
*reads* as a number, so `.SORT()` sorts them numerically — which is why `"9"`
comes before `"10"` rather than after it. Text order would have given
`["10", "100", "9"]`.

**3.** `SPLIT` gives a list of strings, and the arithmetic methods accept text
that reads as a number:

```kumu
VAR raw = "4,8,15,16"
PRINT SPLIT(raw, ",").AVERAGE()
```

Output:
```
10.75
```

**4.** An empty separator splits into single characters:

```kumu
VAR w = "stressed"
PRINT SPLIT(w, "").REVERSE().JOIN("")
```

Output:
```
desserts
```

**5.** `FOR EACH` and a condition:

```kumu
VAR names = ["Ada", "Grace", "Alan", "Barbara"]
FOR EACH n IN names {
    IF LEN(n) > 4 {
        PRINT n
    }
}
```

Output:
```
Grace
Barbara
```

### Chapter 14: Files and Matrices

**1.** `2`, `2`, `2`.

The Matrix has three rows written down, but row 1 is the header, so `.ROWS()`
reports `2` data rows. `.COLS()` counts the columns, which is also `2` here.
`LEN` on a Matrix agrees with `.ROWS()`.

**2.** The new Matrix was built and discarded. `APPEND_ROW` doesn't change `d` —
nothing in Kumu changes a collection in place:

```kumu
VAR d = (("A"),("1"))
d = d.APPEND_ROW("2")
PRINT d.ROWS()
```

Output:
```
2
```

**3.** Name the column in the header and let `.SUM` find it:

```kumu
VAR d = (("Item","Cost"),("pen","2"),("pad","5"),("ink","9"))
PRINT d.SUM("Cost")
```

Output:
```
16
```

**4.** Assign the new row back, save, then read:

```kumu
VAR d = (("Item","Cost"),("pen","2"),("pad","5"),("ink","9"))
d = d.APPEND_ROW("cap","4")
d.SAVE("costs.csv")
PRINT READ_CSV("costs.csv").ROWS()
```

Output:
```
4
```

### Chapter 15: Working with Dates

**1.** `-1`. `DATE_DIFF` counts from the first date to the second, and the
second one here is a day *earlier*, so the count comes back negative. The sign
is the whole point: a negative result is how you recognise a date that has
already passed.

**2.** `0`. There is nothing wrong with the 15th of April — the problem is how
it's written. Every part of a Kumu date is a fixed width, so the month needs its
padding zero: `2026-04-15` is accepted.

**3.** The separators are slashes instead of dashes, so it isn't a Kumu date at
all:

<!-- check
VAR deadline = "2026/12/25"
PRINT DATE_DIFF("2026-09-24", deadline)
-->
```
Runtime Error: DATE_DIFF to: '2026/12/25' is not a valid date -- dates are written as YYYY-MM-DD (e.g. 2026-09-23) (line 2)
```

Writing it `"2026-12-25"` fixes it. Notice the message names which of the two
arguments was the bad one.

**4.** Ask first, then act — the same shape as `IS_NUMBER` in Chapter 5:

```kumu
PRINT "Type a date (YYYY-MM-DD):"
INPUT typed
IF IS_DATE(typed) {
    PRINT typed + " is a " + WEEKDAY(typed) + "."
} ELSE {
    PRINT "That isn't a date I can read."
}
```

**5.** Sorting puts the earliest first, and you can index the result straight
away:

```kumu
VAR dates = ["2026-12-14", "2026-01-05", "2026-09-23"]
PRINT dates.SORT()[1]
```

Output:
```
2026-01-05
```

`.MIN()` would *not* work here: it's for numbers, and these are strings. Sorting
is the right tool precisely because the format makes text order match time
order.

**6.** A fortnight is fourteen days:

```kumu
VAR fortnight = DATE_ADD(TODAY(), 14)
PRINT "A fortnight from today is " + fortnight + ", a " + WEEKDAY(fortnight) + "."
```

### Chapter 16: Comments

**1.** `# hello`.

Inside quotes a `#` is just a character, so the first one is printed. The second
`#` is outside the string and does start a real comment, which Kumu ignores.

**2.** No fixed answer — but if you found a comment you wanted to delete, that's
the exercise working. The test is whether a comment tells you something the line
below it doesn't already say.

