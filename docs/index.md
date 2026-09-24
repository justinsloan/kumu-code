---
title: Home
layout: default
nav_order: 1
permalink: /
---

# Kumu

A small, BASIC-inspired programming language built for teaching — with a hand-written
interpreter in C++17, a reference manual, and a 16-chapter textbook.
{: .kumu-tagline }

Kumu keeps the friendly parts of BASIC — `PRINT`, `IF`, `FOR`, keywords in capitals —
and drops the parts that make it hard to learn from. There are no line numbers and no
`GOTO`. Every block in the language is written exactly one way, a header followed by
`{ ... }`, so there is only ever one shape to learn.

```kumu
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

## Where to start

If you are learning to program, start with the **[Textbook]({{ site.baseurl }}/textbook/)**.
It is written to be worked through in order, on your own, with a runnable example for
nearly every idea and answers to every exercise in the back.

If you already program and just want the specifics, the
**[Manual]({{ site.baseurl }}/manual/)** is the reference: ten sections covering the full
syntax, every built-in, and the precise semantics.

The **[Goals]({{ site.baseurl }}/goals/)** page explains what Kumu is trying to do and
why it is shaped the way it is.

## Running it

Kumu needs `g++` with C++17 support and `make`. There are no other dependencies.

```sh
git clone https://github.com/justinsloan/kumu-code.git
cd kumu-code
make
./kumu collections.kumu
```

`make` produces a single `kumu` binary. To run your own program, save it with a `.kumu`
extension and pass it in:

```sh
./kumu myprogram.kumu
```

## Example programs

The repository ships four programs you can read and run:

| Program | What it shows |
| :--- | :--- |
| [`todo.kumu`](https://github.com/justinsloan/kumu-code/blob/main/todo.kumu) | A complete to-do manager — due dates, tags, priorities, completion and an archive, sorted any way you like and persisted to `todo.csv`. The largest program here. |
| [`address.kumu`](https://github.com/justinsloan/kumu-code/blob/main/address.kumu) | The flagship example — an interactive address book using essentially every feature together, persisting contacts to a CSV file. |
| [`collections.kumu`](https://github.com/justinsloan/kumu-code/blob/main/collections.kumu) | Matrix vs. List: when you want a table and when you want a sequence. |
| [`test.kumu`](https://github.com/justinsloan/kumu-code/blob/main/test.kumu) | A broad feature sweep, useful as a syntax reference by example. |
| [`hello.kumu`](https://github.com/justinsloan/kumu-code/blob/main/hello.kumu) | A short tour that ends with a deliberate mistake, to show what an error looks like. |
