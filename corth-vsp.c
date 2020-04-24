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
#define LINESIZE 15

void backline(FILE* sfd) {
  //keep going back until a newline character is identified
  char c;
  for (fseek(sfd, -2, SEEK_CUR), fread(&c, sizeof(char), 1, sfd);
       c != 10 && !fseek(sfd, -2, SEEK_CUR);
       fread(&c, sizeof(char), 1, sfd))
    sidx--;
  if (sidx > 0)
    sidx--;
  else
    sidx = 0;
  sline--;
}

int linedepth(char* buff) {
  //just keep seeing how many tabs there are at the start
  unsigned long i;
  for (i = 0; buff[i] == '\t' && buff[i] != '\0'; i++)
    ;
  return i;
}

_Bool find(char* word, char* str, unsigned long place) {
  //see if this string is at a specific location; ~ is wildcard
  unsigned long i, j;
  i = 0;
  if (word[0] == '~') {
    if (place > 0)
      if (str[place-1] <= 'z' && str[place-1] >= 'a')
	return 0;
    i++;
  }
  for (j = place; word[i] != '\0' && word[i] != '~'; i++, j++)
    if (word[i] != str[j])
      return 0;
  if (word[i] == '~')
    if (str[j] <= 'z' && str[j] >= 'a')
      return 0;
  return 1;
}

int typeline(char* buff) {
  //look for key words and determine the type of the line
  unsigned long i;
  unsigned long last;
  unsigned long argc;
  int type;
  if (buff[0] == '\0')
    return TERM;
  for (i = last = 0; buff[i] != '\0'; i++, last++)
    ;
  type = 0;
  argc = 0;
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
	else if (type == 0)
	  type = INSA;
    if (i+1 <= last)
      if (find("->", buff, i)) {
	argc++;
	if (!(type&NORM))
	  type = NORM;
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
      else if (find("~if~", buff, i)) {
        if (!(type&NORM))
	  type = NORM;
	type |= IF;
      }
      else if (find("~in~", buff, i)) {
        if (!(type&NORM))
	  type = NORM;
	type &= ~ANIO;
      }
    if (i+2 <= last)
      if (find("~adr~", buff, i)) {
        if (!(type&NORM))
	  type = NORM;
	type |= ANIO;
      }
      else if (find("~out~", buff, i)) {
        if (!(type&NORM))
	  type = NORM;
	type &= ~ANIO;
      }
      else if (find("~for~", buff, i))
	type = FOR;
      else if (find("~int~", buff, i))
	type = INT;
    if (i+3 <= last)
      if (find("~goto~", buff, i))
	type = GOTO;
      else if (find("~else~", buff, i)) {
        if (!(type&NORM))
	  type = NORM;
	type |= ELSE;
      }
  }
  //use argc and finalize the INS type (it is the only one with no key word)
  if (type == 0)
    type = INS;
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
  int notend;
  for (notend = fread(&c, sizeof(char), 1, sfd);
       notend && (c == 10 || c == 13);
       notend = fread(&c, sizeof(char), 1, sfd)) {
    if (c == 10) {
      fread(&c, sizeof(char), 1, sfd);
      break;
    }
    sidx++;
  }
  if (notend) {
    fseek(sfd, -1, SEEK_CUR);
    for (i = 0, notend = fread(&c, sizeof(char), 1, sfd);
	 notend && c != 10 && c != 13;
	 notend = fread(&c, sizeof(char), 1, sfd)) {
      i++;
      sidx++;
    }
    if (*buff == NULL)
      *buff = malloc(sizeof(char)*(i+1));
    else
      *buff = realloc(*buff, sizeof(char)*(i+1));
    if (notend) {
      sidx++;
      fseek(sfd, -(i+1), SEEK_CUR);
    }
    else
      fseek(sfd, -i, SEEK_CUR);
    for (i = 0, notend = fread(&c, sizeof(char), 1, sfd);
	 notend && c != 10 && c != 13;
	 notend = fread(&c, sizeof(char), 1, sfd)) {
      (*buff)[i] = c;
      i++;
    }
    (*buff)[i] = '\0';
    sline++;
    if (notend && i == 0)
      grabline(buff, sfd);
  }
  else {
    (*buff)[0] = '\0';
  }
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

void printnum(FILE* fd, short num) {
  int a;
  char c;
  for (a=1<<12; a > 0; a = a>>4) {
    if (num/a%16 < 10)
      c = num/a%16+'0';
    else
      c = num/a%16+'a'-10;
    fwrite(&c, sizeof(char), 1, fd);
  }
}

void setnext(FILE* fd, unsigned long i, unsigned long l) {
  if (fseek(fd, i+10, SEEK_SET)) {
    printf("error 0x0a\nL%d: unable to access next re-write\n", sline);
    exit(0x0a);
  }
  printnum(fd, l%(1<<16));
  if (fseek(fd, tidx, SEEK_SET)) {
    printf("error 0x0b\nL%d: unable to return to cursor location\n\
this is most likely indicative of a severe compiler fault\n", sline);
    exit(0x0b);
  }
}

void writecomm(FILE* fd, char* opcode, unsigned long select) {
  int a;
  char c;
  printnum(fd, tidx/LINESIZE%(1<<16));
  fwrite(opcode, sizeof(char), 2, fd);
  printnum(fd, select%(1<<16));
  tidx += LINESIZE;
  printnum(fd, tidx/LINESIZE%(1<<16));
  //clerity only;remove this later!!!!! also update LINESIZE
  fprintf(fd, " ");
}

void initint(char* buff) {
  unsigned long i, j;
  char* tinybuff;
  unsigned long size;
  for (i = 3, tinybuff = NULL; 1;) {
    while(buff[i] == ' ')
      i++;
    for (j = 0; buff[i] != '[' && buff[i] != ',' && buff[i] != '\0'; i++, j++)
      if (buff[i] == ' ') {
	printf("error 0x04\nL%d: Space within variable name\n", sline);
	exit(0x04);
      }
    i -= j;
    if (j == 0) {
      printf("error 0x03\nL%d: Empty declaration\n", sline);
      exit(0x03);
    }
    rcount++;
    if (ram == NULL)
      ram = malloc(sizeof(struct r)*rcount);
    else
      ram = realloc(ram, sizeof(struct r)*rcount);
    ram[rcount-1].name = malloc(sizeof(char)*(j+1));
    for (j = 0; buff[i] != '[' && buff[i] != ',' && buff[i] != '\0'; j++, i++)
      ram[rcount-1].name[j] = buff[i];
    ram[rcount-1].name[j] = '\0';
    if (buff[i] == '[') {
      for (j = 0, i++; buff[i] != ']'; i++, j++)
	if (buff[i] == '\0') {
	  printf("error 0x06\nL%d: incomplete array declaration\n", sline);
	  exit(0x06);
	}
      if (tinybuff == NULL)
        tinybuff = malloc(sizeof(char)*(j+1));
      else
        tinybuff = realloc(tinybuff, sizeof(char)*(j+1));
      i -= j;
      for (j = 0; buff[i] != ']'; i++, j++)
	tinybuff[j] = buff[i];
      tinybuff[j] = '\0';
      size = expr(tinybuff, 0);
      ram[rcount-1].size = size;
      ram[rcount-1].value = malloc(sizeof(unsigned long)*size);
    }
    else {
      ram[rcount-1].size = 1;
      ram[rcount-1].value = malloc(sizeof(unsigned long));
    }
    if (buff[i] == '\0')
      break;
    i++;
  }
}

void argument(int* argc, int type) {
  *argc = 0;
  if (type & ARGS0)
    *argc |= 1;
  if (type & ARGS1)
    *argc |= 2;
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
  buff = NULL;
  while (1) {
    grabline(&buff, sfd);
    type = typeline(buff);
    switch (type) {
    case BEGCOM:
      comment = 1;
      break;
    case ENDCOM:
      comment = 0;
      break;
    case TERM:
      usegotos();
      return 0;
    }
    if (comment)
      continue;
    if (linedepth(buff) < depth) {
      backline(sfd);
      return 0;
    }
    if (tidx/LINESIZE >= (1<<16)) {
      printf("error 0x07\nL%d: compiled program exceeds maximum size \
(64k instructions)\n", sline);
      exit(0x07);
    }
    switch (type) {
    case INT:
      initint(buff);
      break;
    case MACRO:
      initmacro(buff);
      break;
    case FOR:
      initfor(buff);
      while (testfor(buff)) {
	fbstart = sidx;
	insertline(sfd, tfd, depth+1);
	incfor(buff);
	if (testfor(buff))
	  movecur(sfd, fbstart);
      }
      break;
    case INS:
      usemacro(buff, 0);
      break;
    case INSA:
      usemacro(buff, expr(buff, 0));
      break;
    case SET:
      setlabel(buff, 0);
      break;
    case SETA:
      setlabel(buff, expr(buff, 0));
      break;
    case GOTO:
      setgoto(buff, 0);
      break;
    case GOTOA:
      setgoto(buff, expr(buff, 0));
      break;
    }
    //type should be normal unless error
    if (!(type&NORM)) {
      printf("error0x05\nL%d invalid line type", sline);
      exit(0x05);
    }
    argument(&argc, type);
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
  insertline(sfd, tfd, 0);
  fclose(sfd);
  fclose(tfd);
  return 0;
}
