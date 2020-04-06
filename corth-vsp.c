#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define DEC 'A'
#define CALL 'B'
#define INT 'C'
#define IF 'D'
#define ELSE 'E'
#define ADR 'F'
#define OUT 'G'
#define LABEL 'H'
#define GOTO 'I'
#define ASN 'J'
#define TAB 'K'
#define FOR 'L'
#define ENDING 'M'
#define ENDLN 'N'
#define TOKEN 0
#define DPL 1
//displacement (tells tokencheck() to return negative length of token)

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
      p = c, c = fgetc(source);
    }
  if (code && (p != EOF))
    fprintf(target, "%c", p);

  fclose(source);
  fclose(target);
}

void tokeninc(char * shift) {
  int i;
  for (i = 0; i < 17; i++)
    shift[i] = shift[i+1];
}

int compare(char * shift, char * object) {
  int i, j, val;
  if (object[0] != '%') {
    val = 1;
    for (i = 0; object[i] != '\0'; i++) ;
    i--;
    for (j = 16; i >= 0; j--, i--)
      if (shift[j] != object[i])
	val = 0;
    if (shift[17] >= 'a' && shift[17] <= 'z')
      val = 0;
    if (shift[j-1] >= 'a' && shift[j-1] <= 'z')
      val = 0;
    return (val);
  }
  else {
    //Because of how this checks, macro calls must be the only thing on a line
    //they are differentiated from variables because variables are only used
    //as part of a larger statement.
    //This has the added effect that you can declare a macro and a variable
    //with the same name.
    if ((shift[17] < 'a' || shift[17] > 'z') && !compare(shift, "int") &&
	!compare(shift, "if") && !compare(shift, "else") &&
	!compare(shift, "adr") && !compare(shift, "out") &&
	!compare(shift, "goto") && !compare(shift, "for"))
      for (i = 16, val = 0, j = 0; i >= 0; i--, j--)
        if (shift[i] == '\t' || shift[i] == '\n')
	  return val*j;
	else if (shift[i] <= 'a' || shift[i] >= 'z')
	  return 0;
	else
	  val = 1;
    else
      return (0);
  }
}

int tokencheck(char * shift, int select) {
  if (select == TOKEN) {
    if (shift[16] == '#')
      return DEC;
    else if (compare(shift, "int"))
      return INT;
    else if (compare(shift, "if"))
      return IF;
    else if (compare(shift, "else"))
      return ELSE;
    else if (compare(shift, "adr"))
      return ADR;
    else if (compare(shift, "out"))
      return OUT;
    else if (shift[16] == ':')
      return LABEL;
    else if (compare(shift, "goto"))
      return GOTO;
    else if (shift[16] == '=' && shift[17] != '=' && shift[15] != '=')
      return ASN;
    else if (shift[16] == '\t')
      return TAB;
    else if (compare(shift, "for"))
      return FOR;
    else if (compare(shift, "%call"))
      return CALL;
    else if (shift[16] == '\n')
      return ENDLN;
    else
      return 0;
  }
  else {
    if (shift[16] == '#')
      return -1;
    else if (compare(shift, "int"))
      return -3;
    else if (compare(shift, "if"))
      return -2;
    else if (compare(shift, "else"))
      return -4;
    else if (compare(shift, "adr"))
      return -3;
    else if (compare(shift, "out"))
      return -3;
    else if (shift[16] == ':')
      return -1;
    else if (compare(shift, "goto"))
      return -4;
    else if (shift[16] == '=' && shift[17] != '=' && shift[15] != '=')
      return -1;
    else if (shift[16] == '\t')
      return -1;
    else if (compare(shift, "for"))
      return -3;
    else if (compare(shift, "%call"))
      return compare(shift, "%call");
    else if (shift[16] == '\n')
      return -1;
    else
      return 0;
  }
}

void token(char* sourceadr, char* targetadr) {
  FILE * source = fopen(sourceadr, "r");
  FILE * target = fopen(targetadr, "w");
  char shift[19]; //19 so that the last character is \0 for printf purposes
  int i;
  char * end;

  shift[18] = '\0';
  for (i = 0; i < 18; i++)
    shift[i] = '\0';
  for (; shift[16] != EOF; tokeninc(shift),
       shift[17] != EOF ? (shift[17] = fgetc(source)) : 1)
    //checking shift[16] instead of [17] means
    //that you will have a chance to check for key words at the end of the file
    //moving cursor back means that you can just directly copy the file and,
    //if you get to a token, you can just go back and delete what you wrote
    //this means that at the end of the file it has to be purged so that
    //random text isn't hanging; add ENDING token
    //fine to use fseek because there will be no carriage return problems
    //eg. it is 2 bytes in the file but 1 character, so mismatch length
    //because by definition token keywords must be concatenated together
    if (tokencheck(shift, TOKEN)) {
      fseek(target, tokencheck(shift, DPL), SEEK_CUR);
      fprintf(target, "%c", tokencheck(shift, TOKEN));
    }
    else if (shift[17] != EOF) {
      //doing the following ensures proper handling of line breaks
      end = &(shift[17]);
      fprintf(target, end);
    }
  fprintf(target, "%c", ENDING);
  fclose(target);
  fclose(source);
}
    
/*void mem(char* sourceadr, char* targetadr) {
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
  for (b = '\0', a = '\0', value = 0, count = 0, i = 0;
       (c = fgetc(source)) != EOF; a = b, b = c)
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
  fclose(target);
  fclose(source);
  }*/

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
