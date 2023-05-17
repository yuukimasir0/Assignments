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
char *va_table_proc[1 << 10];
int va_table_size = 0;
int va_table_size_proc = 0;
int label_num = 0;
int imm_num = 0;
char Large_im[1 << 16] = "halt";
int ident[1 << 10][1 << 10];
int num[1 << 10][1 << 10];
int num_size[1 << 10];
int num_now = 0;
int BR_num;

void error(char *s, ...);
void statement(void);
void term();
void factor();
void make_va_table(void);
int serch_va_table(void);
int serch_va_table_proc(void);
void condition(int label_num_);
void culc(char*);

typedef struct {
  int ope;
  int reg;
} beforeope;

beforeope bope;

void compiler(void) {
  init_getsym();
  getsym();
  bope.ope = 0;

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
  BR_num = va_table_size;
}

int serch_va_table() {
  int i = -1;
  if(va_table_size_proc > 0) i = serch_va_table_proc();
  if(i != -1) return i;
  for (i = 0; i <= va_table_size; i++) if (strcmp(va_table[i], tok.charvalue) == 0) return i;
  error("%sUndeclared valiables\n", SYNTAX_ERROR);
}

int serch_va_table_proc() {
  int i = 0;
  for (; i <= va_table_size; i++) if (strcmp(va_table_proc[i], tok.charvalue) == 0) return i;
  return -1;
}

void culc(char* s){
  if(ident[num_now][num_size[num_now] - 2] == 1) {
    if(bope.ope != 1 || bope.reg != num[num_now][num_size[num_now] - 2]){
      fprintf(outfile, "load r0,%d\n", num[num_now][num_size[num_now] - 2]);
      bope.ope = 0;
    }
    // fprintf(stderr, "%s:  num[%d][%d] = %d\n", s, num_now, num_size[num_now] - 2, num[num_now][num_size[num_now] - 2]);      //stderrrrrrr
    if(ident[num_now][num_size[num_now] - 1] == 1) {
      fprintf(outfile, "%s r0,%d\nstore r0,%d\n", s, num[num_now][num_size[num_now]], BR_num);
      bope.ope = 1;
      
      bope.reg = BR_num;
      ident[num_now][num_size[num_now]] = 2;
      num[num_now][num_size[num_now]++] = BR_num++;
    } else if(ident[num_now][num_size[num_now] - 1] == 0) {
      int imm = num[num_now][--num_size[num_now]];
      int reg = BR_num++;
      if(imm < 1 << 16){
        fprintf(outfile, "%si r0,%d\nstore r0,%d\n", s, imm, reg);
        bope.ope = 1;
        
        bope.reg = reg;
      } else {
        fprintf(outfile, "%s r0,IMM%d\nstore r0,%d\n", s, imm_num, reg);
        bope.ope = 1;
        
        bope.reg = reg;
        sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, imm);
      }
      ident[num_now][num_size[num_now]] = 2;
      num[num_now][num_size[num_now]++] = reg;
    } else {
      fprintf(outfile, "%s r0,%d\nstore r0,%d\n", s, BR_num - 1, BR_num - 1);
      bope.ope = 1;
      
      bope.reg = BR_num - 1;
    }
    ident[num_now][--num_size[num_now] - 1] = 2;
  }
  
  else if(ident[num_now][num_size[num_now] - 2] == 0) {
    fprintf(outfile, "loadi r0,%d\n", num[num_now][num_size[num_now] - 2]);
    bope.ope = 0;
    // fprintf(stderr, "%s:  num[%d][%d] = %d\n", s, num_now, num_size[num_now] - 2, num[num_now][num_size[num_now] - 2]);      //stderrrrrrr
    if(ident[num_now][num_size[num_now] - 1] == 1) {
      fprintf(outfile, "%s r0,%d\nstore r0,%d\n", s, num[num_now][num_size[num_now] - 1], BR_num);
      bope.ope = 1;
      
      bope.reg = BR_num;
      ident[num_now][num_size[num_now]] = 2;
      num[num_now][num_size[num_now]++] = BR_num++;
    } else if(ident[num_now][num_size[num_now] - 1] == 0) {
      int imm = num[num_now][num_size[num_now] - 1];
      int reg = BR_num++;
      if(imm < 1 << 16){
        fprintf(outfile, "%si r0,%d\nstore r0,%d\n", s, imm, reg);
        bope.ope = 1;
        
        bope.reg = reg;
      } else {
        fprintf(outfile, "%s r0,IMM%d\nstore r0,%d\n", s, imm_num, reg);
        sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, imm);
        bope.ope = 1;
        
        bope.reg = reg;
      }
      ident[num_now][num_size[num_now]] = 2;
      num[num_now][num_size[num_now]++] = reg;
    } else {
      fprintf(outfile, "%s r0,%d\nstore r0,%d\n", s, BR_num - 1, BR_num - 1);
      bope.ope = 1;
      
      bope.reg = BR_num - 1;
    }
    ident[num_now][--num_size[num_now] - 1] = 2;
    } 
    
    
    else {
    if(ident[num_now][num_size[num_now] - 1] == 1) {
    if(bope.ope != 1 || bope.reg != --BR_num){
      fprintf(outfile, "load r0,%d\n", BR_num);
    }
      fprintf(outfile, "%s r0,%d\nstore r0,%d\n", s, num[num_now][--num_size[num_now]], BR_num);
      bope.ope = 1;
      
      bope.reg = BR_num;
      ident[num_now][num_size[num_now]] = 2;
      num[num_now][num_size[num_now]++] = BR_num++;
    } else if(ident[num_now][num_size[num_now] - 1] == 0) {
      if(bope.ope != 1 || bope.reg != --BR_num){
        fprintf(outfile, "load r0,%d\n", BR_num);
        bope.ope = 0;
      }
      int imm = num[num_now][--num_size[num_now]];
      int reg = BR_num++;
      if(imm < 1 << 16){
        fprintf(outfile, "%si r0,%d\nstore r0,%d\n", s, imm, reg);
        bope.ope = 1;
        
        bope.reg = reg;
      } else {
        fprintf(outfile, "%s r0,IMM%d\nstore r0,%d\n", s, imm_num, reg);
        sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, imm);
        bope.ope = 1;
        
        bope.reg = reg;
      }
      ident[num_now][num_size[num_now]] = 2;
      num[num_now][num_size[num_now]++] = reg;
    } else {
      if(bope.ope != 1 || bope.reg != BR_num - 2){
        fprintf(outfile, "load r0,%d\n", BR_num - 2);
        bope.ope = 0;
      }
      BR_num--;
      fprintf(outfile, "%s r0,%d\nstore r0,%d\n", s, BR_num, BR_num - 1);
      bope.ope = 1;
      
      bope.reg = BR_num - 1;
    }
    ident[num_now][--num_size[num_now] - 1] = 2;
  }
}

void expression() {
  term();
  while(tok.attr == SYMBOL && (tok.value == PLUS || tok.value == MINUS)) {
    switch(tok.value){
      case PLUS:
        getsym();
        term();
        culc("add");
        break;
      case MINUS:
        getsym();
        term();
        culc("sub");
        break;
    }  
  }
  if(num_now == 0 && num_size[0] == 1) {
    if(ident[0][0] == 1) {
      if(bope.ope != 1 || bope.reg != num[0][0]){
        fprintf(outfile, "load r0,%d\n", num[0][0]);
        bope.ope = 0;
      }
    } else if(ident[0][0] == 0) {
      if(num[0][0] < 1 << 16) {
        fprintf(outfile, "loadi r0,%d\n", num[0][0]);
        bope.ope = 0;
      } else {
        fprintf(outfile, "load r0,IMM%d\n", imm_num);
        sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, num[0][0]);
        bope.ope = 0;
      }
    } else {
      if(bope.ope != 1 || bope.reg != --BR_num){
        fprintf(outfile, "load r0,%d\n", BR_num);
        bope.ope = 0;
      }
    }
    num_size[0]--;
  }
}

void term() {
  factor();
  while((tok.attr == SYMBOL && tok.value == TIMES) || (tok.attr == RWORD && tok.value == DIV)) {
    switch(tok.value){
      case TIMES:
        getsym();
        term();
        culc("mul");
        break;
      case DIV:
        getsym();
        term();
        culc("div");
        break;
    }  
  }
}

void factor(){
  switch (tok.attr){
    case IDENTIFIER:
      ident[num_now][num_size[num_now]] = 1;
      num[num_now][num_size[num_now]++] = serch_va_table();
      getsym();
      return;
    case NUMBER:
      ident[num_now][num_size[num_now]] = 0;
      num[num_now][num_size[num_now]++] = tok.value;
      getsym();
      return;
    case SYMBOL:
      if(tok.value == LPAREN) {
        num_now++;
        getsym();
        expression();
        if(tok.value == RPAREN) {
          ident[num_now - 1][num_size[num_now - 1]] = 2;
          num[num_now - 1][num_size[num_now - 1]++] = num[num_now][--num_size[num_now]];
          num_now--;
          getsym();
        } else error("10");
      } else if(tok.value == MINUS) {

      }
  }
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
    bope.ope = 1;
    
    bope.reg = index;
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
      if (tok.attr == IDENTIFIER) {
          fprintf(outfile, "load r0,%d\nwrited r0\nloadi r1,'\\n'\nwritec r1\n", serch_va_table());  // 出力
          bope.ope = 0;
      }
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
    if(tok.attr == RWORD && tok.value == ELSE) {
      fprintf(outfile, "jmp L%d\n", label_num_ + 1);
      fprintf(outfile, "L%d:\n", label_num_++);
      bope.ope = 0;
      getsym();
      statement();
    }
    fprintf(outfile, "L%d:\n", label_num_);
    bope.ope = 0;
  } else if(tok.attr == RWORD && tok.value == WHILE){
    int label_num_ = label_num;
    label_num += 2;
    fprintf(outfile, "L%d:\n", label_num_ + 1);
    bope.ope = 0;
    condition(label_num_);
    if(tok.attr != RWORD || tok.value != DO) error("5");
    getsym();
    // fprintf(stderr, "%d %d %s\n", tok.attr, tok.value, tok.charvalue);
    statement();
    fprintf(outfile, "jmp L%d\n", label_num_ + 1);
    fprintf(outfile, "L%d:\n", label_num_);
    bope.ope = 0;
  } else {
    expression();
  }
}

void condition(int label_num_) {
  getsym();
  expression();
  fprintf(outfile, "loadr r1,r0\n");
  bope.ope = 0;
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
  bope.ope = 0;
}
