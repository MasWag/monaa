% MONAA(1)
% Masaki Waga
% May 2018

# NAME

monaa - MONitoring tool Accelerated by Automata

# SYNOPSIS

    monaa [OPTIONS] PATTERN [FILE]
    monaa [OPTIONS] -e PATTERN [FILE]
    monaa [OPTIONS] -f FILE [FILE]

# DESCRIPTION

**Monaa** is a tool for timed patten matching with automata-based acceleration.

# OPTIONS

**-h**, **--help**
: Print a help message.

**-q**, **--quiet**
: Quiet mode. Causes any results to be suppressed.

**-a**, **--ascii**
: Ascii mode. (default)

**-b**, **--binary**
: Binary mode.

**-V**, **--version**
: Print the version

**-E**, **--event**
: Event mode (default)

**-S**, **--signal**
: Signal mode

**-i** *file*, **--input** *file*
: Read a timed word from *file*.

**-f** *file*, **--automaton** *file*
: Read a timed automaton from *file*.

**-e** *pattern*, **--expression** *pattern*
: Specify a *pattern* by a timed regular expression.

# EXIT STATUS

0
: if there is no error.

1
: if a error on options has happened.

2
: if a parse error has happened.

# EXAMPLE

The following is an example to monitor a log in **data.txt** over a timed automata in **pattern.dot**.

`monaa -f pattern.dot -i data.txt`

By default, monaa reads the log from **stdin**. Thus, the following is an example to monitor a log from **stdin**.

`monaa -f pattern.dot`

The following is an example to monitor a log over the timed regular expression `(ab)%(2,10)`.

`monaa -e '(ab)%(2,10)'`
