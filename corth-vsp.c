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
  _Bool new;
  
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
  _Bool notend, close;

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
  long ctab, ptab, i;
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

int strgrab(char* str, FILE* stream) {
  long i;
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
  long i, j, depth;
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
      macro[mcount].body = (char*) malloc(sizeof(char)*(i+1));
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
  long i;
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

long evalexpr(char* buff) {
  printf("evalexpr\n>>> %s <<<", buff);
  return 0;
}

int macrograbline(long mac, char** buff, long* pos) {
  long i;
  char c;
  printf("macrograbline\n");
  for (i = 0, c = macro[mac].body[*pos+i]; c != TERM && c != END; ++i,
       c = macro[mac].body[*pos+i]) ;
  if ((*buff) == NULL)
    *buff = malloc(sizeof(char)*(i+2));
  else
    *buff = realloc((*buff), sizeof(char)*(i+2));
  if (i == 0)
    return 0;
  for (i = 0, c = macro[mac].body[*pos]; c != TERM && c != END; ++(*pos), ++i,
       c = macro[mac].body[*pos])
    (*buff)[i] = c;
  (*buff)[i] = END;
  (*buff)[i+1] = TERM;
  return 1;
}

void macrouse(long mac, FILE* sfd, FILE* tfd) {
  char* buff;
  long pos;
  void useline();
  printf("macrouse\n");
  for (pos = 0, buff = NULL; macrograbline(mac, &buff, &pos);)
    useline(buff, sfd, tfd);

  free(buff);
}

void forinit(char* buff) {
  long i, j;
  char* tinybuff;
  _Bool  valid;
  printf("forinit\n");
  for (valid = 1, i = 0, tinybuff = NULL; valid; i++) {
    for (; buff[i] != SEP && buff[i] != NAME && buff[i] != NUM &&
	   buff[i] != TERM && buff[i] != FEND; i++)
      if (buff[i] == PS)
	break;
    for (j = 0; buff[i] != FEND && buff[i] != TERM && buff[i] != SEP; i++, j++)
      ;
    printf("\ti = %d; j = %d\n", i, j);
    if (tinybuff == NULL)
      tinybuff = malloc(sizeof(char)*(j+1));
    else
      tinybuff = realloc(tinybuff, sizeof(char)*(j+1));
    for (i -= j, j = 0; buff[i] != FEND && buff[i] != TERM &&
	   buff[i] != SEP; i++, j++)
      tinybuff[j] = buff[i];
    tinybuff[j] = TERM;
    evalexpr(tinybuff);
    if (buff[i] != SEP)
      valid = 0;
  }

  free(tinybuff);
}

int forgrabline(char** buff, long* depth, FILE* sfd) {
  char c;
  long i;
  char* tinybuff;
  int grabline();
  printf("forgrabline\n");
  for (i = 1; fread(&c, 1, 1, sfd); i++)
    if (c != '}' && c != '{')
      break;
    else if (c == '}')
      (*depth)--;
    else
      (*depth)++;
  fseek(sfd, -i, SEEK_CUR);
  if (depth)
    if (grabline(tinybuff, sfd)) {
      for (i = 0; tinybuff[i] != TERM; i++)
	(*buff)[i] = tinybuff[i];
      (*buff)[i] = TERM;
      free(tinybuff);
      return 1;
    }
  free(tinybuff);
  return 0;
}

int foruse(char* buff, FILE* sfd, FILE* tfd) {
  char* tinybuff;
  long i, j, depth;
  _Bool valid;
  void useline();
  printf("foruse\n");
  for (i = 0; buff[i] != FEND && buff[i] != TERM; i++)
    ;
  for (i++; buff[i] != FEND && buff[i] != TERM && buff[i] != NUM &&
	 buff[i] != NAME && buff[i] != PS; i++)
    ;
  for (j = 0; buff[i] != FEND && buff[i] != TERM; j++, i++)
    ;
  tinybuff = malloc((sizeof(char)*(j+1));
  for (i -= j, j = 0; buff[i] != FEND && buff[i] != TERM; j++, i++)
    tinybuff[j] = buff[i];
  tinybuff[j] = TERM;
  if (!evalexpr(tinybuff)) {
    free(tinybuff);
    return 0;
  }
  depth = 0;
  while (forgrabline(&tinybuff, &depth, sfd))
    useline(tinybuff, sfd, tfd);
  for (valid = 1, i = 0, tinybuff = NULL; valid; i++) {
    for (; buff[i] != SEP && buff[i] != NAME && buff[i] != NUM &&
	   buff[i] != TERM && buff[i] != FEND; i++)
      if (buff[i] == PS)
	break;
    for (j = 0; buff[i] != FEND && buff[i] != TERM && buff[i] != SEP; i++, j++)
      ;
    if (tinybuff == NULL)
      tinybuff = malloc(sizeof(char)*(j+1));
    else
      tinybuff = realloc(tinybuff, sizeof(char)*(j+1));
    for (i -= j, j = 0; buff[i] != FEND && buff[i] != TERM &&
	   buff[i] != SEP; i++, j++)
      tinybuff[j] = buff[i];
    tinybuff[j] = TERM;
    evalexpr(tinybuff);
    if (buff[i] != SEP)
      valid = 0;
  }
  evalexpr(tinybuff);
  free(tinybuff);
  return 1;
}

void putval(long val, FILE* tfd) {
  long compare;
  char c;
  printf("putval\n");
  for (compare = 1; compare <= (val/16); compare *= 16)
    ;
  for (; compare >= 1; val = val-(val/compare)*compare, compare /= 16)
    if (val/compare <= 9) {
      c = val/compare+'0';
      fwrite(&c, 1, 1, tfd);
    }
    else {
      c = val/compare+'a'-10;
      fwrite(&c, 1, 1, tfd);
    }
}

int isvalue(char* buff) {
  long i, j, k;
  _Bool eql;
  char* tinybuff;
  
  for (i = 0, tinybuff = NULL; buff[i] != TERM; i++)
    if (buff[i] == NUM) {
      free(tinybuff);
      return 1;
    }
    else if (buff[i] == NAME) {
      for (i += 2, j = 0; buff[i] != ES; i++, j++)
	;
      if (tinybuff == NULL)
	tinybuff = malloc((sizeof(char)*(j+1));
      else
	tinybuff = realloc(tinybuff, sizeof(char)*(j+1));
      for (i -= j, j = 0; buff[i] != ES; i++, j++)
	tinybuff[j] = buff[i];
      tinybuff[j] = TERM;
      for (k = 0; k < rcount; k++) {
	for (j = 0, eql = 0; tinybuff[j] != TERM; j++)
	  if (tinybuff[j] != ram[k].label[j])
	    eql = 0;
	if (tinybuff[j] != ram[k].label[j])
	  eql = 0;
	if (eql) {
	  free(tinybuff);
	  return 1;
	}
      }
    }
  return 0;
}

void writeline(char* buff, FILE* tfd) {
  char c;
  long i, j;
  char* tinybuff;
  _Bool display;
  printf("writeline\n");
  for (i = 0, display = 0; buff[i] != TERM; i++)
    switch (buff[i]) {
    case ADR:
    case OUT:
    case IN:
    case LABEL:
      display = 1;
      break;
    }
  for (i = 0, tinybuff = NULL; buff[i] != TERM;) {
    for (; buff[i] != NAME && buff[i] != NUM && buff[i] != TERM; i++) {
      c = buff[i];
      fwrite(&c, 1, 1, tfd);
    }
    if (buff[i] == NAME || buff[i] == NUM || (buff[i] == PS &&
					      buff[i-1] != GOTO &&
					      buff[i-1] != IF &&
					      buff[i-1] != ELSE)) {
      for (j = 0; buff[i] != SBE && buff[i] != SEP && buff[i] != TERM &&
	     buff[i] != END; i++, j++)
	;
      tinybuff = malloc(sizeof(char)*(j+1));
      for (i -= j, j = 0; buff[i] != SBE && buff[i] != SEP && buff[i] != TERM &&
	     buff[i] != END; i++, j++) {
	tinybuff[j] = buff[i];
      }
      tinybuff[j] = TERM;
      if (display && isvalue(tinybuff))
	putval(evalexpr(tinybuff), tfd);
      else if (display && !isvalue(tinybuff))
	for (j = 0; tinybuff[j] != TERM; j++)
	  fwrite(&(tinybuff[j]), 1, 1, tfd);
    }
  }
  c = '\n';///////////////////
  fwrite(&c, 1, 1, tfd);///////////////////
  free(tinybuff);
}

void useline(char* linebuff, FILE* sfd, FILE* tfd) {
  char* strbuff;
  long i, j, k;
  _Bool eql;
  printf("useline\n");
  for (i = 0, strbuff = NULL; 1; i++) {
    printf("useline.loop %s\n", linebuff);
    if (linebuff[i] == FOR) {
      printf("useline.loop.FOR\n");
      forinit(linebuff);
      while (foruse(linebuff, sfd, tfd)) ;
      break;
    }
    else if (linebuff[i] == NAME) {
      printf("useline.loop.NAME\n");
      for (i += 2, j = 0; linebuff[i] != ES; i++, j++)
	;
      printf("\tj = %d\n", j);
      if (strbuff == NULL)
	strbuff = malloc(sizeof(char)*(j+1));
      else
	strbuff = realloc(strbuff, sizeof(char)*(j+1));
      for (i -= j, j = 0; linebuff[i] != ES; i++, j++)
	strbuff[j] = linebuff[i];
      strbuff[j] = TERM;
      for (k = 0; k < mcount; k++) {
	for (j = 0, eql = 1; macro[k].label[j] != '\0'; j++)
	  if (macro[k].label[j] != strbuff[j])
	    eql = 0;
	if (macro[k].label[j] != strbuff[j])
	  eql = 0;
	if (eql)
	  break;
      }
      if (eql) {
	macrouse(k, sfd, tfd);
	break;
      }
    }
    else if (linebuff[i] == TERM) {
      writeline(linebuff, tfd);
      break;
    }
  }
  free(strbuff);
}
      
int grabline(char** buff, FILE* sfd) {
  long i;
  char c;
  _Bool notend;
  printf("grabline\n");
  for (i = 0, notend = fread(&c, 1, 1, sfd); notend && c != END;
       notend = fread(&c, 1, 1, sfd))
    i++;
  if ((*buff) == NULL)
    (*buff) = malloc(sizeof(char)*(i+2));
  else
    (*buff) = realloc((*buff), sizeof(char)*(i+2));
  if (!notend && i == 0)
    return 0;
  fseek(sfd, -(i+1), SEEK_CUR);
  for (i = 0, fread(&c, 1, 1, sfd); c != END; i++, fread(&c, 1, 1, sfd))
    (*buff)[i] = c;
  (*buff)[i] = END;
  (*buff)[i+1] = TERM;
  printf("%s\n", *buff);
  return 1;
}

void insertevaluate(char* s, char* t) {
  FILE* sfd = fopen(s, "rb");
  FILE* tfd = fopen(t, "wb");
  char* buff;
  printf("insertevaluate\n");
  buff = NULL;
  while (grabline(&buff, sfd)) {
    printf("%s\n", buff);
    useline(buff, sfd, tfd);
  }

  free(buff);
  fclose(sfd);
  fclose(tfd);
}
  
void freemacrosvariables() {
  long i;

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
  insertevaluate(t, b);
  /*translateinc(b, t);
  programpoint(t, b);*/
  freemacrosvariables();
  
  return 0;
}
