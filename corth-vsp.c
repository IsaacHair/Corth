#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define NULL 0
#define DEC 1
#define CALL 2
#define INT 3
#define IF 4
#define ELSE 5
#define ADR 6
#define OUT 7
#define LABEL 8
#define GOTO 9
#define ASN 10
#define TAB 11
#define FOR 12

struct ramstruct {
  char label[17];
  uint64_t value;
} *ram;

void comment(char* sourceadr, char* targetadr) {
  FILE * source = fopen(sourceadr, "r");
  FILE * target = fopen(targetadr, "w");
  char c, p;
  int code;

  for ((p = fgetc(source)) == EOF ? (c = p) : (c = fgetc(source)), code = 1;
       c != EOF;)
    if (p == '/' && c == '*') {
      code = 0;
      (p = fgetc(source)) == EOF ? (c = p) : (c = fgetc(source));
    }
    else if (p == '*' && c == '/') {
      code = 1;
      (p = fgetc(source)) == EOF ? (c = p) : (c = fgetc(source));
    }
    else if (code) {
      fprintf(target, "%c", p);
      p = c;
      c = fgetc(source);
    }
    else {
      p = c;
      c = fgetc(source);
    }
  if (code && (p != EOF))
    fprintf(target, "%c", p);

  fclose(source);
  fclose(target);
}

void token(char* sourceadr, char* targetadr) {
  FILE * source = fopen(sourceadr, "r");
  FILE * target = fopen(targetadr, "w");
  char shift[18];
  int i;

  for (i = 0; i < 18; i++)
    shift[i] = '\0';
  for (; shift[16] != EOF; shift[17] != EOF ? (shift[17] = fgetc(source)) : 1, tokeninc(shift)) {
    //checking shift[16] instead of [17] means that you will have a chance to check for key words at the end of the file
    if (tokencheck(shift))
      fprintf(target, "%c", tokencheck(shift));
    
void mem(char* sourceadr, char* targetadr) {
  FILE * source = fopen(sourceadr, "r");
  FILE * target = fopen(targetadr, "w");
  char a, b, c, d, e;
  int value, count, i;
  char buff[16];

  for (a = b = c = d = '\0', value = 0, count = 0, e = fgetc(source); e != EOF;
       a = b, b = c, c = d, d = e, e = fgetc(source))
    if (a == 'i' && b == 'n' && c == 't') {
      value = 1;
      count++;
    }
    else if (c == '\n')
      value = 0;
    else if (value && c == ',')
      count++;
  ram = (struct ramstruct*)malloc(sizeof(struct ramstruct[count]));
  for (b = '\0', a = '\0', value = 0, count = 0, i = 0; (c = fgetc(source)) != EOF; a = b, b = c)
    if (a == 'i' && b == 'n' && c == 't') {
      value = 1;
      for (; i > 0; i--)
	ram[count].label[i] = buff[i];
    }
    else if (c == '\n' && value) {
      value = 0;
      count++;
    }
    else if (value && c == ',') {
      i = 0;
      count++;
    }
    else if ('a' <= c <= 'z')
  printf("ram: %s, %d\n", ram[0].label, ram[0].value);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    printf("Usage: cov <source> <target> <buffer>\n");
    return (-1);
  }

  //go back and forth, keeping most compiled version in target file
  comment(argv[1], argv[2]);
  token(argv[2], argv[3]);
  /*mem(argv[3], argv[2]);
  do {
    if (!macros(argv[2], argv[3])) {
      copy(argv[3], argv[2]);
      break;
    }
  } while (macros(argv[3], argv[2]));
  do {
    if (!fors(argv[2], argv[3])) {
      copy(argv[3], argv[2]);
      break;
    }
  } while (fors(argv[3], argv[2]));
  do {
    if (!evaluate(argv[2], argv[3])) {
      copy(argv[3], argv[2]);
      break;
    }
  } while (evaluate(argv[3], argv[2]));
  translate(argv[2], argv[3]);
  gotos(argv[3], argv[2]);*/
  free(ram);

  return (0);
}
