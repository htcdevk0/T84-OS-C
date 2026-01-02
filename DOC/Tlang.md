TLANG - BASIC LANGUAGE NOTES

This document is a simple reference for developers who want to fork or work
with TLANG. It describes the basic syntax and core features of the language.

--------------------------------------------------

VARIABLE DECLARATIONS

1) int
Integer variable.

Syntax:
int <name> = <value>

Example:
int x = 10


2) schar
String variable.

Syntax:
schar <name> = "<value>"

Example:
schar text = "Hello"


3) bool
Boolean variable.
Values can be true or false.
Internally handled as 0 (false) or 1 (true).

Syntax:
bool <name> = <value>

Valid values:
true
false

Example:
bool flag = true

--------------------------------------------------

OUTPUT

write()

Prints values or text to the output.

Accepted arguments:
- int
- schar
- bool
- raw text
- simple integer expressions

Rules:
write("ABC", x + y)

--------------------------------------------------

CONTROL FLOW

if statement

Syntax:
if <var> < < / > / != / == / >= / <= > <condition>:
    result...


elif statement

Same syntax and rules as if.

else statement

Executed when all previous conditions are false.

--------------------------------------------------

INPUT

input()

Gets user input and stores it in a variable.

Syntax:
input(<var>)

--------------------------------------------------

LOOPS

for loop

Syntax:
for <var> <name> = <value> ; <condition> ; <condition2>:
    final...

Common usage:
<var>++

--------------------------------------------------

IDE

4IDE
