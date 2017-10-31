libmonaa
========

This directory contains libmonaa, the C++ API of MONAA.

Overview
--------

This library mainly provides the function monaa, which does do online timed pattern matching by the timed FJS algorithm, and as its parameters, the classes WordContainer, TimedAutomaton, and AnsContainer. The input and output container classes WordContainer and AnsContainer just define the interface of the container, and the classes passed by their template arguments defines the procedure. Therefore, users can define the functionality suitable for their application. For example, AnsContainer::push_back() can be used as a call back function when the procedure finds a matching in the input.

For the detail, see [the reference](https://maswag.github.io/monaa/).


Installation
------------

libmonaa is tested on Arch Linux and Mac OSX 10.11.6

### Requirements

* C++ compiler supporting C++14 and the corresponding libraries.
* Boost
* Eigen
* CMake

### Instructions

The build of libmonaa is also done when MONAA is compiled. Please install libmonaa.a and the header files to appropriate places.
