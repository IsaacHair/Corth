Version 1:

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





Version 2:

First of all, there needs to be a command to read the input,
which was completely missing from the syntax before.
Just like before, whitespace doesn't matter except for tab
deliniation of nested statements and newlines after each statement.
To allow for statements to take up multiple lines, a backslash can
be placed directly before a new line to tell the compiler to ignore
that line break.
The compiling process still starts by deleting all comments.
Along with comments, '\' is removed and the newline after it is
replaced with a space.
Then, any key words are replaced with their corresponding tokens.
This includes replacing "=" and "==" and "," and "\n" etc
with their tokens.
During this process, if something is not a token, triangular braces
are put around it. A token indicating its type is placed directly
before it.
Parenthesis around expressions are kept.
Tabs are special in that they are replaced by curly braces denoting
where blocks of code begin and end.
Square braces, which indicate concatenation of a variable to a label,
are kept as they appear.
Basically this process will be tuned until everything can be neatly
converted into token form.
A lot of this process involves pre-evaluation of some statements,
like labels, so that tokens can appear in a neat and easy to decode
order.
This is going to take some thought and experimentation but should produce
a modified version of the code that can be manipulated easily during the
rest of the compiling process.
The max length for labels, macros, and variables is 16 characters. However,
if there is a [, this length can be infinitely long, only being limited
to 16 characters after the ].
However, brackets must contain an expression.
Note: to avoid ambiguity, numbers are always written as hex numbers, with
preceding zeros optional. Max length for a number is 16 digits.
To avoid ambiguity between numbers and variables/labels/macros
(eg number "aa" vs variable "aa"), if a number starts with a letter
(a-f), a zero must be placed in front.
The preceding zeros are deleted automatically during the compiling process,
so you don't need to worry whether path[x] will be called path[0a] or
path[a] because all preceding zeros will be deleted before macro/label/
variable calls occur.
Also going to read as a text file while deleting comments, then treat
as binary after that.
Important: newlines are represented by a carriage return and then a line
feed character in text files; to handle the files properly, it is best
to replace this with a one character identification.
Arrays can be declared; declaring an array simply involves placing brackets
after the label and writing the size; there can be as many dimensions
as you want.
The compiling process is going to require a recursive section for the
[]s that can be added to labels and variables.
Also, at this point, screw it, might as well add support for local variables
and arguments passed to macros.
Looks like recursively inserting macros/for loop bodies/values within []
might not actually be powerful enough to resolve all cases of nesting.
If the user states something like

for (;myarray[table[x][y]+5a]; x = x+1)
    myarray[table[x][y]+5a] = myarray[table[x][y]+4b]

The for loop can't expand.
BUT OH OK SO ITS FINE; you can't assign anything unless it is in a for loop.
Wait what if you do

int myarray[4aaa], table[10][10]
for (;myarray[table[x][y]+5a]; x = x+1)
    for (myarray[table[x][y]+5a] = myarray[table[x][y]+4b]; 0;)
    	out(0000, 0000)

Looks like what will need to happen is macros are pasted in as normal using
recursion.
Then, for loops are also evaluated recursively. Every time you reach a name
containing [], so an array or a label or a goto, recursively collapse it
and then return to handling the for loop based on that collapsed version.
This works because [] statements never contain for loops or macro calls.
For loops can contain macro calls and macros can contain for loops, however,
so a different process is needed for them if things like this aren't banned

int x
for (x = 0; 0;)
a

#a
    for (;x;)
    	a

In theory, since x is set to zero, this should never collapse into an infinite
loop.
However, blindly collapsing will cause this.
Ok so maybe the protocol should be to start at the outer most nest level.
If you get to a macro call, insert it.
If you get to a [] expression, collapse it.
If you get to a for loop, evaluate it.
Inside the for loop, repeat the same process, and once you reach the end,
return to the start of the for loop and check again.
This allows statements like the one above to work.

OH so by the way the second statement of the for loop, the conditional
statement, is just looking for a non-zero value, and using operands like
&& convert the values on either side to either 1 or 0 and deal with them.
So x+value increments x by value.
But x+(value&&othervalue) either increments x by 1 or nothing.
Random note: if no goto statement appears at the end of the code, the next
address is assumed to be 0000.

So currently the finished version is going to be very bare bones.
There will be no support for local variables/labels/macros.
There will be no if-else statements for perprocessor variables.
However:
There will be arrays.
There will be assignment of preprocessor variables within loops, not just
in the body of for loops.
Note that names can contain up to 16 characters. However, there can be
infinitely many dimensions on an array or a label, but these must be marked
with square brackets. Also, the name can't continue after the label.
So, as[11]ds would be bad syntax.
All values, whether pointers or constants or integers or anything, are stored
as unsigned 8 byte numbers. During the last step of conversion to assembly,
any numbers longer than two bytes are chopped off and only the last two bytes
are used.

For insertion and expression evaluation:
 General outline:
    - insertevaluate() buffers one line at a time (with malloc) and sends them
    to useline(char* buff) to actually process the line. After buffering
    the line, the cursor is left directly after the END token.
    
    In useline(char* buff):
    - The line type is determined.
    - A function is called to evaluate the line.
    - If the line is a for loop, then it is sent to forinit(char* buff),
    then sent to foruse(char* buff, FILE* sfd, FILE* tfd) until a value
    of 0 is returned to indicate that the copying procedure is done. Note
    that foruse() will fseek() back to the s t a r t of the line it was fed
    once done evaluating the for loop. If the condition is false, it will
    move the cursor directly a f t e r the } token marking the end of t h i s
    for loop, then return 0. In the foruse() function, it will again buffer
    each line and process it using useline(). Note that the foruse() function
    creates buffb for each line it reads, preserving the buff that it
    was passed as an argument. In forinit() and foruse(), expressions are
    identified and passed to evalexpr(char* buffc); again, the original
    buff that was passed and buffb are preserved.
    - If the line is a macro call, the macro is simply transferred from ram to
    buffd using malloc() as usual, and each line is fed to useline() until the
    macro is done. The index of the macro in the macro[i] struct is passed
    to macrouse(unsigned long int macro).
    - If the line is neither a macro call or a for loop header, then the line
    is passed to writeline(char* buff, FILE* tfd).
    - In writeline(), the line is copied character by character. If a NAME or
    NUM string is written, the function grabs whatever expression is there and
    passes it to evalexpr() w i t h o u t writing the line in the target file.
    Instead, it writes the return value of evalexpr() if the expression is
    within a statement containing a token that actually compiles to machine
    code, eg "if" or "adr", and not just a pre-processor thing. If the line
    will end up being blank, nothing is written to the target file (eg the
    line was just to re-assign a new value to a pre-processor variable).
    - In evalexpr(), the expression is evaluated. First, the function scans for
    any assignment or comparison operators, then places parenthesis around the
    expressions on each side. Parenthesis are also placed around comparison
    operators around assignment. Note that commas can be placed between
    statements. Then, the deepest parenthesis level is identified,
    and expressions at this level are buffered using malloc one by one, then
    passed to microexpr(char* buffe) to be evaluated from left to right. This
    function returns the value of the expression, and then evalexpr() inserts
    this in place of the expression in parenthesis. Once the only operators
    left are assignment operators, the evalexpr() function will evaluate
    these from right to left and set the variables to what they should be.
    Regardless of how many assignments took place (zero to infinity), the
    expression on the rightmost side of all assignment operators is the return
    value of evalexpr(). If the expression had no assignments, this would
    simply be the value of the expression.
    - microexpr() simply evaluates from left to right. The value of comparison
    operators is either zero or one. The rest of the operators simply do what
    their names suggest; ADD would just add two numbers.

So basically it makes more sense to just grab expressions where a string or
operator starts. The missing start parenthesis can be inferred based on the
parenthesis within the expression. So, all expressions start with a NAME,
NUM, or the NOT operator (as this is the only operator that does not take two
inputs) and end with SEP, FEND, END, or TERM. The mismatched
parenthesis left in the program are simply discarded, and the mismatched
parenthesis within expressions still provide enough information, as explained
above. Also, now, statements separated by SEP in for loops are sent one at
a time to evalexpr(). Note that this is the only place where comma (SEP)
separation is grammatically correct.

    - For all of the functions which grab a line and then set the cursor
    at the start of the next line, grabline(FILE* sfd) is used.

NOTE: THE CODE FOR UPLOADING AND COMPILING VSP ASSEMBLY NEEDS TO BE
CHANGED TO USE BINARY FILES INSTEAD OF TEXT FOR RELIABILITY.

This whole thing is a bit of a mess. It might be a good idea to get a solid
outline for what is expected first and break this into manageable chunks.
The expression identification currently sucks and dealing with mismatched
parenthesis and having to re-write the identification process over and over
again for each function and mixing ram and files to buffer data is hard
to deal with.




Version 3:

- Changing file extension to .cv
- Syntax is updated, grammar is more explicit, everything is slightly
  more limited so that a working version of this compiler can be created.
- More complex versions will come later.

- syntax:

keywords

for
:
goto
if
else
adr
out
in
<-
int
,
;
'\t'
'\n'
#
[
]

- for expr0, expr1, expr2 ... exprx; conditionexpr; expr0, expr1 ... exprx
- label:
- goto label
- if adr optional, optional <- optional
- if in optional
- else
- else if adr optional, optional <- optional
- else if in optional
- adr optional, optional <- optional
- out optional, optional
- int var0, var1[] ... varx
- #macro

- expressions:

pre-evaluation
      <variable name>
      [
      ]
      <number written in hex, 1 - 16 digits>
      (
      )

first priority
      /
      %
      *
      +
      -
      &
      |
      ^
      !
      
secod priority
      <
      >
      <=
      >=
      ==
      !=
      &&
      ^^
      ||

third priority
      =

- Expressions are expected directly after any statements where they should
  appear. This is the only location where expressions should appear.
  An expression somewhere else results in a syntax error. This includes
  assignments. Expressions are passed to expr(char** buff), which converts
  the buffer to the desired final value.
- expr(): First, the expression is isolated and inserted
  into a buffer string in ram. Then, the deepest level of [] is identified,
  then the deepest level of () within that. The string of data here is
  buffered and sent to microexpr() to be evaluated and have its value pasted
  back into the big buffer in place of the string. Then the next string at
  that level is grabbed and used; if there are no strings at this level, then
  back up a level with (); if () is already at level 0, back up a level at []
  and look for the deepest (). Repeat this process untill [] and () are both
  at level zero and the value returned from microexpr() has been pasted. At
  this point, the buffer contains the desired value, so the function is done.
  Note that this means that a pointer to the buffer is passed, so it will be
  a char**, and everything is done in hex lower case as usual.
- microexpr(): Identify first priority operands and buffer the values on
  either side, then evaluate those values with that operand, then insert
  the value in place of what used to be there. If '!' is identified, then
  only the value to its immediate right is used. When handling values, the
  value is first checked to see if it is a member of the variable name list.
  If so, the value of the variable is inserted. If [] directly followed the
  name, then the hex string inside is converted to a uint64_t and then indexed
  to the corresponding value of the array to retrieve the value. Do this
  will all first priority operands, simply moving from left to right.
  Once all first priority operands are gone, switch to second priority with
  the same general scheme, going left to right
  Once those are all gone, switch to third priority. These operands go from
  right to left, so simply evaluate backwards. The assignment operator does
  two things: Obviously, it assigns the value of the right hand side to the
  left hand side. Second, though, it collapses to become the value on the right
  side.
  Once no operands remain, one last pass occurs to conver everything to numbers.
  Then, the function ends because its buffer is now the final hex string value.


Actual compiling workflow:
- Scan through the file and identify key words. Buffer the file into global
  variable: (use malloc/free)

  struct l {
  	 int type;
	 long depth;
	 char** arg;
  } *line;

  Yes, there are 3 levels of pointers here, but there is good reason. The
  pointer on line allows each line to be buffered. The double pointer
  on arg allows arg to point to a list of strings, each string being an
  argument. FOR is the only dynamic size operator, so after each section,
  ';' is placed as an operator to indicate the end of that section, then
  you move on to the next operator. Each string is terminated with '\0' as
  is normal behavior. The depth is the number of tabs at the beginning of the
  line in the source file. Lines in the source file are terminated with a
  carriage return or a newline character or ';', whichever comes first,
  and the next line only starts with a character other than those.
  Note that "int x;\t\nblah...." would result in a line containing '\t' only.
  Blank lines with no discernable type are discarded and not buffered into
  line. Note that a terminating line just containing TERM is placed at the end.
  Comments are also identified and skipped during this stage.
  During this process, int and # are also not converted to lines; instead, they
  are buffered immediately into: (use malloc/free)

  struct r {
  	 uint64_t* value;
	 char* label;
  } *ram;

  and

  struct m {
  	 char* label;
	 struct l *line;
  } *macro;

  Essentially, macro defines a collection of mini-programs with a label and
  a set of lines like the larger program. The reasoning behind the long* in
  ram is so that arrays can be declared. When variables are declared, they are
  of the type:

  int myvariable0

  or

  int myvariable0[100]

  Variables can be labeled as any combonation of lower case letters and numbers.
  However, it is smart to include a letter besides a-f because, when expressions
  are evaluated, the variable called "800" will replace the number "800" and you
  are kinda screwed. Also note that arrays can be declared, but they are only
  1 dimensional. You can use 2 or more dimensional arrays, it is just up to
  the programmer to multiply out the index values and then place them within
  the 1 dimensional array. Note that only one pair of [] can follow a variable
  name, and nothing can come after this. However, there is no limit to variable
  name length, but as far as numbers go, everything is converted to uint64_t.
  Also, all numbers are always written in lower case hex. For example, the
  declaration above for "myvariable0[100]" declares two hundred and fifty six
  uint64_t's, not one hundred, because 100 is in hex.

  Types are as follows: (#s in decimal, not hex)

TERM	0      //the program is done
LABEL	1
GOTO	2
INS	3	//insert macro
II0	4	//if in, 0 arguments
II1	5	//if in, mask
IA0	6	//if address, 0 arguments
IA1	7	//if address, value to set address to only
IA2	8	//if address, value to set address to and mask
IA0A	9	//previous def + assignment
IA1A	10	//previous def + assignment
IA2A	11	//previous def + assignment
E	12	//else
EII0	13	//else + previous def (this is the same for the rest)
EII1	14
EIA0	15
EIA1	16
EIA2	17
EIA0A	18
EIA1A	19
EIA2A	20
A1	21
A2	22
A0A	23
A1A	24
A2A	25
O1	26
O2	27
LABELA	28	//label with an array
GOTOA	29	//goto with an array
INSA	30	//insert with an array
FOR	31

  Note that A0 is ommitted; doing nothing with the address is not an
  instruction. The only time "adr" with no arguments is typed is in
  conjuction with an if statement or if else statement. Similar logic for
  ommitting plain input and O0. However, A0A is kept because they
  are doing something: assignment.

  Also, with labels, there can also be one dimensional arrays. Again, these
  can be adapted for 2+ dimensions. This extends to macros, too.

  Every argument boils down to a numeric expression except for:
  - terminating ';'s within FOR
  - the name of a label within GOTO/GOTOA
  - the name of a label within LABEL/LABELA
  - the name of a label within INS/INSA
  The first situation, with FOR, doesn't require fancy handling because
  the ';' is just an indicator. For the last three, though, each one contains
  a text/number string which is preserved in the zeroth argument slot, and
  if the optional array is used, then the first argument slot contains the
  index of this array which is an expression.


  Anyways, once everything is buffered, the second pass involves copying the
  *line to *line2 which is of the same type and uses the malloc/free process
  as well. During copying, everything is preserved, except for the fact that
  expressions are evaluated as they are copied. FOR lines are not copied;
  instead, they repeatedly copy the body of the FOR and evaluate the FOR
  argument expressions along the way (and condition), which is everything after
  the FOR line that has a higher depth than the FOR until something with the
  same depth or lower is encountered. INS lines are also not copied; instead,
  the body of the macro is copied following the above procedure (essentially,
  just switch from reading line to macro[mac].line). This procedure
  terminates with the copying of the empty line with just TERM as type. If
  this was acting on a macro, copying returns outside the macro procedure.
  Otherwise, if this was for the whole program, copying is now done. Note that
  during macro copying there is a correction factor for the tab depth
  that accumulates as more macros are called within one another.


Better storage method:

  struct l {
  	 int type;
	 char** arg;
	 struct l* content;
  }* line;

Follow the procedure above, but instead of needing tab depth for the lines,
increasing tab depth indicates creating a new content structure. This can be
passed as an argument recursively, allowing nesting. If tab depth decreases,
just add a term character and then return to the parent function, which contains
the pointer to the outside structure. This allows for loops as well as
if and else to be contained in a very smooth way. When reading, these types
indicate an increase in depth; otherwise, depth is not increased. A TERM
instruction indicates a decrease in depth or the end if the function evaluating
this is the parent function. Still store macros and variables seperately,
because this is still more natural.

During for loop insertion, the members of the inside "content" struct are
passed without increasing depth on the struct being written to. However,
depth for if and else is still increased.

IMPORTANT COMMENTS: First of all, remove the idea of terminating lines with ';'
as this is difficult and there is already enough operator overloading going
on. Second, this is the workflow:

insertline(struct l* point, int* i, FILE* sfd) is called from main()
with i as zero and "line" as the pointer for the line structure. This function
calls grabline(char** buff, FILE* sfd), which buffers the next line.
linetype(char* buff, _Bool* comment), which returns type and also allows for
continued comment recognition across all lines, is called next. Comments result
in no nesting at all, so comment does not need to be passed to insertline().
Anyways, insertline() then writes the line. If a depth increase is needed,
insertline() will call itself with pointer "point.content" and the process
will repeat. malloc() is called each time a line is added, and, at the end,
free() will need to be called to actually read the whole thing and free the
structure. If a depth decrease is needed, then a TERM line is added, then
that instance of insertline() ends. If the depth is remaining the same or the
line is a TERM line, then the pointer it contains is set to NULL. If the source
file ends, this also indicates a depth decrease, which results in the original
insertline() returning to main(). If the depth remains the same, insertline()
will call itself, with i incrementedWRONG! NOTE:this uses curried functions wait
maybe not ok this structure is gonna be weird.

sooooo......

insertline() is called. It repeatedly grabs lines and, well, inserts them
into the line structure. Pointers here are also marked as null if they are
unused. If there is a depth increase, it calls a new instance of itself with
the more nested pointer. Important: it remains in the loop of grabbing lines,
so when the function handling the indented lines returns, the function that
called it can resume reading lines. It only returns if the line depth decreses.
If there is a macro definition, it will simply handle the
macro and then goes back to insertline() with a pointer to macro[x].line
instead of line.

Anyways you have:

void insertline(struct l* point, _Bool* comment, FILE* sfd)
void grabline(buff**, FILE* sfd)
int typeline(buff*)


Now, malloc'ed memory is not going to be freed at the end of the program
because this will already occur automatically. Files will still be closed,
though.

The actual program also has error and line counters, etc. See the actual
program for the details; this readme is just an outline.
Also, the syntax for comments requires the code to begin on the line after
the comment is terminated. There are other minor updates too, like the ->
operator.

Important: invisible tabs are NOT deleted

ok wait...

first of all, remember that the definitions are completely changed.

and back to what i was saying...

The program will return an error if there are invisible tabs or if the start
of a comment isn't at the beginning of a line and the end isn't at the end
of the line or the comment begins and ends on the same line.
The only comment method is /*...\n...*/

Also now using exit() to close the program immediately upon an error instead
of constantly testing for error. This also makes printing strings for the
errors easier.

Expressions can be used within variable declarations, but, if that expression
contains any other variables, they must have already been declared.
For all other expressions, the variable does not have to be declared already.
For macros, the macro can be declared anywhere, but a macro cannot contain
a definition for another macro.

********

Screw it, the compilation process will work best in only one pass. Basically,
all variables must be declared before use. All macros must be declared before
use. Also, labels and gotos will work in a somewhat interesting way. If a label
appears before a goto calling that label is ever stated, then the label's
location is simply saved and the goto(s) can jump there, nothing too difficult.
However, if the label appears after its corresponding goto(s), then each
goto statement simply does not jump anywhere. The goto next address is filled
in with 0000 or something. Anyways, the cursor location of that 0000 is saved
for each goto. Once the label is identified, then the cursor location is saved,
and the program goes back to each cursor location for those gotos and inserts
the program address of the label, then jumps back to the label.
Basically, this is how the program address counter works:
It starts at 0000 and increments by one for each instruction.
If there is an if-else statement or a chain of if, else-if, else-if ...
then each line containing an if will jump to two locations, both as close
as possible to the end of the statement and obviously one being an odd address
and the other being an even address. The odd address will just contain the
body of the if statement. The even address, which should be right before the odd
address, will be left and have its cursor location saved until the else
statement is reached, at this point, it will contain the body of that statement.
If it is actually an else-if, this is fine; the body of the else statement is
just another "if" and the "if" protocol is just repeated.
Hold on... in a way, the file doesn't need to be loaded into ram at all at this
point, it will just be directly processed into another file.





Version 4 (java):

The overall model is to buffer the source file line by line as a set of
objects. Then, these objects are used to construct a set of line objects for
the target file. This is then copied into the actual target file.

Source line objects:

class sourceline {
	public void sourceline(String line) {
		/*blah blah init that shit hard*/
	}
	public boolean type(String query) {
		/*return the value of the respective function
		for that query (ie. if, else, macro, ...)*/
	}
	private boolean typeif() {
		
	}
	private boolean typeelse() {
		
	}
	private boolean typeasn() {
		
	}
	private boolean typemacro() {
		
	}
	private boolean typefor() {
		
	}
	private boolean typelabel() {
		
	}
	private boolean typegoto() {
		
	}
	private boolean typeadr() {
		
	}
	private boolean typeout() {
		
	}
	private boolean typein() {
		
	}
	public int args() {
		/*return the number of arguments in the statement (this
		excludes the assignment argument if present)*/
	}
	public String grabarg(int idx) {
		/*return the idx number argument as a String*/
	}
}
class expr {
	public void expr() {
		/*init that shit boi*/
	}
	public unsigned long val() {
		/*return value*/
	}
}
/*
 * Defines a stack for variables to be stored on when functions are called.
 * Need to define a method for writing nested if-elses in a macro.
 * This language is probably going to need some major expansion.
 * It looks like this will need:
 *   Arrays
 *   "For" loops which define repeated insertion of their content.
 *   Macros
 *   If/Else, I/O, and Ram manipulation instructions
 *   Allocation/Deallocation of local and global variables.
 * Actually, now that I think about it, the current language supports
 * everything that I am looking for.
 * This architecture should not only allow for the creation of functions
 * but also the creation of objects.
 * The programmer just has to define everything from scratch.
 * Nested return will require writing every function call before defining
 * the function.
 * Each call must allocate another 2 bytes on the allAddr array for that
 * function.
 * Then, this array can be used to build the return nest.
 * Note that the value of allAddr is the address of the first byte.
 */

//macro to define nested return array using the sub-function ID on the stack
#nestedReturn(allAddr, length)
	int flag
	for (flag = 1; length && flag; flag = 0)
		if adr fff8
			nestedReturnSub(allAddr+1)
		else
			nestedReturnSub(allAddr)

length of 0005 should produce:

if adr 0000
	if adr 0001
		goto allAddr+3
	else
		goto allAddr+1
else
	if adr 0001
		goto allAddr+2
	else
		if adr 0002
			goto allAddr+4
		else
			goto allAddr+0

for a least significant to most significant architecture

QUICK QUESTION: how the fuck is memory management going to work
is this shit pass by reference or by value?
are there going to be local variables?
how are arrays handled?

OK NEW PLAN:

******V5********

going to create a very simple language:
"for" loops
macros
gotos
global variables
if statement

Variables must be allocated at the beginning of the program.
Variables can be accessed by using their name.
For arrays, simply use the name of the variable and then add [].
If no [] is added to the variable, the default value is 0.
Note that technically variables that are not arrays can still have []
because they can be considered arrays with one element.

NOTE: in this language, there is no if-else structure; you have to
create that using if statements and gotos.

Variables and lables can be used as one dimensional arrays, but macros cannot.
