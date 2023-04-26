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
char Large_im[1 << 16] = "halt\n";

void error(char *s, ...);
void statement(void);
void make_va_table(void);
int serch_va_table(void);
void condition(int label_num_);

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
          fprintf(outfile, "%s", Large_im);
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

int serch_va_table() {
  int i = 0;
  for (; i <= va_table_size; i++) if (strcmp(va_table[i], tok.charvalue) == 0) return i;
  error("1");
}

void expression() {
  // printf("%d\n", tok.attr);
  if (tok.attr == NUMBER) {
    if (tok.value >= 1 << 16) {
      sprintf(Large_im, "%s\nL%d: %d", Large_im, label_num, tok.value);
      fprintf(outfile, "load r0,L%d\n", label_num++);
    } else {
      fprintf(outfile, "loadi r0,%d\n", tok.value);
    }
  } else if (tok.attr == IDENTIFIER) {
    fprintf(outfile, "load r0,%d\n", serch_va_table());
  } else {
    error("%sExpected numeric constant before operator. line: %d", SYNTAX_ERROR,
          tok.sline);
  }
  getsym();
  if (tok.attr == SYMBOL){ if(tok.value != PLUS && tok.value != MINUS && tok.value != TIMES) return; } 
  else if (tok.attr != RWORD || tok.value != DIV) return;
  int tok_tmp = tok.value;  
  getsym();
  if (tok.attr == NUMBER) {
    fprintf(outfile, "loadi r1,%d\n", tok.value);
  } else if (tok.attr == IDENTIFIER) {
    fprintf(outfile, "load r1,%d\n", serch_va_table());
  } else
    error("%sExpected numeric constant after operator. line: ", SYNTAX_ERROR,
          tok.sline);
  switch (tok_tmp) {
  case PLUS:
    fprintf(outfile, "addr r0,r1\n");
    break;
  case MINUS:
    fprintf(outfile, "subr r0,r1\n");
    break;
  case TIMES:
    fprintf(outfile, "mulr r0,r1\n");
    break;
  case DIV:
    fprintf(outfile, "divr r0,r1\n");
    break;
  default:
    break;
  }
  getsym();
}

void statement(void) {
  if (tok.attr == IDENTIFIER) {
    int index = serch_va_table();
    getsym();
    if (tok.attr == SYMBOL && tok.value == BECOMES) {
      getsym();
      expression();
    } else
      error("ghajsihgvua");
    fprintf(outfile, "store r0,%d\n", index);
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
      if (tok.attr == IDENTIFIER) 
        fprintf(outfile, "load r0,%d\nwrited r0\nloadi r1,'\\n'\nwritec r1\n", serch_va_table());  // 出力
      getsym();
    } while (tok.attr == SYMBOL && tok.value == COMMA);
  } else if(tok.attr == RWORD && tok.value == IF) {
    int label_num_ = label_num;
    label_num += 2;
    condition(label_num_);
    // fprintf(stderr, "%d %d %s\n", tok.attr, tok.value, tok.charvalue);
    if(tok.attr != RWORD || tok.value != THEN) error("4");
    getsym();
    statement();
    fprintf(outfile, "jmp L%d\n", label_num_ + 1);
    fprintf(outfile, "L%d:\n", label_num_++);
    if(tok.attr == RWORD && tok.value == ELSE) {
      getsym();
      statement();
    }
    fprintf(outfile, "L%d:\n", label_num_);
  } else if(tok.attr == RWORD && tok.value == WHILE){
    int label_num_ = label_num;
    label_num += 2;
    fprintf(outfile, "L%d:\n", label_num_ + 1);
    condition(label_num_);
    if(tok.attr != RWORD || tok.value != DO) error("5");
    getsym();
    // fprintf(stderr, "%d %d %s\n", tok.attr, tok.value, tok.charvalue);
    statement();
    fprintf(outfile, "jmp L%d\n", label_num_ + 1);
    fprintf(outfile, "L%d:\n", label_num_);
  } else {
    expression();
  }
}

void condition(int label_num_) {
  getsym();
  expression();
  fprintf(outfile, "loadr r1,r0\n");
  switch (tok.value) {
    case LESSTHAN:   
      getsym();
      expression();
      fprintf(outfile, "cmpr r1,r0\n");
      fprintf(outfile, "jge L%d\n", label_num_);
      break;
    case LESSEQL:
      getsym();
      expression();
      fprintf(outfile, "cmpr r1,r0\n");
      fprintf(outfile, "jgt L%d\n", label_num_);
      break;
    case GRTRTHAN:
      getsym();
      expression();
      fprintf(outfile, "cmpr r1,r0\n");
      fprintf(outfile, "jle L%d\n", label_num_);
      break;
    case GRTREQL:
      getsym();
      expression();
      fprintf(outfile, "cmpr r1,r0\n");
      fprintf(outfile, "jlt L%d\n", label_num_);
      break;
    case EQL:
      getsym();
      expression();
      fprintf(outfile, "cmpr r1,r0\n");
      fprintf(outfile, "jnz L%d\n", label_num_);
      break;
    case NOTEQL:
      getsym();
      expression();
      fprintf(outfile, "cmpr r1,r0\n");
      fprintf(outfile, "jz L%d\n", label_num_);
      break;
    default:
      break;
  }
}
