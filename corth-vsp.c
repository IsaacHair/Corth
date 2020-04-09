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

#define FOR "\001"
#define LABEL "\002"
#define GOTO "\003"
#define IF "\004"
#define ELSE "\005"
#define ADR "\006"
#define OUT "\007"
#define IN "\010"
#define ASN "\011"
#define INT "\012"

int find(char* shift, char* comm) {
  if (compare(shift, "int"))
    ret = INT;
  else if (compare(shift, "for"))
    ret = FOR;
  /*else if (compare(shift, "goto"))
    return GOTO;
  else if (compare(shift, "if"))
    return IF;
  else if (compare(shift, "else"))
    return ELSE;
  else if (compare(shift, "adr"))
    return ADR;
  else if (compare(shift, "out"))
    return OUT;
  else if (compare(shift, "in"))
    return IN;
  else if ((shift[SLAST] > 'z' || shift[SLAST] < 'a') &&
	   shift[SLAST] != '[' && shift[SLAST] != ']')
	   return ('c');*/
  else
    return 0;

  return 1;
}

void token(char* s, char* t) {
  FILE* sfd = fopen(s, "r");
  FILE* tfd = fopen(t, "w");
  char shift[SLEN+1];
  char comm[SLEN+1];
  int i;
  
  for (i = 0; i < SLEN; i++)
    shift[i] = ' ';
  for (shift[SLEN] = fgetc(sfd); shift[SLEN-1] != EOF;
       shift[SLEN] = fgetc(sfd)) {
    for (i = 0; i < SLEN-1; i++)
      shift[i] = shift[i+1];
    if (find(shift, comm))
      fwrite(
  
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
