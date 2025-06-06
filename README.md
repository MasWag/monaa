MONAA --- A Tool for Timed Patten Matching with Automata-Based Acceleration
===========================================================================

[![Boost.Test](https://github.com/MasWag/monaa/actions/workflows/boosttest.yml/badge.svg?branch=master)](https://github.com/MasWag/monaa/actions/workflows/boosttest.yml)
[![Documentation Status](https://readthedocs.org/projects/monaa/badge/?version=latest)](https://monaa.readthedocs.io/en/latest/?badge=latest)
[![Docker Pulls](https://img.shields.io/docker/pulls/maswag/monaa)](https://hub.docker.com/r/maswag/monaa)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](./LICENSE)


This is the source code repository for MONAA --- A Tool for Timed Patten Matching with Automata-Based Acceleration.

Demo on Google Colab is available!!

[![Open In Colab (demo)](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/MasWag/monaa/blob/master/demo/MONAA%20demo.ipynb)
[![Open In Colab (velocity demo)](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/MasWag/monaa/blob/master/demo/MONAA%20velocity%20demo.ipynb)

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

MONAA is tested on Arch Linux, Ubuntu (20.04, 22.04, 24.04), and macOS (12 Monterey, 13 Ventura, 14 Sonoma). We also provide a [docker image](https://hub.docker.com/repository/docker/maswag/monaa).

### Requirements

* C++ compiler supporting C++20 and the corresponding libraries.
* Boost (>= 1.59)
* Eigen
* CMake (>= 3.30)
* Bison (>= 3.0)
* Flex

### Instructions

```sh
mkdir build
cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make && make install
```

### Usage of the docker image

Note: Docker support is experimental. It seems this image does not work with Docker Desktop for macOS.

You can use monaa via docker by `docker run -it maswag/monaa ...` instead of `monaa ...`.
The following shows an example.

```sh
docker run -v $PWD:/mnt -it maswag/monaa -f /mnt/examples/small.dot -i /mnt/examples/small.txt
```

Examples
--------

See [Getting Started](./doc/getting_started.md) for an example usage.

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
         | expr % (s,t) | expr % [s,t) | expr % (s,t] | expr % [s,t] | expr % (>s) | expr % (>=s) | expr % (<t) | expr % (<=t) | expr % (=t) (Time Restriction)

Related Tool
-------------

- [TimeTrace](https://time-trace.vercel.app/): A web frontend of MONAA
    - GitHub: https://github.com/JonasGLund99/TimeTrace

References
-------------

* A Boyer-Moore Type Algorithm for Timed Pattern Matching. Masaki Waga, Takumi Akazaki, and Ichiro Hasuo
* Efficient Online Timed Pattern Matching by Automata-Based Skipping. Masaki Waga, Ichiro Hasuo, and Kohei Suenaga
* MONAA: a Tool for Timed Patten Matching with Automata-Based Acceleration. Masaki Waga, Ichiro Hasuo, and Kohei Suenaga
