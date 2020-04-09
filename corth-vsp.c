#include <stdio.h>
#include <stdlib.h>

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

#define SLAST 17

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

#define FOR 65
#define LABEL 66
#define GOTO 67
#define IF 68
#define ELSE 69
#define ADR 70
#define OUT 71
#define IN 72
#define ASN 73
#define INT 74

int chunk(char* shift, char* comm) {
  //grabs an expression that is not a token;
  //the ending spacer character is kept and the beginning one is not
  //eg "int (shit, damn)" processes to "damn)" instead of " damn)" or " damn"
  //only don't send last character if it is EOF
  int i, j;

  for (i = SLAST-1; i >= 0 && ((shift[i] >= 'a' && shift[i] <= 'z') ||
			       shift[i] == '[' || shift[i] == ']');
       i--) ;
  for (i++, j = 0; i <= SLAST; i++, j++)
    if (shift[i] != EOF)
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
    return 1;
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

void token(char* s, char* t) {
  FILE* sfd = fopen(s, "r");
  FILE* tfd = fopen(t, "wb");
  char shift[SLAST+2];
  char comm[SLAST+2];
  int i;
  shift[SLAST+1] = '\0'; //diagnostic purposes; allows string to be printf-ed
  comm[SLAST+1] = '\0';
  
  for (i = 0; i <= SLAST; i++)
    shift[i] = comm[i] = ' ';
  for (; shift[SLAST-1] != EOF;) {
    for (i = 0; i <= SLAST-1; i++)
      shift[i] = shift[i+1];
    shift[SLAST] = fgetc(sfd);
    find(shift, comm);
    fwrite(comm, 1, find(shift, comm), tfd);
  }
  
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
  token(t, b);

  return 0;
}
