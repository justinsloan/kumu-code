---
title: Goals
layout: default
nav_order: 2
permalink: /goals/
---

# What Kumu is for

Kumu exists to be somebody's first language. Every choice in it was made with one
question in mind: *does this help a beginner, or does it just help the person who wrote
the interpreter?*

This page explains what that means in practice, so you can decide whether Kumu is worth
your time before you commit to the textbook.

## Familiar, without the baggage

Kumu is BASIC-inspired, because BASIC got something right: `PRINT "hello"` does what it
looks like it does. Keywords are ordinary English words in capitals, and a program is a
list of instructions that runs from the top. Nothing about that needs improving.

What does need improving is everything BASIC built on top of it. Kumu has **no line
numbers** and **no `GOTO`**. Programs are organised into blocks and functions, the way
every language you might learn next is organised, so the habits you build here transfer
instead of having to be unlearned.

## One shape to learn

Most languages ask you to memorise several different ways of writing the same idea —
braces here, a colon and indentation there, an `end` keyword somewhere else. Kumu has
exactly one:

```kumu
IF score >= 60 {
    PRINT "pass"
} ELSE {
    PRINT "fail"
}

WHILE tries < 3 {
    tries = tries + 1
}

FOR EACH name IN names {
    PRINT name
}

FUNCTION greet(who) {
    PRINT "Hello, " + who
}
```

A header, then `{ ... }`. `IF`, `ELSEIF`, `ELSE`, `WHILE`, `FOR`, `FOR EACH`, `FUNCTION`,
`TRY` and `CATCH` all read the same way, so there is never a question of which style a
construct wants.

Braces do that job and nothing else. Round brackets group a sub-expression and carry a
function's arguments; square brackets are for Lists. Each kind of bracket means one
thing.

## The error messages are the point

Here is the part worth reading even if you decide Kumu isn't for you.

A beginner spends far more time reading error messages than reading successful output.
That makes the error messages the part of the language you actually interact with most
while learning — and in most languages they are an afterthought written for the compiler
author's convenience.

In Kumu they got the most design effort of anything. Every uncaught error shows the line
that failed with a caret under the exact spot, and where it can, it tells you what you
probably meant:

```
Runtime Error: Undefined variable: totl -- did you mean 'total'? (line 3)

 3 | PRINT totl / count
   |       ^
```

When the failure happens inside a function, you get the chain of calls that led there,
innermost first, so you can see *how* you arrived at the problem and not just where it
landed:

```
Runtime Error: Division by zero (line 2)

 2 |     RETURN x / 0
   |              ^
  in half() called at line 4
```

Kumu also notices the specific mistakes beginners actually make, rather than failing
somewhere further along where the real cause is invisible. Typing a keyword in lowercase,
writing `=` where you meant `==`, doing arithmetic on two strings, calling a method that
doesn't exist — each gets its own message naming the problem. Nothing fails silently: a
stray `%` or `;` is reported, not skipped.

## Collections that can't surprise you

There are two ways to hold more than one value. A **List** is a sequence, indexed from 1,
holding any values at all. A **Matrix** is a table with a permanent header row, for data
that would otherwise live in a spreadsheet.

Both follow one rule that is unusual for this kind of language, and deliberate: **no
method ever changes the collection you called it on.** Every method that modifies data
returns a *new* collection.

```kumu
book = book.APPEND_ROW("Cy", "555-0199", "cy@example.com")
```

Not `book.APPEND_ROW(...)` on its own — that computes a new book and throws it away.

This is the one place where Kumu asks a little more of you than the obvious design would.
It is worth it because aliasing bugs, where two names refer to the same underlying data
and changing one silently changes the other, are genuinely baffling the first time you
meet them. Here they cannot happen. Everything else in the language is ordinary,
mutable-by-default, imperative code.

## What Kumu is not

Being honest about the limits is part of being useful:

- **It is not a production language.** There is no package manager, no ecosystem of
  libraries, and no plan for one.
- **There is no automated test suite yet.** The example programs demonstrate the
  language; they do not verify it.
- **There is no installer.** You build the interpreter and run it out of the source
  directory.
- **It will not be the last language you learn**, and it isn't meant to be. It is meant
  to make the second one easier.

## Start here

Ready to try it? The **[Textbook]({{ site.baseurl }}/textbook/)** starts from
`PRINT "Hello, World"` and ends with a working guessing game, seventeen chapters later.
Every example in it has been run through the interpreter — the output you see printed is
real, not guessed.
