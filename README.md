MONAA --- A Tool for Timed Patten Matching with Automata-Based Acceleration [![wercker status](https://app.wercker.com/status/5179a3c60c6d9988fcb04fad81aa1d4c/s/master "wercker status")](https://app.wercker.com/project/byKey/5179a3c60c6d9988fcb04fad81aa1d4c)
============================================================================================================================================================================================

This is the source code repository for MONAA --- A Tool for Timed Patten Matching with Automata-Based Acceleration.

Usage
-----

### Synopsis

    monaa [OPTIONS] PATTERN [FILE]
    monaa [OPTIONS] -e PATTERN [FILE]
    monaa [OPTIONS] -f FILE [FILE]

### Options

**-h**, **--help** Print a help message. <br />
**-q**, **--quiet** Quiet mode. Causes any results to be suppressed. <br />
**-a**, **--ascii** Ascii mode. (default) <br />
**-b**, **--binary** Binary mode. <br />
**-V**, **--version** Print the version <br />
**-E**, **--event** Event mode (default) <br />
**-S**, **--signal** Signal mode <br />
**-i** *file*, **--input** *file* Read a timed word from *file*. <br />
**-f** *file*, **--automaton** *file* Read a timed automaton from *file*. <br />
**-e** *pattern*, **--expression** *pattern* Specify a *pattern* by a timed regular expression. <br />

Installation
------------

MONAA is tested on Arch Linux and Mac OSX 10.11.6

### Requirements

* C++ compiler supporting C++14 and the corresponding libraries.
* Boost
* Eigen
* CMake
* Bison
* Flex

### Instructions

```sh
mkdir build
cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make && make install
```

Syntax of Timed Automata
------------------------

You can use [DOT language](http://www.graphviz.org/content/dot-language) to represent a timed automaton. For the timing constraints and other information, you can use the following custom attributes.

<table>
<thead>
<tr class="header">
<th></th>
<th>attribute</th>
<th>value</th>
<th>description</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td>vertex</td>
<td>init</td><td>0 or 1</td><td><tt>init=1</tt> if the state is initial</td></tr>
<tr class="even">
<td>vertex</td><td>match</td><td>0 or 1</td><td><tt>match=1</tt> if the state is accepting</td>
</tr>
<tr class="odd">
<td>edge</td><td>label</td><td>[a-z], [A-Z]</td><td>the value represents the event on the transition</td>
</tr>
<tr class="even">
<td>edge</td><td>reset</td><td>a list of integers</td><td>the set of variables reset after the transition</td>
</tr>
<tr class="odd">
<td>edge</td><td>guard</td><td>a list of inequality constraints</td><td>the guard of the transition</td>
</tr>
</tbody>
</table>

Syntax of Timed Regular Expressions
-----------------------------------


    expr : c (An event)
         | ( expr ) (Grouping)
         | expr + (Kleene Plus)
         | expr * (Kleene Star)
         | expr expr (Concatenation)
         | expr | expr (Disjunction)
         | expr & expr (Conjunction)
         | expr % (s,t) (Time Restriction)


References
-------------

* A Boyer-Moore Type Algorithm for Timed Pattern Matching. Masaki Waga, Takumi Akazaki, and Ichiro Hasuo
* Efficient Online Timed Pattern Matching by Automata-Based Skipping. Masaki Waga, Ichiro Hasuo, and Kohei Suenaga
* MONAA: a Tool for Timed Patten Matching with Automata-Based Acceleration. Masaki Waga, Ichiro Hasuo, and Kohei Suenaga
