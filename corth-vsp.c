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
struct l {
  int type;
  unsigned long location;
  char** arg;
  struct l* line;
} *line = NULL;
struct m {
  char* name;
  struct l* line;
} *macro = NULL;
unsigned long mcount = 0;
struct r {
  char* name;
  unsigned long size;
  unsigned long* value;
} *ram = NULL;
unsigned long rcount = 0;
struct b {
  char* name;
  unsigned long location;
} *label = NULL;
unsigned long lcount = 0;
_Bool comment = 0;
unsigned long location = 0;

void backline(FILE* sfd){
  char c;
  printf("backline\n");
  //keep going back until a newline character is identified
  //Testing fseek ensures that, if this is the start of the file,
  //the loop will not continue forever.
  for (fseek(sfd, -2, SEEK_CUR), fread(&c, sizeof(char), 1, sfd);
       c != 10 && !fseek(sfd, -2, SEEK_CUR);
       fread(&c, sizeof(char), 1, sfd))
    ;
  //decrease line count
  location--;
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
  for (notend = fread(&c, sizeof(char), 1, sfd);
       notend && (c == 10 || c == 13);
       notend = fread(&c, sizeof(char), 1, sfd)) {
    printf("garbage c = %d\n", c);
    if (c == 10) {
      fread(&c, sizeof(char), 1, sfd);
      break;
    }
  }
  //go back so that the valid character is seen; if !notend, then not needed
  if (notend)
    fseek(sfd, -1, SEEK_CUR);
  //read line until you reach a terminating character
  //just finding the size of the buffer right now
  for (i = 0, notend = fread(&c, sizeof(char), 1, sfd);
       notend && c != 10 && c != 13;
       notend = fread(&c, sizeof(char), 1, sfd)) {
    printf("check c = %d\n", c);
    i++;
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
  if (notend)
    fseek(sfd, -(i+1), SEEK_CUR);
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
  location++;
  //if the line is blank, grab a non-blank one
  //returning a blank line indicates the end of the file
  if (notend && i == 1)
    grabline(buff, sfd);
  //This function will leave the file cursor about to read the character
  //just after the f i r s t line terminating character.
}

void forargs(char* buff, char*** point) {}

void strarg(char* buff, char*** point) {}

void strnumarg(char* buff, char*** point) {}

void grabargs(char* buff, char*** point, int argc) {}

void initmacro(char* buff) {}

void simplify(char* buff) {}

unsigned long hexstrnum(char* buff) {}

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
	printf("error 0x04\nL%d: Space within variable name\n", location);
	exit(0x04);
      }
    //go back to the start of the name
    i -= j;
    if (j == 0) {
      printf("error 0x03\nL%d: Empty declaration\n", location);
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
	  printf("error 0x06\nL%d: incomplete array declaration\n", location);
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
      simplify(tinybuff);
      size = hexstrnum(tinybuff);
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
    
int insertline(struct l** point, FILE* sfd, long depth) {
  //initialize for memory management ease of use
  char* buff = NULL;
  char* tinybuff = NULL;
  long i, j;
  int type;
  int argc;
  printf("insertline\n");
  //increment across the lines in this block, increment line count each time
  for (i = 0; 1; i++) {
    printf("insertline.for\n");
    //allocate ram for this action and initialize
    if ((*point) == NULL)
      (*point) = malloc(sizeof(struct l)*(i+1));
    else
      (*point) = realloc((*point), sizeof(struct l)*(i+1));
    (*point)[i].line = NULL;
    (*point)[i].arg = NULL;
    //get the next n o n - b l a n k line; blank line in buff[] = end of file
    grabline(&buff, sfd);
    //variable location is now constant for the remainder of the loop
    printf("\ninsertline.for: location:%d\n", location);
    printf("gotline:");
    printf("%s\n", buff);
    //figure out the type of the line
    type = typeline(buff);
    //handle comments and ending first and ignore if the line contains a comment
    if (type == BEGCOM) {
      comment = 1;
      continue;
    }
    else if (type == ENDCOM) {
      comment = 0;
      continue;
    }
    if (type == TERM) {
      (*point)[i].type = TERM;
      return 0;
    }
    //if this is actually code, then use the line
    if (!comment) {
      //create a new instance inside the struct and re-read the same line
      //once done, simply continue where you left off
      //move i to the correct index
      //avoid incrementing location
      if (linedepth(buff) > depth) {
	printf("depth increase yall\n");
	backline(sfd);
	if (i > 0)
	  i--;
	insertline(&((*point)[i].line), sfd, depth+1);
	continue;
      }
      //return to the previous instance after marking this as the end
      //again, rewind a line to allow continuing from where you left off
      //avoid incrementing location
      else if (linedepth(buff) < depth) {
	printf("depth decrease yall\n");
	(*point)[i].type = TERM;
	backline(sfd);
	return 0;
      }
      //check for lines that provide compiler initiation instructions
      //do not save these lines but d o increment location
      if (type == INT) {
	initint(buff);
	i--;
      }
      else if (type == MACRO) {
	initmacro(buff);
	i--;
      }
      //insert the line if the depth is the same and it needs to be saved
      //save the lines and increment the location
      (*point)[i].type = type;
      (*point)[i].location = location;
      //starting by looking at NORM type lines
      else if (type & NORM) {
	argc = 0;
	if (type & ARGS0)
	  argc += 1;
	if (type & ARGS1)
	  argc += 2;
	grabargs(buff, &((*point)[i].arg), argc);
      }
      else if (type == FOR)
	forargs(buff, &((*point)[i].arg));
      else if (type == SET || type == INS || type == GOTO)
	strarg(buff, &((*point)[i].arg));
      else if (type == SETA || type == INSA || type == GOTOA)
	strnumarg(buff, &((*point)[i].arg));
      else {
	printf("error 0x05\nL%d: undefined line type %d\n", location, type);
	exit(0x05);
      }
    }
    //get rid of this line if it is a comment but don't reduce location count
    if (comment)
      i--;
  }
}

void show(struct l* point) {
  long i;

  for (i = 0; point[i].type != TERM; i++) {
    //printf("read: %d\n", point[i].type);
    if (point[i].arg != NULL) {
      printf("arg type%d location%d\t:%s\n", point[i].type, point[i].location,
	     point[i].arg[0]);
    }
    if (point[i].line != NULL) {
      //printf("nest\n");
      show(point[i].line);
    }
  }
  //printf("de-nest\n");
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
  printf("hola0\n");
  insertline(&line, sfd, 0);
  show(line);
  fclose(sfd);
  return 0;
}
