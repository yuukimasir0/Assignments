#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getsym.h"

#define SYNTAX_ERROR "syntax error: "

extern TOKEN tok;
extern FILE *infile;
extern FILE *outfile;
char *va_table[1 << 10];
int va_table_size = 0;
int label_num = 0;

void error(char *s, ...);
void statement(void);
void make_va_table(void);

void compiler(void) {
  init_getsym();

  getsym();

  if (tok.attr == RWORD && tok.value == PROGRAM) {
    getsym();

    if (tok.attr == IDENTIFIER) {
      getsym();

      if (tok.attr == SYMBOL && tok.value == SEMICOLON) {
        getsym();
        make_va_table();
        statement();
        if (tok.attr == SYMBOL && tok.value == PERIOD) {
          fprintf(stderr, "Parsing Completed. No errors found.\n");
        } else
          error("%sAt the end, a period is required. line: %d", SYNTAX_ERROR,
                tok.sline);
      } else
        error("%sAfter program name, a semicolon is needed. line:  %d",
              SYNTAX_ERROR, tok.sline);
    } else
      error("%sProgram identifier is needed. line: %d", SYNTAX_ERROR,
            tok.sline);
  } else
    error("%sAt the first, program declaration is required. line: %d",
          SYNTAX_ERROR, tok.sline);
}

void error(char *s, ...) {
  va_list args;
  va_start(args, s);
  vfprintf(stderr, s, args);
  va_end(args);
  fprintf(stderr, "\n");
  exit(1);
}

//
// Parser
//

void make_va_table() {
  va_table_size = 0;
  if (tok.attr == RWORD && tok.value == VAR) {
    do {
      getsym();
      if (tok.attr == IDENTIFIER) {
        va_table_size++;
        va_table[va_table_size - 1] =
            realloc(va_table[va_table_size - 1], sizeof(tok.charvalue));
        strcpy(va_table[va_table_size - 1], tok.charvalue);
      } else
        error("1");
      getsym();
    } while (tok.attr == SYMBOL && tok.value == COMMA);
    if (tok.attr != SYMBOL || tok.value != SEMICOLON) error("2");
    getsym();
  }
}

void expression() {
  // printf("%d\n", tok.attr);
  char ope[1 << 10];
  if (tok.attr == NUMBER) {
    fprintf(outfile, "loadi r0,%d\n", tok.value);
  } else if (tok.attr == IDENTIFIER) {
    int i = 0;
    for (; i <= va_table_size; i++) {
      if (i == va_table_size) break;
      if (strcmp(va_table[i], tok.charvalue) == 0) break;
    }
    if (i == va_table_size) error("");
    fprintf(outfile, "load r0,%d\n", i);
  } else {
    error("%sExpected numeric constant before operator. line: %d", SYNTAX_ERROR,
          tok.sline);
  }
  getsym();
  if (tok.attr == SYMBOL) {
    if (tok.value == PLUS) {
      strcpy(ope, "addr r0,r1\n");
    } else if (tok.value == MINUS) {
      strcpy(ope, "subr r0,r1\n");
    } else if (tok.value == TIMES) {
      strcpy(ope, "mulr r0,r1\n");
    } else if (tok.value == SEMICOLON) {
      return;
    } else
      error("%sExpected operator before numeric constant. line: %d",
            SYNTAX_ERROR, tok.sline);
  } else if (tok.attr == RWORD && tok.value == DIV) {
    strcpy(ope, "divr r0,r1\n");
  } else
    return;

  getsym();
  if (tok.attr == NUMBER) {
    fprintf(outfile, "loadi r1,%d\n", tok.value);
  } else if (tok.attr == IDENTIFIER) {
    int i = 0;
    for (; i <= va_table_size; i++) {
      if (i == va_table_size) break;
      if (strcmp(va_table[i], tok.charvalue) == 0) break;
    }
    if (i == va_table_size) error("");
    fprintf(outfile, "load r1,%d\n", i);
  } else
    error("%sExpected numeric constant after operator. line: ", SYNTAX_ERROR,
          tok.sline);
  fprintf(outfile, "%s", ope);  // 出力
  getsym();
}

void statement(void) {
  // printf("%d %d %s\n", tok.attr, tok.value, tok.charvalue);
  if (tok.attr == IDENTIFIER) {
    int i = 0;
    for (; i <= va_table_size; i++) {
      if (i == va_table_size) break;
      if (strcmp(va_table[i], tok.charvalue) == 0) break;
    }
    if (i == va_table_size) error("");
    getsym();
    if (tok.attr == SYMBOL && tok.value == BECOMES) {
      getsym();
      expression();
    } else
      error("ghajsihgvua");
    fprintf(outfile, "store r0,%d\n", i);
  } else if (tok.attr == RWORD && tok.value == BEGIN) {
    do {
      getsym();
      if (tok.attr == RWORD && tok.value == BEGIN) {
        statement();
        if (tok.attr != SYMBOL || tok.value != SEMICOLON)
          error("%sExcept at the end, a semicolon is recuired. line: %d",
                SYNTAX_ERROR, tok.sline);
        getsym();
      }
      statement();
    } while (tok.attr == SYMBOL && tok.value == SEMICOLON);
    if (tok.attr != RWORD || tok.value != END)
      error(
          "%sAt the end of the nesting process, \"end\" is recuired. line: %d",
          SYNTAX_ERROR, tok.sline);
    getsym();
  } else if (tok.attr == RWORD && tok.value == WRITE) {
    do {
      getsym();
      int i = 0;
      for (; i < va_table_size; i++) {
        if (strcmp(va_table[i], tok.charvalue) == 0) break;
      }
      fprintf(outfile, "load r0,%d\nwrited r0\nloadi r1,'\\n'\nwritec r1\n",
              i);  // 出力
      getsym();
    } while (tok.attr == SYMBOL && tok.value == COMMA);
  } else {
    expression();
  }
}

void condition() {
  getsym();
  expression();
  fprintf(outfile, "loadr r1,r0");
  switch (tok.value) {
    case LESSTHAN:
      getsym();
      expression();
      fprintf(outfile, "compr r1,r0");
      getsym();
      if(tok.attr == RWORD && tok.value == THEN);
      break;
    case LESSEQL:
      getsym();
      expression();
      fprintf(outfile, "compr r1,r0");
      getsym();
      if(tok.attr == RWORD && tok.value == THEN);
      break;
    case GRTRTHAN:
      getsym();
      expression();
      fprintf(outfile, "compr r1,r0");
      getsym();
      if(tok.attr == RWORD && tok.value == THEN);
      break;
    case GRTREQL:
      getsym();
      expression();
      fprintf(outfile, "compr r1,r0");
      getsym();
      if(tok.attr == RWORD && tok.value == THEN);
      break;
    case EQL:
      getsym();
      expression();
      fprintf(outfile, "compr r1,r0");
      getsym();
      if(tok.attr == RWORD && tok.value == THEN);
      break;
    case NOTEQL:
      getsym();
      expression();
      fprintf(outfile, "compr r1,r0");
      getsym();
      if(tok.attr == RWORD && tok.value == THEN);
      break;
    default:
      break;
  }
}
