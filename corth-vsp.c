HEAD
#include <stdio.h>
#include <stdlib.h>

//tokens
#define TSTART 65
#define FOR    (TSTART+0)
#define LABEL  (TSTART+1)
#define GOTO   (TSTART+2)
#define IF     (TSTART+3)
#define ELSE   (TSTART+4)
#define ADR    (TSTART+5)
#define OUT    (TSTART+6)
#define IN     (TSTART+7)
#define ASN    (TSTART+8)
#define INT    (TSTART+9)
#define TAB    (TSTART+10)
#define END    (TSTART+11)
#define SEP    (TSTART+12)

//index to last element of buffer
#define SLAST 17

void comment(char* s, char* t) {
  FILE* sfd = fopen(s, "r");
  FILE* tfd = fopen(t, "w");
  char c;
  _Bool comment;
  
  for (c = fgetc(sfd), comment = 0; c != EOF; c = fgetc(sfd))
    if (c == '/')
      if ((c = fgetc(sfd)) == '*')
	comment = 1;
      else
	c == EOF ? fprintf(tfd, "/") : fprintf(tfd, "/%c", c);
    else if (c == '*')
      if ((c = fgetc(sfd)) == '/')
	comment = 0;
      else
	c == EOF ? fprintf(tfd, "*") : fprintf(tfd, "*%c", c);
    else if (!comment)
      fprintf(tfd, "%c", c);

  fclose(sfd);
  fclose(tfd);
}

void flow(char* s, char* t) {
  FILE * sfd = fopen(s, "r");
  FILE * tfd = fopen(t, "wb");
  char c;
  int new;
  
  for (c = fgetc(sfd), new = 1; c != EOF; c = fgetc(sfd))
    if (c == '\n' || c == 10 || c == 13 || c == ';') {
      if (!new) {
	c = END;
	fwrite(&c, 1, 1, tfd);
	new = 1;
      }
    }
    else if (c == '\t') {
      c = TAB;
      fwrite(&c, 1, 1, tfd);
    }
    else if (c == ',') {
      c = SEP;
      fwrite(&c, 1, 1, tfd);
    }
    else {
      new = 0;
      fwrite(&c, 1, 1, tfd);
    }
  
  fclose(sfd);
  fclose(tfd);
}

_Bool compare(char* shift, char* object) {
  int i, j;
  _Bool result;
  for (i = 0; object[i] != '\0'; i++) ;
  for (result = 1, j = SLAST-1, i--; i >= 0; i--, j--)
    if (shift[j] != object[i])
      result = 0;
  if ((shift[j] >= 'a' && shift[j] <= 'z') || shift[j] == '[' ||
      shift[j] == ']')
    result = 0;
  if ((shift[SLAST] >= 'a' && shift[SLAST] <= 'z') || shift[SLAST] == '[' ||
      shift[SLAST] == ']')
    result = 0;
  return (result);
}

int chunk(char* shift, char* comm) {
  //grabs an expression that is not a token;
  //the ending spacer character is kept and the beginning one is not
  //eg "int (shit, damn)" processes to "damn)" instead of " damn)" or " damn"
  //only don't send last character if it is EOF
  int i, j;

  for (i = SLAST-1; i >= 0 && ((shift[i] >= 'a' && shift[i] <= 'z') ||
			       shift[i] == '[' || shift[i] == ']'); i--) ;
  for (i++, j = 0; i <= SLAST && shift[i] != EOF; i++, j++)
    comm[j] = shift[i];
  comm[j] = '\0';
  return j;
}

int find(char* shift, char* comm) {
  if (compare(shift, "int")) {
    comm[0] = INT;
    if (shift[SLAST] != EOF)
      comm[1] = shift[SLAST];
    return 2;
  }
  else if (compare(shift, "for")) {
    comm[0] = FOR;
    if (shift[SLAST] != EOF)
      comm[1] = shift[SLAST];
    return 2;
  }
  else if (compare(shift, "goto")) {
    comm[0] = GOTO;
    if (shift[SLAST] != EOF)
      comm[1] = shift[SLAST];
    return 2;
  }
  else if (compare(shift, "if")) {
    comm[0] = IF;
    if (shift[SLAST] != EOF)
      comm[1] = shift[SLAST];
    return 2;
  }
  else if (compare(shift, "else")) {
    comm[0] = ELSE;
    if (shift[SLAST] != EOF)
      comm[1] = shift[SLAST];
    return 2;
  }
  else if (compare(shift, "adr")) {
    comm[0] = ADR;
    if (shift[SLAST] != EOF)
      comm[1] = shift[SLAST];
    return 2;
  }
  else if (compare(shift, "out")) {
    comm[0] = OUT;
    if (shift[SLAST] != EOF)
      comm[1] = shift[SLAST];
    return 2;
  }
  else if (compare(shift, "in")) {
    comm[0] = IN;
    if (shift[SLAST] != EOF)
      comm[1] = shift[SLAST];
    return 2;
  }
  else if ((shift[SLAST] > 'z' || shift[SLAST] < 'a') &&
	   shift[SLAST] != '[' && shift[SLAST] != ']')
    return (chunk(shift, comm));
  else
    return 0;
}

void firsttoken(char* s, char* t) {
  FILE* sfd = fopen(s, "rb");
  FILE* tfd = fopen(t, "wb");
  char shift[SLAST+2];
  char comm[SLAST+2];
  int i;
  shift[SLAST+1] = '\0'; //diagnostic purposes; allows string to be printf-ed
  comm[SLAST+1] = '\0';
  
  for (i = 0; i <= SLAST; i++)
    shift[i] = comm[i] = ' ';
  for (; shift[SLAST] != EOF;) {
    for (i = 0; i <= SLAST-1; i++)
      shift[i] = shift[i+1];
    shift[SLAST] = fgetc(sfd);
    find(shift, comm);
    fwrite(comm, 1, find(shift, comm), tfd);
  }
  
  fclose(sfd);
  fclose(tfd);
}

void spacedelete(char * s, char * t) {
  FILE * sfd = fopen(s, "rb");
  FILE * tfd = fopen(t, "wb");
  char c;
  
  while (fread(&c, 1, 1, sfd))
    if (c != ' ')
      fwrite(&c, 1, 1, tfd);

  fclose(sfd);
  fclose(tfd);
}

int main(int argc, char** argv) {
  if (argc != 4) {
    printf("usage: cvm <source> <target> <buffer>\n");
    return (-1);
  }

  char* s = argv[1];
  char* t = argv[2];
  char* b = argv[3];

  comment(s, t);
  flow(t, b);
  firsttoken(b, t);
  spacedelete(t, b);
  //secondtoken(b, t);
  //while (bracket(t, b))
  //  bracket(b, t);
  
  return 0;
}0429dc0b3fbacc6d8aa424f4d6fc118347186ce9
