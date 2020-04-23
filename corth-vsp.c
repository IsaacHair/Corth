#include <stdio.h>
#include <stdlib.h>

//normal line indicators; all arguments are expressions; use as flag bits
#define IF      1       //bit indicating whether this is an "if" line
#define ELSE    2       //bit indicating "else" line
#define ANIO    4       //use address not using i/o (1 = true; 0 = reverse)
#define ASN     8       //bit indicating whether assignment occurs
#define ARGS0   16      //bits at 32 and 16 indicate the # of arguments (0 to 2)
#define ARGS1   32
#define NORM    64      //bit indicating if this is a normal line (1 = normal)
//special line indicators; use as the entire number
#define TERM	0       //the program is done
#define SET	1       //set label
#define GOTO	2
#define INS	3	//insert macro
#define SETA	4	//label with an array
#define GOTOA	5	//goto with an array
#define INSA	6	//insert with an array
#define FOR	7
#define BEGCOM  8
#define ENDCOM  9
#define INT     10
#define MACRO   11

//ram and initializations
struct m {
  char* name;
  unsigned long size;
  unsigned long* sidx;
} *macro = NULL;
unsigned long mcount = 0;
struct r {
  char* name;
  unsigned long size;
  unsigned long* value;
} *ram = NULL;
unsigned long rcount = 0;
struct l {
  char* name;
  unsigned long gotosize;
  unsigned long* gototidx;
  _Bool set;
  unsigned long settidx;
} *label = NULL;
_Bool comment = 0;
//current line in source file
unsigned long sline = 0;
//cursor location in target file
unsigned long tidx = 0;
//cursor location in source file
unsigned long sidx = 0;
//line length in chars for target file
#define LINESIZE 14

void backline(FILE* sfd) {
  char c;
  printf("backline\n");
  //keep going back until a newline character is identified
  //Testing fseek ensures that, if this is the start of the file,
  //the loop will not continue forever.
  //Note that sidx needs to indicate which character this is at for
  //macro insertion purposes
  for (fseek(sfd, -2, SEEK_CUR), fread(&c, sizeof(char), 1, sfd);
       c != 10 && !fseek(sfd, -2, SEEK_CUR);
       fread(&c, sizeof(char), 1, sfd))
    sidx--;
  if (sidx > 0)
    sidx--;
  else
    sidx = 0;
  //decrease line count
  sline--;
}

int linedepth(char* buff) {
  unsigned long i;
  //just keep seeing how many tabs there are at the start
  for (i = 0; buff[i] == '\t' && buff[i] != '\0'; i++)
    ;
  return i;
}

_Bool find(char* word, char* str, unsigned long place) {
  unsigned long i, j;
  for (i = 0, j = place; word[i] != '\0'; i++, j++)
    if (word[i] != str[j])
      return 0;
  return 1;
}

int typeline(char* buff) {
  printf("typeline\n");
  unsigned long i;
  unsigned long last;
  unsigned long argc;
  int type;
  //if the line is blank except for padding ' 's and '\0'
  if (buff[2] == '\0')
    return TERM;
  //find the length
  for (i = last = 0, argc = 1; buff[i] != '\0'; i++, last++)
    ;
  //set type to term; it should be changed; if not, it is a macro call
  //THIS RELIES ON THE FACT THAT TERM IS ZERO
  //MAYBE THIS IS A BAD IDEA
  //JUST SETTING TO ZERO ACTUALLY
  type = 0;
  //scan for key words and tokens; mixing setting bits and full assignment is ok
  //this is because each line that is written correctly will only use one
  //have to break for comments because they can contain anything
  //if there are components of normal and special lines, there will either
  //be an invalid line type that results in a fatal error or
  //the line type will just be unexpected but still work
  //In this case, if there are too few arguments, a fatal error will be thrown;
  //if there is the correct number or even too many no error will be thown
  //actually maybe there will be; refer to the function that grabs arguments
  for (i = 0; buff[i] != '\0'; i++) {
    if (i <= last)
      if (find(":", buff, i))
	type = SET;
      else if (find(",", buff, i))
	argc++;
      else if (find(";", buff, i))
	argc++;
      else if (find("#", buff, i))
	type = MACRO;
      else if (find("[", buff, i))
	if (type == SET)
	  type = SETA;
	else if (type == GOTO)
	  type = GOTOA;
        //testing for TERM because that is the default for INS
	else if (type == 0)
	  type = INSA;
    if (i+1 <= last)
      if (find("->", buff, i)) {
	argc++;
	type |= NORM;
	type |= ASN;
      }
      else if (find("/*", buff, i)) {
	type = BEGCOM;
	break;
      }
      else if (find("*/", buff, i)) {
	type = ENDCOM;
	break;
      }
    if (i+3 <= last)
      if (find(" if ", buff, i)) {
	type |= NORM;
	type |= IF;
      }
      else if (find(" in ", buff, i)) {
	type |= NORM;
	type &= ~ANIO;
      }
    if (i+4 <= last)
      if (find(" adr ", buff, i)) {
	type |= NORM;
	type |= ANIO;
      }
      else if (find(" out ", buff, i)) {
	type |= NORM;
	type &= ~ANIO;
      }
      else if (find(" for ", buff, i))
	type = FOR;
      else if (find(" int ", buff, i))
	type = INT;
    if (i+5 <= last)
      if (find(" goto ", buff, i))
	type = GOTO;
      else if (find(" else ", buff, i)) {
	type |= NORM;
	type |= ELSE;
      }
  }
  //the only possibility left is a macro call
  if (type == 0)
    type = INS;
  //set the number of arguments if this is a normal line
  if (type&NORM) {
    if (argc&1)
      type |= ARGS0;
    if (argc&2)
      type |= ARGS1;
  }
  return type;
}

void grabline(char** buff, FILE* sfd) {
  int i;
  char c;
  printf("grabline\n");
  //allows end of the file to be identified; fread will return 0
  int notend;
  //skip all the garbage that marks the end of the line (might be >1 characters)
  //handle sidx too
  for (notend = fread(&c, sizeof(char), 1, sfd);
       notend && (c == 10 || c == 13);
       notend = fread(&c, sizeof(char), 1, sfd)) {
    printf("garbage c = %d\n", c);
    if (c == 10) {
      fread(&c, sizeof(char), 1, sfd);
      break;
    }
    sidx++;
  }
  //go back so that the valid character is seen; if !notend, then not needed
  //also means that sidx is in the correct spot
  if (notend)
    fseek(sfd, -1, SEEK_CUR);
  //read line until you reach a terminating character
  //just finding the size of the buffer right now
  //handle sidx
  for (i = 0, notend = fread(&c, sizeof(char), 1, sfd);
       notend && c != 10 && c != 13;
       notend = fread(&c, sizeof(char), 1, sfd)) {
    printf("check c = %d\n", c);
    i++;
    sidx++;
  }
  printf("check i: %d\n", i);
  //allocate buffer
  //realloc prevents huge amounts of ram from being consumed
  //there are 3 extra characters: ' ' at the start, ' ' at the end, then '\0'
  if (*buff == NULL)
    *buff = malloc(sizeof(char)*(i+3));
  else
    *buff = realloc(*buff, sizeof(char)*(i+3));
  //rewind file to re-read line
  //if the loop terminated because it was the end, then there is 1 less rewind
  //only need to increment sidx if not end
  //the rest of the function will just return to this sidx spot so it is good
  if (notend) {
    sidx++;
    fseek(sfd, -(i+1), SEEK_CUR);
  }
  else
    fseek(sfd, -i, SEEK_CUR);
  //actually copy into the buffer
  //starting with i = 1 since this is the index; ' ' will be at [0] always
  for (i = 1, notend = fread(&c, sizeof(char), 1, sfd);
       notend && c != 10 && c != 13;
       notend = fread(&c, sizeof(char), 1, sfd)) {
    printf("copy c = %d\n", c);
    (*buff)[i] = c;
    i++;
  }
  //terminating character and padding ' '
  (*buff)[0] = ' ';
  (*buff)[i] = ' ';
  (*buff)[i+1] = '\0';
  printf("i = %d; got line:%s\n", i, *buff);
  //increase line count
  sline++;
  //if the line is blank, grab a non-blank one
  //returning a blank line indicates the end of the file
  if (notend && i == 1)
    grabline(buff, sfd);
  //This function will leave the file cursor about to read the character
  //just after the f i r s t line terminating character.
}

void initfor(char* buff) {}

_Bool testfor(char* buff) {
  return 0;
}

void incfor(char* buff) {}

void movecur(FILE* fd, unsigned long i) {}

void usemacro(char* buff, unsigned long i) {}

void setlabel(char* buff, unsigned long i) {}

void setgoto(char* buff, unsigned long i) {}

void initmacro(char* buff) {}

void usegotos() {}

unsigned long expr(char* buff, unsigned long i) {
  return 69;
}

void setnext(FILE* fd, unsigned long i, unsigned long l) {
  int a;
  char c;
  if (!fseek(fd, i+10, SEEK_SET)) {
    printf("error 0x0a\nL%d: unable to access next re-write\n", sline);
    exit(0x0a);
  }
  for (a=1<<12; a > (0 && (c = l/a%16+ (l/a%16 < 10 ? '0' : 'a'-10))); a = a>>4)
    fwrite(&c, sizeof(char), 1, fd);
  if (!fseek(fd, tidx, SEEK_SET)) {
    printf("error 0x0b\nL%d: unable to return to cursor location\n\
this is most likely indicative of a severe compiler defect\n", sline);
    exit(0x0b);
  }
}

void writecomm(FILE* fd, char* opcode, unsigned long select) {
  int a;
  char c;
  for (a=1<<12; a > (0 && (c = tidx/LINESIZE/a%16+
			   (tidx/LINESIZE/a%16 < 10 ? '0' : 'a'-10))); a = a>>4)
    fwrite(&c, sizeof(char), 1, fd);
  fwrite(opcode, sizeof(char), 2, fd);
  for (a=1<<12; a > (0 && (c = select/a%16+
			   (select/a%16 < 10 ? '0' : 'a'-10))); a = a>>4)
    fwrite(&c, sizeof(char), 1, fd);
  tidx += LINESIZE;
  for (a=1<<12; a > (0 && (c = tidx/LINESIZE/a%16+
			   (tidx/LINESIZE/a%16 < 10 ? '0' : 'a'-10))); a = a>>4)
    fwrite(&c, sizeof(char), 1, fd);
}

void initint(char* buff) {
  unsigned long i, j;
  char* tinybuff;
  unsigned long size;
  for (i = 5, tinybuff = NULL; 1;) {
    //scan for start of name
    //note: ' ' is not expected in the name and, if present, will result in
    //the space being removed
    while(buff[i] == ' ')
      i++;
    for (j = 0; buff[i] != '[' && buff[i] != ',' && buff[i] != '\0'; i++, j++)
      //if there is a space not at the very end where ' ' is added
      if (buff[i] == ' ' && buff[i+1] != '\0')
	j--;
      else if (buff[i] == ' ') {
	printf("error 0x04\nL%d: Space within variable name\n", sline);
	exit(0x04);
      }
    //go back to the start of the name
    i -= j;
    if (j == 0) {
      printf("error 0x03\nL%d: Empty declaration\n", sline);
      exit(0x03);
    }
    //actually save the name
    //also increment the number of variables
    //also malloc the first portion
    rcount++;
    if (ram == NULL)
      ram = malloc(sizeof(struct r)*rcount);
    else
      ram = realloc(ram, sizeof(struct r)*rcount);
    ram[rcount-1].name = malloc(sizeof(char)*(j+1));
    //just check for space as an indicator now; it is the char before '\0'
    //just getting the tagline for now; need to check if it is an array
    for (j = 0; buff[i] != '[' && buff[i] != ',' && buff[i] != '\0' &&
	   buff[i] != ' '; j++, i++)
      ram[rcount-1].name[j] = buff[i];
    ram[rcount-1].name[j] = '\0';
    //Array handling; the [] part is not included in the name,
    //it will decide the size of the memory allocation.
    //The size is stored along with the variable name
    //to avoid accidental attempts to read unallocated memory.
    //after the number inside [] is identified and used
    //(w r i t t e n  i n  h e x)
    //i will be incremented until ',' or '\0' is identified.
    if (buff[i] == '[') {
      for (j = 0, i++; buff[i] != ']'; i++, j++)
	if (buff[i] == '\0') {
	  printf("error 0x06\nL%d: incomplete array declaration\n", sline);
	  exit(0x06);
	}
      //allocate memory
      if (tinybuff == NULL)
	tinybuff = malloc(sizeof(char)*(j+1));
      else
	tinybuff = realloc(tinybuff, sizeof(char)*(j+1));
      i -= j;
      //just insert the string now that the size is known
      //the statement is not going to terminate before ']'; that was already
      //checked for
      for (j = 0; buff[i] != ']'; i++, j++)
	tinybuff[j] = buff[i];
      tinybuff[j] = '\0';
      //converting the expression to an unsigned long
      size = expr(tinybuff, 0);
      //saving the value in ram
      ram[rcount-1].size = size;
      ram[rcount-1].value = malloc(sizeof(unsigned long)*size);
    }
    else {
      ram[rcount-1].size = 1;
      ram[rcount-1].value = malloc(sizeof(unsigned long));
    }
    //move past the end of this statement
    i++;
    //line is done
    if (buff[i] == '\0')
      break;
  }
}
    
int insertline(FILE* sfd, FILE* tfd, long depth) {
  int type, nexttype;
  int argc;
  char c;
  int i;
  char* buff;
  unsigned long noop;
  unsigned long iflast;
  unsigned long fbstart;
  unsigned long mbstart;
  while (1) {
    grabline(&buff, sfd);
    type = typeline(buff);
    if (type == BEGCOM) {
      comment = 1;
      continue;
    }
    else if (type == ENDCOM) {
      comment = 0;
      continue;
    }
    if (type == TERM) {
      usegotos();
      return 0;
    }
    if (!comment) {
      if (linedepth(buff) < depth) {
	backline(sfd);
	return 0;
      }
      if (tidx/LINESIZE >= (1<<16)) {
	printf("error 0x07\nL%d: compiled program exceeds maximum size \
(64k instructions)\n", sline);
	exit(0x07);
      }
      if (type == INT)
	initint(buff);
      else if (type == MACRO)
	initmacro(buff);
      else if (type & NORM) {
	argc = 0;
	if (type & ARGS0)
	  argc |= 1;
	if (type & ARGS1)
	  argc |= 2;
	if ((type & ANIO) && (argc == 1)) {
	  writecomm(tfd, "80", ~expr(buff, 0));
	  writecomm(tfd, "a0", expr(buff, 0));
	}
	else if ((type & ANIO) && (argc > 1)) {
	  writecomm(tfd, "80", (~expr(buff, 0))&expr(buff, 1));
	  writecomm(tfd, "a0", (expr(buff, 0))&expr(buff, 1));
	}
	else if (!(type & ANIO) && (argc == 1) && !(type & IF)) {
	  writecomm(tfd, "40", ~expr(buff, 0));
	  writecomm(tfd, "60", expr(buff, 0));
	}
	else if (!(type & ANIO) && (argc > 1) && !(type & IF)) {
	  writecomm(tfd, "40", (~expr(buff, 0))&expr(buff, 1));
	  writecomm(tfd, "60", (expr(buff, 0))&expr(buff, 1));
	}
        if (type & ASN)
	  if (!(type & ANIO)) {
	    printf("error 0x08\nL%d: \
assignment on an output statement\n", sline);
	    exit(0x08);
	  }
	  else if (expr(buff, 2))
	    writecomm(tfd, "e0", 0);
	  else
	    writecomm(tfd, "c0", 0);
	if ((type & ELSE) && !(type & IF)) {
	  insertline(sfd, tfd, depth+1);
	  return 0;
	} 
	else if (type & IF) {
	  if (!(tidx/LINESIZE%2))
	    writecomm(tfd, "40", 0);
	  if (type & ANIO)
	    writecomm(tfd, "20", 0);
	  else if (argc == 1)
	    writecomm(tfd, "00", expr(buff, 0));
	  else if (argc == 0)
	    writecomm(tfd, "00", 65535);
	  else {
	    printf("error 0x09\nL%dtoo many arguments to 'if'\n", sline);
	    exit(0x09);
	  }
	  noop = tidx;
	  writecomm(tfd, "40", 0);
	  insertline(sfd, tfd, depth+1);
	  iflast = tidx-LINESIZE;
	  setnext(tfd, noop, tidx/LINESIZE);
	  grabline(&buff, sfd);
	  nexttype = typeline(buff);
	  backline(sfd);
	  if (nexttype & ELSE) {
	    insertline(sfd, tfd, depth);
	    setnext(tfd, iflast, tidx/LINESIZE);
	  }
	  else
	    setnext(tfd, iflast, tidx/LINESIZE);
	  if (type & ELSE)
	    return 0;
	}
      }
      else if (type == FOR) {
	initfor(buff);
	while (testfor(buff)) {
	  fbstart = sidx;
	  insertline(sfd, tfd, depth+1);
	  incfor(buff);
	  if (testfor(buff))
	    movecur(sfd, fbstart);
	}
      }
      else if (type == INS)
	usemacro(buff, 0);
      else if (type == INSA)
	usemacro(buff, expr(buff, 0));
      else if (type == SET)
	setlabel(buff, 0);
      else if (type == SETA)
	setlabel(buff, expr(buff, 0));
      else if (type == GOTO)
	setgoto(buff, 0);
      else if (type == GOTOA)
	setgoto(buff, expr(buff, 0));
      else {
	printf("error 0x05\nL%d: undefined line type %d\n", sline, type);
	exit(0x05);
      }
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("error 0x01\nusage: cvm <source file> <target file>\n");
    exit(0x01);
  }
  FILE* sfd = fopen(argv[1], "rb");
  if (sfd == NULL) {
    printf("error 0x02\nsource file not found\n");
    exit(0x02);
  }
  FILE* tfd = fopen(argv[2], "wb");
  printf("hola0\n");
  insertline(sfd, tfd, 0);
  fclose(sfd);
  fclose(tfd);
  return 0;
}
