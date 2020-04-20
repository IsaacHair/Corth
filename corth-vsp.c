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
struct r {
  char* name;
  unsigned long size;
  unsigned long* value;
} *ram = NULL;
struct b {
  char* name;
  unsigned long location;
} *label = NULL;
_Bool comment;
int error;
unsigned long location;

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
}

int linedepth(char* buff) {
  long i;
  //just keep seeing how many tabs there are at the start
  for (i = 0; buff[i] == '\t' && buff[i] != '\0'; i++)
    ;
  return i;
}
int typeline(char* buff) {
  printf("typeline\n");
  if (buff[0] == '\0')
    return TERM;
  return E;
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
  if (*buff == NULL)
    *buff = malloc(sizeof(char)*(i+1));
  else
    *buff = realloc(*buff, sizeof(char)*(i+1));
  //rewind file to re-read line
  //if the loop terminated because it was the end, then there is 1 less rewind
  if (notend)
    fseek(sfd, -(i+1), SEEK_CUR);
  else
    fseek(sfd, -i, SEEK_CUR);
  //actually copy into the buffer
  for (i = 0, notend = fread(&c, sizeof(char), 1, sfd);
       notend && c != 10 && c != 13;
       notend = fread(&c, sizeof(char), 1, sfd)) {
    printf("copy c = %d\n", c);
    (*buff)[i] = c;
    i++;
  }
  //terminating character
  (*buff)[i] = '\0';
  printf("i = %d; got line:%s\n", i, *buff);
  //if the line is blank, increase the location count and grab a non-blank one
  //returning a blank line indicates the end of the file
  if (notend && i == 0) {
    location++;
    grabline(buff, sfd);
  }
  //This function will leave the file cursor about to read the character
  //just after the f i r s t line terminating character.
}

int insertline(struct l** point, FILE* sfd, long depth) {
  //initialize for memory management ease of use
  char* buff = NULL;
  long i, j;
  printf("insertline\n");
  //increment across the lines in this block, increment line count each time
  for (i = 0; 1; i++) {
    printf("\ninsertline.for: location:%d\n", location);
    //check for an error on any level
    if (error)
      return 1;
    //allocate ram for this action and initialize
    if ((*point) == NULL)
      (*point) = malloc(sizeof(struct l)*(i+1));
    else
      (*point) = realloc((*point), sizeof(struct l)*(i+1));
    (*point)[i].line = NULL;
    (*point)[i].arg = NULL;
    //get the next n o n b l a n k line; blank line in buff[] = end of file
    grabline(&buff, sfd);
    printf("gotline:");
    printf("%s\n", buff);
    //handle comments and ending first
    if (typeline(buff) == BEGCOM)
      comment = 1;
    else if (typeline(buff) == ENDCOM)
      comment = 0;
    if (typeline(buff) == TERM) {
      (*point)[i].type = TERM;
      return 0;
    }
    //if this is actually code, then use the line
    if (!comment) {
      //create a new instance inside the struct and re-read the same line
      //once done, simply continue where you left off
      //move i to the correct index
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
      else if (linedepth(buff) < depth) {
	printf("depth decrease yall\n");
	(*point)[i].type = TERM;
	backline(sfd);
	return 0;
      }
      //insert the line if the depth is the same
      switch (typeline(buff)) {
      case E:
	(*point)[i].type = E;
	(*point)[i].location = location;
	for (j = 0; buff[j] != TERM; j++)
	  ;
	(*point)[i].arg = malloc(sizeof(char*)*1);
	(*point)[i].arg[0] = malloc(sizeof(char)*(j+1));
	for (j = 0; buff[j] != TERM; j++)
	  (*point)[i].arg[0][j] = buff[j];
	(*point)[i].arg[0][j] = TERM;
	break;
      }
    }
    //get rid of this line if it is a comment but don't reduce location count
    else
      i--;
    //increase location after handling the line only if it's the right depth
    location++;
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
    printf("error 1\nusage: cvm <source file> <target file>\n");
    return 1;
  }
  FILE* sfd = fopen(argv[1], "rb");
  if (sfd == NULL) {
    printf("error 2\nsource file not found\n");
    return 2;
  }
  _Bool comment = 0;
  error = comment = 0;
  location = 1;
  printf("hola0\n");
  insertline(&line, sfd, 0);
  show(line);
  fclose(sfd);
  if (error) {
    printf("error %d on line %d\n", error, location);
    return error;
  }
  return 0;
}
