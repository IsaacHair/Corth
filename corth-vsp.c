#include <stdio.h>
#include <stdlib.h>

#define TERM	0      //the program is done
#define LABEL	1
#define GOTO	2
#define INS	3	//insert macro
#define II0	4	//if in, 0 arguments
#define II1	5	//if in, mask
#define IA0	6	//if address, 0 arguments
#define IA1	7	//if address, value to set address to only
#define IA2	8	//if address, value to set address to and mask
#define IA0A	9	//previous def + assignment
#define IA1A	10	//previous def + assignment
#define IA2A	11	//previous def + assignment
#define E	12	//else
#define EII0	13	//else + previous def (this is the same for the rest)
#define EII1	14
#define EIA0	15
#define EIA1	16
#define EIA2	17
#define EIA0A	18
#define EIA1A	19
#define EIA2A	20
#define A1	21
#define A2	22
#define A0A	23
#define A1A	24
#define A2A	25
#define O1	26
#define O2	27
#define LABELA	28	//label with an array
#define GOTOA	29	//goto with an array
#define INSA	30	//insert with an array
#define FOR	31
#define BEGCOM  32
#define ENDCOM  33
#define BLANK   34

struct l {
  int type;
  char** arg;
  struct l* line;
} *line = NULL;
struct m {
  char* name;
  struct l* line;
} *macro = NULL;
struct r {
  char* name;
  long size;
  long* value;
} *ram = NULL;
struct b {
  char* name;
  long location;
} *label = NULL;

void backline(FILE* sfd){}
int linedepth(char* buff){}
int typeline(char* buff){}

void grabline(char** buff, FILE* sfd) {
  int i;
  char c;
  //allows end of the file to be identified; fread will return 0
  int notend;
  //skip all the garbage that marks the end of the line (might be >1 characters)
  for (notend = fread(&c, sizeof(char), 1, sfd);
       notend && (c == '\n' || c == 10 || c == 13);
       notend = fread(&c, sizeof(char), 1, sfd))
    ;
  //read line until you reach a terminating character
  //just finding the size of the buffer right now
  for (i = 0, notend = fread(&c, sizeof(char), 1, sfd);
       notend && c != '\n' && c != 10 && c != 13;
       notend = fread(&c, sizeof(char), 1, sfd))
    i++;
  //allocate buffer
  //realloc prevents huge amounts of ram from being consumed
  if (*buff == NULL)
    *buff = malloc(sizeof(char)*(i+1));
  else
    *buff = realloc(sizeof(char)*(i+1));
  //rewind file to re-read line
  fseek(sfd, -(i+1), SEEK_CUR);
  //actually copy into the buffer
  for (i = 0, notend = fread(&c, sizeof(char), 1, sfd);
       notend && c != '\n' && c != 10 && c != 13;
       notend = fread(&c, sizeof(char), 1, sfd)) {
    (*buff)[i] = c;
    i++;
  }
  //terminating character
  (*buff)[i] = '\0';
  //This function will leave the file cursor about to read the character
  //just after the first line terminating character.
}

int insertline(struct l* point, _Bool* comment, FILE* sfd, long depth,
	       int* error, long* line) {
  //initialize for memory management ease of use
  char* buff = NULL;
  long i;

  //increment across the lines in this block, increment line count each time
  for (i = 0; 1; i++, (*line)++) {
    //check for an error on any level
    if (*error)
      return 1;
    //allocate ram for this action and initialize
    if (point == NULL)
      point = malloc(sizeof(struct l)*(i+1));
    else
      point = realloc(point, sizeof(struct l)*(i+1));
    point[i].line = NULL;
    //get the next line
    grabline(&buff, sfd);
    //handle comments and ending first
    if (typeline(buff) == BEGCOM)
      *comment = 1;
    else if (typeline(buff) == ENDCOM)
      *comment = 0;
    if (typeline(buff) == TERM) {
      point[i].type = TERM;
      return 0;
    }
    //if this is actually code, then use the line
    if (!(*comment)) {
      //create a new instance inside the struct and re-read the same line
      //once done, simply continue where you left off
      if (linedepth(buff) > depth) {
	backline(sfd);
	insertline(point[i].line, comment, sfd, depth+1, error, line);
	continue;
      }
      //return to the previous instance after marking this as the end
      //again, rewind a line to allow continuing from where you left off
      if (linedepth(buff) < depth) {
	point[i].type = TERM;
	backline(sfd);
	return 0;
      }
      //insert the line if the depth is the same
      switch (typeline(buff)) {
      case BLANK:
	continue;
      case LABEL:
	etcman;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("error 1\nusage: cvm <source file> <target file>\n");
    return 1;
  }
  FILE* sfd = fopen(argv[1], "r");
  if (test == NULL) {
    printf("error 2\nsource file not found\n");
    return 2;
  }
  _Bool comment = 0;
  int error = 0;
  long line = 0;
  insertline(line, &comment, sfd, 0, &error, &line);
  fclose(sfd);
  if (error) {
    printf("error %d on line %d\n", error, line);
    return error;
  }
  return 0;
}
