#include <stdio.h>
#include <stdlib.h>

//tokens
#define TSTART 65

#define TERM   '\0'
#define FOR    (TSTART+1)
#define LABEL  ':'
#define GOTO   (TSTART+2)
#define IF     (TSTART+3)
#define ELSE   (TSTART+4)
#define ADR    (TSTART+5)
#define OUT    (TSTART+6)
#define IN     (TSTART+7)
#define ASN    '='
#define INT    (TSTART+8)
#define TAB    (TSTART+9)
#define END    (TSTART+10)
#define SEP    (TSTART+11)
#define FEND   (TSTART+12)
#define DEF    '#'

#define PS     '('
#define PE     ')'
#define SBS    '['
#define SBE    ']'
#define CBS    '{'
#define CBE    '}'

#define DIV    '/'
#define MOD    '%'
#define MUL    '*'
#define ADD    '+'
#define SUB    '-'
#define SU     (TSTART+13)
#define SD     (TSTART+14)
#define BAND   '&'
#define BOR    '|'
#define BXOR   '^'

#define LESS   '<'
#define MORE   '>'
#define ELESS  (TSTART+15)
#define EMORE  (TSTART+16)
#define EQL    (TSTART+17)
#define NEQL   (TSTART+18)
#define LAND   (TSTART+19)
#define LOR    (TSTART+20)
#define LXOR   (TSTART+21)
#define NOT    '!'

#define NUM    (TSTART+22)
#define NAME   (TSTART+23)
#define BS     (TSTART+24)
#define ES     (TSTART+25)

//index to last element of buffer
#define SLAST 17

//macro buffer
struct mtype {
  char label[17];
  char* body;
} *macro;
int mcount;

//variable buffer
struct rtype {
  char label[17];
  //dimensions go from left to right; in path[33][66][0aa]
  //33 is the value of dimension 0, 66 is the value of dimension 1
  //and 0aa is the value of dimension 2
  //when accessing memory, the zeroth dimension is added to the
  //first dimension times the number of elements in the zeroth dimension
  //and then added to the number of elements in the first times the number
  //in the zeroth dimes the value of the second dimension
  //This can be expanded to apply to between 0 dimensions and the max possible
  //with the ram available on the compiling computer
  unsigned long int *dimension;
  unsigned long int *value;
} *ram;
int rcount;

//label buffer
struct ltype {
  char label[17];
  unsigned long int *address;
} *label;
int lcount;

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
    if (c == '\n' || c == 10 || c == 13) {
      if (!new) {
	c = END;
	fwrite(&c, 1, 1, tfd);
	new = 1;
      }
    }
    else if (c == ';') {
      c = FEND;
      fwrite(&c, 1, 1, tfd);
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
  c = END;
  if (!new)
    fwrite(&c, 1, 1, tfd);
  
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

#define COMP(str, symb)				\
  if (compare(shift, str)) {			\
    comm[0] = symb;				\
    if (shift[SLAST] != EOF) {			\
      comm[1] = shift[SLAST];			\
      return 2;					\
    }						\
    else					\
      return 1;					\
  }

int find(char* shift, char* comm) {
  COMP("int", INT)
  COMP("for", FOR)
  COMP("goto", GOTO)
  COMP("if", IF)
  COMP("else", ELSE)
  COMP("adr", ADR)
  COMP("out", OUT)
  COMP("in", IN)
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
  shift[SLAST+1] = '\0'; //diagnostic purposes; allows strings to be printf-ed
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

#define CHECK(ch0, ch1, symb)				\
  if (c == ch0) {					\
    if (fread(&c, 1, 1, sfd)) {				\
      if (c == ch1) {					\
	c = symb;					\
	fwrite(&c, 1, 1, tfd);				\
      }							\
      else {						\
	buff[1] = c;					\
	fwrite(buff, 1, 2, tfd);			\
      }							\
    }							\
    else {						\
      fwrite(buff, 1, 1, tfd);				\
    }							\
    continue;						\
  }

void secondtoken(char* s, char* t) {
  FILE * sfd = fopen(s, "rb");
  FILE * tfd = fopen(t, "wb");
  char c;
  char buff[2];

  while (fread(&c, 1, 1, sfd)) {
    buff[0] = c;
    CHECK('<', '<', SU)
    CHECK('>', '>', SD)
    CHECK('<', '=', ELESS)
    CHECK('>', '=', EMORE)
    CHECK('=', '=', EQL)
    CHECK('!', '=', NEQL)
    CHECK('&', '&', LAND)
    CHECK('|', '|', LOR)
    CHECK('^', '^', LXOR)
    fwrite(&c, 1, 1, tfd);
  }

  fclose(sfd);
  fclose(tfd);
}

int groupdata(char* s, char* t) {
  FILE * sfd = fopen(s, "rb");
  FILE * tfd = fopen(t, "wb");
  char c;
  char buff[3];
  int notend, close;

  fread(&c, 1, 1, sfd);
  while(1)
    if (c >= '0' && c <= '9') {
      buff[2] = c;
      buff[1] = BS;
      buff[0] = NUM;
      fwrite(buff, 1, 3, tfd);
      while ((notend = fread(&c, 1, 1, sfd)) != 0 &&
	     ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
	fwrite(&c, 1, 1, tfd);
      buff[0] = ES;
      fwrite(buff, 1, 1, tfd);
      if (!notend)
        break;
    }
    else if (c >= 'a' && c <= 'z') {
      buff[2] = c;
      buff[1] = BS;
      buff[0] = NAME;
      fwrite(buff, 1, 3, tfd);
      close = 1;
      while ((notend = fread(&c, 1, 1, sfd)) != 0 &&
	     ((c >= 'a' && c <= 'z') || c == '[')) {
	fwrite(&c, 1, 1, tfd);
	if (c == '[') {
	  close = 0;
	  notend = fread(&c, 1, 1, sfd);
	  break;
	}
      }
      if (close) {
	buff[0] = ES;
	fwrite(buff, 1, 1, tfd);
      }
      if (!notend)
	break;
    }
    else if (c == ']') {
      fwrite(&c, 1, 1, tfd);
      close = 1;
      while ((notend = fread(&c, 1, 1, sfd)) != 0 &&
	     ((c >= 'a' && c <= 'z') || c == '[')) {
	fwrite(&c, 1, 1, tfd);
	if (c == '[') {
	  close = 0;
	  notend = fread(&c, 1, 1, sfd);
	  break;
	}
      }
      if (close) {
	buff[0] = ES;
	fwrite(buff, 1, 1, tfd);
      }
      if (!notend)
	break;
    }
    else {
      fwrite(&c, 1, 1, tfd);
      notend = fread(&c, 1, 1, sfd);
      if (!notend)
	break;
    }
  
  fclose(sfd);
  fclose(tfd);
}

void bracket(char* s, char* t) {
  FILE * sfd = fopen(s, "rb");
  FILE * tfd = fopen(t, "wb");
  int ctab;
  int ptab;
  int i;
  _Bool place;
  char c;
  char buff;
  
  for (ptab = ctab = place = 0; fread(&c, 1, 1, sfd);)
    if (c == END) {
      place = 1;
      ptab = ctab;
      ctab = 0;
      fwrite(&c, 1, 1, tfd);
    }
    else if (c == TAB)
      ctab++;
    else if (place) {
      place = 0;
      if (ctab > ptab) {
	i = ctab;
	buff = CBS;
	while (i > ptab) {
	  fwrite(&buff, 1, 1, tfd);
	  i--;
	}
	fwrite(&c, 1, 1, tfd);
      }
      else if (ctab < ptab) {
	i = ctab;
	buff = CBE;
	while (i < ptab) {
	  fwrite(&buff, 1, 1, tfd);
	  i++;
	}
	fwrite(&c, 1, 1, tfd);
      }
      else
	fwrite(&c, 1, 1, tfd);
    }
    else
      fwrite(&c, 1, 1, tfd);
  i = ptab;
  buff = CBE;
  while (i > 0) {
    fwrite(&buff, 1, 1, tfd);
    i--;
  }

  fclose(tfd);
  fclose(sfd);
}

int strprocess(char* str, FILE* stream) {
  return 0;
}

int strgrab(char* str, FILE* stream) {
  int i;
  char c;
  _Bool notend;
  
  while (fread(&c, 1, 1, stream))
    if (c == NUM || c == NAME)
      break;
  fseek(stream, 1, SEEK_CUR);
  fread(&c, 1, 1, stream);
  for (i = 0, notend = fread(&c, 1, 1, stream);
       notend && i < 16; i++, notend = fread(&c, 1, 1, stream)) {
    if (c == ES) {
      str[i] = '\0';
      return ES;
    }
    else if (c == SBS) {
      str[i] = '\0';
      return SBS;
    }
    str[i] = c;
  }
  str[i] = '\0';
  if (c == ES)
    return ES;
  else
    return SBS;
}

void macrobuffer(char* s, char* t) {
  FILE* sfd = fopen(s, "rb");
  FILE* tfd = fopen(t, "wb");
  char c;
  int i, j, depth;
  _Bool notend;

  for (macro = NULL, mcount = 0; fread(&c, 1, 1, sfd);)
    if (c == DEF) {
      if (macro == NULL)
	macro = malloc(sizeof(struct mtype));
      else
	macro = realloc(macro, sizeof(struct mtype)*(mcount+1));
      strgrab(macro[mcount].label, sfd);
      while (c != END)
	fread(&c, 1, 1, sfd);
      for (i = depth = 0, notend = fread(&c, 1, 1, sfd); (depth || !i) &&
	     notend; notend = fread(&c, 1, 1, sfd), i++)
        if (c == CBS)
	  depth++;
	else if (c == CBE)
	  depth--;
      fseek(sfd, -i, SEEK_CUR);
      macro[mcount].body = (char*) malloc(i+1);
      for (j = 0, fread(&c, 1, 1, sfd); i > 0; i--, j++, fread(&c, 1, 1, sfd))
	macro[mcount].body[j] = c;
      macro[mcount].body[j] = TERM;
      mcount++;
    }
    else
      fwrite(&c, 1, 1, tfd);

  fclose(sfd);
  fclose(tfd);
}

unsigned long int hexstrlong(char* str) {
  int i;
  unsigned long int ret;

  for (i = ret = 0; str[i] != '\0' && i < 16; ret<<4, i++)
    if (str[i] >= '0' && str[i] <= '9')
      ret += str[i]-'0';
    else
      ret += str[i]-'a'+10;
  return ret;
}

void variable(char* s, char* t) {
  FILE* sfd = fopen(s, "rb");
  FILE* tfd = fopen(t, "wb");
  char c;
  char buff[17];
  unsigned long int val;
  unsigned long int total;
  int i;
  int cont;

  for (ram = NULL, rcount = 0; fread(&c, 1, 1, sfd);)
    if (c == INT) {
      if (ram == NULL)
	ram = malloc(sizeof(struct rtype));
      else
	ram = realloc(ram, sizeof(struct rtype)*(rcount+1));
      cont = strgrab(ram[rcount].label, sfd);
      for (i = 0, total = 1, ram[rcount].dimension = NULL; cont == SBS; i++) {
	cont = strgrab(buff, sfd);
	val = hexstrlong(buff);
	if (ram[rcount].dimension == NULL)
	  ram[rcount].dimension = malloc(sizeof(unsigned long int));
	else
	  ram[rcount].dimension = realloc(ram[rcount].dimension,
					  sizeof(unsigned long int)*(i+1));
	ram[rcount].dimension[i] = val;
	total *= val;
      }
      ram[rcount].value = malloc(total*sizeof(unsigned long int));
      rcount++;
      while (c != END)
	fread(&c, 1, 1, sfd);
    }
    else
      fwrite(&c, 1, 1, tfd);
      
  fclose(sfd);
  fclose(tfd);
}

void freemacrosvariables() {
  int i;

  for (i = 0; i < mcount; i++)
    free(macro[i].body);
  free(macro);
  for (i = 0; i < rcount; i++) {
    free(ram[i].dimension);
    free(ram[i].value);
  }
  free(ram);
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
  secondtoken(b, t);
  groupdata(t, b);
  bracket(b, t);
  variable(t, b);
  macrobuffer(b, t);
  /*insertevaluate(t, b);
  translateinc(b, t);
  programpoint(t, b);*/
  freemacrosvariables();
  
  return 0;
}
