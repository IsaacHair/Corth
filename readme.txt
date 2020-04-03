Need buffers for comments and malloc, macros, other macros, for loop,
other for loop, evaluate, other evaluate, translation, and gotos.
Remember to free memory at end. Possibly condense the huge number
of buffer files into just two files that go back and forth, (or
one file that alternates with the target file), but less
useful for diagnostics, so do this later. Also need source file and target.
Going to just have buffer and target files written to. To test various stages,
just comment out the rest of the code.

The compiler commands are (), int, #, [], and for. # tells the compiler to declare
a macro for compile time use only. () indicates a macro call. int tells
the compiler to declare a variable for compile time use only. for describes a variable
manipulation and incrementation scheme. Understand that this is not actually going to
create a for loop in the resulting program. Rather, it creates a loop with
variables that can be used to write code based on those variables.
All variables declared inside a macro are global, same with for loops.
All variables declared outside anything are global. for can also be used as an if
statement with flag. Lables work the same way as variables in terms of global vs
non-global. Note that only vsb code written directly or in a macro call from direct code
will be compiled. Everything is global. Note that () is not needed to call a macro.
All you need is just the name of the macro.

The commands that actually produce vsb code are if, else, adr, :, =, goto, and out.
adr or out statement required inside an if statement. Optional assignment with adr.
Out takes zero to two parameters, adr can take between zero and two; first is
the value to assign it to and the second is the mask for assignment (1 = assign,
0 = protect). : is placed after a lable to use with goto. A space can be put after
the label and before the colon. Always global. Need to ensure that if-else statements can
be used without needing to increase the indent level every time.

Macros themselves are NEVER actually compiled into vsp assembly. Instead, they need to
be called from within the program.

Spaces after each command or none, then parenthesis. Semicolons only inside for loops. Commas
for multiple arguments. Tab deliniation for loops, not brackets. Note that parenthesis
are optional but may be needed to ensure the correct grouping. Spacing can be arbitrary,
just ensure that tab deliniation lines up. Square brackets need to be used if combining a
compiler variable with a label. If a compiler variable is used elsewhere, just treat it
like a normal number. Order of operations is strictly left to right unless there are
parenthesis. *Exception: the assignment operator '=' is always the lowest priority.

In a way, for is a special kind of macro. It defines manipulation of variables and allows
looping until something occurs. When it loops, it prints the body of the for loop to the
compiled vsp file. Note that this compiles to vsp assembly, requiring a bit extra processing
to compile to vsp upload code (maybe just combine the two processes into one though). Note
that, since for loops are a type of macro, they can have local labels declared within them.

Don't need goto's for pre-compiled code because these kinds of things should be incorporated
into the vsp code itself using vsp if-else, goto, etc.

Max macro name length: 16 characters, same as max variable name length.

Compiling Process:
- Delete comments.
- Convert everything to tokens and delete extra space and parenthesis.
- Scan for global variables and malloc() them to 8 byte integers.
- Scan for macros and insert; repeat until no macros are left (except for "for" loops).
- Scan again and, if reach a for loop, interpret each statement and
buffer them. Then, repeat the contents of the for loop as if it was a macro,
using the rules from before and incrementing or manipulating
variables as specified until reach point where condition is false, then
move on. If condition is false to begin with, then don't print contents at all.
*The other action that occurs simutaneously is replacing variables
with their actual values. Ex: (1<<(x+2)) becomes (1<<(4+2)) if x happens to
be 4 at that time. *Like for macros, loop these until none left. Evaluate innermost
for loop first, then work your way out.
- Scan again and actually evaluate the expressions. (1<<(4+2)) becomes 0040.
Note that everthing is written in hex as an int, and everything is stored that way.
So, even though (1<<31)/(1<<30) should equal 2, it will actually result in overflows
and be weird. ACTUALLY MAKING DATA TYPES ALL 8 BYTES. Allows for convenience.
Note that the leading zeros can be eliminated if you want. *Like before, do
this recursively, starting with the most nested set of parenthesis and then working your
way out. Repeat until no parentheses remain at all. *Technically, none are needed, even
for calling macros and for adr, out, if, goto, else, and :.
- Scan again and simply translate the labels, gotos, if-else statements, adr(), and
out() statements into vsb assembly. Lables simply add a no-op and then store the
location in an array. If variables are in brackets appended to the label, their value
is printed, ommitting preceding zeros & keeping brackets. Gotos add a no-op that goes to the label
spot. If there are brackets, the value is inserted, ommitting preceding zeros and keeping brakcets.
if and else result in either a jmpr or jmpi depending on the contents of the if(), then an even
address for the else statement from that and an odd address for the contents of the
if statement from that. adr() and out() just result in a couple adr1/adr0 and out1/out0
lines to set the address/output to that with a mask. If the value written is ffff or
0000 then only one line of vsb assembly is needed. An adr() statement can be followed by an =
and then a number. If the number is zero, buf0 is written. If the value is non-zero, buf1
is written. *During this last scan, the compiler will automatically count the address of the
instruction, starting with zero. It will just add one for each instruction. If-else will be
handled, as stated before, so that the "if" condition has an odd address and the "else" condition
is the even counterpart to that address. *See where labels end up and write everything except
for gotos.
- Scan and add in the addresses for gotos.
- Free variables. Using malloc so that you can have as many variables as there is ram
available on the computer. Note: max name length is 16 characters, all lower case. Stored
in a 17 character array because need terminating '\0', just easier to add extra for full length
terminating.

Consider this; how would this be compiled? Technically, it should be non-fininite size:

int (f, i)
for (f = 0; 0;)
a
end:
goto end

#a
out(1000, ffff)
for (i = 1; f&&i; i = 0)
a
for (i = 1; (!f)&&i; i = 0)
out(0001, ffff)

Should yeild:

0000 out0 ffff 0001
0001 out1 1000 0002
0002 out0 ffff 0003
0003 out1 0001 0004
0004 out0 0000 0004

Ok so this shouldn't be necessary when actually writing programs. You can simply avoid infinite
loops within the macros, meaning that simply inserting macros to where they are called one by one
will suffice for programming purposes.
