Timed Regular Expressions
=========================

In this document, we show how to use MONAA to monitor a specification written in timed regular expressions. We assume that you know the basics of timed regular expressions (e.g., [ACM02]) and what MONAA does. If you have never used MONAA, we recommend to read [Getting Started](./getting_started.md), which contains some examples of the TREs.

Timed Regular Expressions
-------------------------

In MONAA, the given timed regular expression is translated into a timed automaton and the timed pattern matching problem is conducted for the translated timed automaton. The timed regular expression supported in MONAA is defined as follows. Here, we use the blank character (' ') to represent the each elements, but MONAA interprets ' ' as an event and you have to omit these blank character when you actually use MONAA. You can also read the code of the [parser](https://github.com/MasWag/monaa/blob/master/monaa/tre_parser.yy) and the [lexer](https://github.com/MasWag/monaa/blob/master/monaa/tre_lexer.l).

```
expr : c (An event)
     | ( expr ) (Grouping)
     | expr + (Kleene Plus)
     | expr * (Kleene Star)
     | expr expr (Concatenation)
     | expr | expr (Disjunction)
     | expr & expr (Conjunction)
     | expr % (s,t) (Time Restriction)
```

For the basic examples, please read [Getting Started](./getting_started.md). We only show advanced specifications here.

Conjunction
-----------

You can use conjunction to represent some complicated timing constraints.
Here, we use the following timed word in `example/getting_started/timed_word2.txt`.
```
A 0.5
B 0.8
C 1.5
A 2.0
B 2.9
C 4.2
```

![The example timed word 2](./fig/getting_started/timed_word2.svg)

The following expression matches a consecutive occurrences of the events A, B, and C such that the duration between the A and B is less than 1 and the duration between the B and C is more than 1.

```
../build/monaa -e '(((AB)%(0,1)C)&(A(BC)%(1,20)))$' < ../examples/getting_started/timed_word2.txt
```

```
  1.500000       <= t <   2.000000
  4.200000        < t' <=        inf
  2.200000        < t' - t <=        inf
=============================
```

Terminate character
-------------------

The following expression matches a consecutive occurrences of the events A, B, and C such that the blank interval after C is longer than 1.

```
../build/monaa -e 'ABC($)%(1,20)' < ../examples/getting_started/timed_word2.txt
```

```
  1.500000       <= t <   2.000000
  5.200000        < t' <  24.200000
  3.200000        < t' - t <  22.700000
=============================
```

References
-------------

- [ACM02] Timed regular expressions. Eugene Asarin, Paul Caspi, and Oded Maler, Journal of the ACM, Volume 49 Issue 2, March 2002, Pages 172-206
