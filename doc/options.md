List of the Options
===================

List of the major options is as follows.

<table>
<tr><th>Short Option</th><th>Long Option</th><th>Description</th></tr>
<tr><td>-i</td><td>--input</td><td>Specify the input file of the timed word. If this option is not used, the timed word is read from stdin.</td></tr>
<tr><td>-f</td><td>--automaton</td><td>Specify the input file of the timed automaton. Exactly one of this option or the '-e' must be given.</td></tr>
<tr><td>-e</td><td>--expression</td><td>Specify the timed regular expression. Exactly one of this option or the '-f' must be given.</td></tr>
<tr><td>-h</td><td>--help</td><td>Show the help message</td></tr>
<tr><td>-q</td><td>--quiet</td><td>Enable the quiet mode. It suppresses most of the messages.</td></tr>
<tr><td>-V</td><td>--version</td><td>Show the version of the MONAA</td></tr>
</table>

The following table shows the experimental options. In a usual usage, you can ignore them.

<table>
<tr><td>-a</td><td>--ascii</td><td>Use the ASCII mode [default]</td></tr>
<tr><td>-b</td><td>--binary</td><td>Use the binary mode (experimental) </td></tr>
<tr><td>-E</td><td>--event</td><td>Interpret the input timed word as a sequence of the events [default]</td></tr>
<tr><td>-S</td><td>--signal</td><td>Interpret the input timed word as a signal (experimental)</td></tr>
</table>
