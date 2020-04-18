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
void grabline(char** buff, FILE* sfd){}
int typeline(char* buff){}

int insertline(struct l* point, _Bool* comment, FILE* sfd, long depth) {
  char* buff;
  long i;
  
  for (i = 0; 1; i++) {
    grabline(&buff, sfd);
    if (typeline(buff) == BEGCOM)
      *comment = 1;
    else if (typeline(buff) == ENDCOM)
      *comment = 0;
    if (typeline(buff) == TERM) {
      point[i].type = TERM;
      return 0;
    }
    if (!(*comment)) {
      if (linedepth(buff) > depth) {
	backline(sfd);
	insertline(point[i].line, comment, sfd, depth+1);
	continue;
      }
      if (linedepth(buff) < depth) {
	point[i].type = TERM;
	backline(sfd);
	return 0;
      }
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
  insertline(line, &comment, sfd, 0);
  return 0;
}
