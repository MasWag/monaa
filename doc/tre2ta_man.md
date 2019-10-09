# TRE2TA Usage Manual

## Name

tre2ta - Translate a timed regular expression to a timed automaton

## Synopsis

    tre2ta [OPTIONS] PATTERN
    tre2ta [OPTIONS] -e PATTERN

## Description

**tre2ta** is a tool to translate a timed regular expression to a timed automaton. For both of them, the syntax is the same as that of MONAA.

## Options

**-h**, **--help**
: Print a help message.

**-V**, **--version**
: Print the version

**-e** *pattern*, **--expression** *pattern*
: Specify a *pattern* by a timed regular expression.

## Exit Status

0
: if there is no error.

1
: if an error on options has happened.

2
: if a parse error has happened.

## Example

The following is an example to translate the TRE `(ab)%(2,10)` to a TA. The result is written to the standard output.

`tre2ta -e '(ab)%(2,10)'`

You can omit `-e` as follows.

`tre2ta '(ab)%(2,10)'`
