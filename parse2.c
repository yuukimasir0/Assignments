#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getsym.h"

#define SYNTAX_ERROR "syntax error: "
#define DEBUG do{fprintf(stderr, "%s:  num[%d][%d] = %d\n", s, num_now, num_size[num_now] - 2, num[num_now][num_size[num_now] - 2]);}while(0)

extern TOKEN tok;
extern FILE *infile;
extern FILE *outfile;
char *va_table[1 << 10];
char *va_table_proc[1 << 10];
int va_table_size = 0;
int max_va_table_size = 0;
int va_table_size_proc = 0;
int label_num = 0;
int imm_num = 0;
char Large_im[1 << 16] = "halt";
int ident[1 << 10][1 << 10];
int num[1 << 10][1 << 10];
int num_size[1 << 10];
int num_now = 0;
int BR_num = 1;

void error(char *s, ...);
void statement(void);
void term(void);
void factor(void);
void make_va_table(void);
int serch_va_table(void);
int serch_va_table_proc(void);
void condition(int label_num_);
void culc(int key);
void exe_BR(int BR_num, char* s1, char* s2, ...);

void compiler(void) {
    init_getsym();
    getsym();
    if(tok.attr == RWORD && tok.value == PROGRAM) {
        getsym();
        if(tok.attr == IDENTIFIER) {
            getsym();
            if(tok.attr == SYMBOL && tok.value == SEMICOLON) {
                getsym();
                make_va_table();
                statement();
                if(tok.attr == SYMBOL && tok.value == PERIOD) {
                    fprintf(outfile, "%s", Large_im);
                    fprintf(stderr, "Parsing Completed. No errors found.\n");
                } else
                    error("%sAt the end, a period is required. line: %d", SYNTAX_ERROR, tok.sline);
            } else
                error("%sAfter program name, a semicolon is needed. line:  %d", SYNTAX_ERROR, tok.sline);
        } else
            error("%sProgram identifier is needed. line: %d", SYNTAX_ERROR, tok.sline);
    } else
        error("%sAt the first, program declaration is required. line: %d", SYNTAX_ERROR, tok.sline);
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
    if(tok.attr == RWORD && tok.value == VAR) {
        do {
            getsym();
            if(tok.attr == IDENTIFIER) {
                va_table_size++;
                va_table[va_table_size - 1] = realloc(va_table[va_table_size - 1], sizeof(tok.charvalue));
                strcpy(va_table[va_table_size - 1], tok.charvalue);
            } else
                error("1");
            getsym();
        } while(tok.attr == SYMBOL && tok.value == COMMA);
        if(tok.attr != SYMBOL || tok.value != SEMICOLON) error("2");
        getsym();
    }
    max_va_table_size = va_table_size;
}

int serch_va_table() {
    int i = -1;
    if(va_table_size_proc > 0) i = serch_va_table_proc();
    if(i != -1) return i;
    for(i = 0; i <= va_table_size; i++)
        if(strcmp(va_table[i], tok.charvalue) == 0) return i;
    error("%sUndeclared valiables\n", SYNTAX_ERROR);
}

int serch_va_table_proc() {
    int i = 0;
    for(; i <= va_table_size; i++)
        if(strcmp(va_table_proc[i], tok.charvalue) == 0) return i;
    return -1;
}

void exe_BR(int BR_num, char* s1, char* s2, ...) {
    if(BR_num < 4) {
        va_list args;
        va_start(args, s2);
        vfprintf(outfile, s1, args);
        va_end(args);
    } else {
        va_list args;
        va_start(args, s2);
        vfprintf(outfile, s2, args);
        va_end(args);
    }
}

void culc(int key) {
    char* s;
    switch(key) {
        case PLUS:
            s = "add";
            break;
        case MINUS:
            s = "sub";
            break;
        case TIMES:
            s = "mul";
            break;
        case DIV:
            s = "div";
            break;
        default:
            error("unexpected error huppend");
    }
    DEBUG;
    if(ident[num_now][num_size[num_now] - 2] == 1) {
        fprintf(outfile, "load r0,%d\n", num[num_now][num_size[num_now] - 2]);
        if(ident[num_now][num_size[num_now] - 1] == 1) {
            exe_BR(BR_num, "%s r0,%d\nloadr r%d,r0\n", "%s r0,%d\nstore r0,%d\n", s, num[num_now][--num_size[num_now]], BR_num);
            ident[num_now][num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = BR_num++;
        } else if(ident[num_now][num_size[num_now] - 1] == 0) {
            int imm = num[num_now][--num_size[num_now]];
            int reg = BR_num++;
            if(imm < 1 << 16) {
                exe_BR(reg, "%si r0,%d\nloadr r%d,r0\n", "%si r0,%d\nstore r0,%d\n", s, imm, reg);
            } else {
                exe_BR(reg, "%s r0,IMM%d\nloadr r%d,r0\n", "%s r0,IMM%d\nstore r0,%d\n", s, imm_num, reg);
                sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, imm);
            }
            ident[num_now][num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = reg;
        } else {
            exe_BR(BR_num - 1, "%sr r%d,r0\n", "%s r0,%d\nstore r0,%d\n", s, BR_num - 1, BR_num - 1);
            ident[num_now][--num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = BR_num - 1;
        }
    } else if(ident[num_now][num_size[num_now] - 2] == 0) {
        if(ident[num_now][num_size[num_now] - 1] == 1) {
            if(num[num_now][num_size[num_now] - 2] < 1 << 16){
                fprintf(outfile, "loadi r0,%d\n", num[num_now][num_size[num_now] - 2]);
            } else {
                fprintf(outfile, "load r0,IMM%d\n", imm_num);
                sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, num[num_now][num_size[num_now] - 2]);
            }
            exe_BR(BR_num, "%s r0,%d\nloadr r%d,r0\n", "%s r0,%d\nstore r0,%d\n", s, num[num_now][num_size[num_now] - 1], BR_num);
            ident[num_now][num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = BR_num++;
        } else if(ident[num_now][num_size[num_now] - 1] == 0) {
            int imm = num[num_now][num_size[num_now] - 2];
            switch(key) {
                case PLUS:
                    imm += num[num_now][num_size[num_now] - 1];
                    break;
                case MINUS:
                    imm -= num[num_now][num_size[num_now] - 1];
                    break;
                case TIMES:
                    imm *= num[num_now][num_size[num_now] - 1];
                    break;
                case DIV:
                    imm /= num[num_now][num_size[num_now] - 1];
                    break;
            }
            int reg = BR_num++;
            if(imm < 1 << 16) {
                exe_BR(reg, "loadi r0,%d\nloadr r%d,r0\n", "loadi r0,%d\n store r0,%d\n", s, imm, reg);
            } else {
                exe_BR(reg, "load r0,IMM%d\nloadr r%d,r0\n", "load r0,IMM%d\n store r0,%d\n", s, imm, reg);
                sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, imm);
            }
            ident[num_now][num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = reg;
        } else {
            if(num[num_now][num_size[num_now] - 2] < 1 << 16){
                fprintf(outfile, "loadi r0,%d\n", num[num_now][num_size[num_now]-- - 2]);
            } else {
                fprintf(outfile, "load r0,IMM%d\n", imm_num);
                sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, num[num_now][num_size[num_now]-- - 2]);
            }
            exe_BR(BR_num - 1, "%sr r%d,r0\n", "%s r0,%d\nstore r0,%d\n", s, BR_num - 1, BR_num - 1);
            ident[num_now][num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = BR_num - 1;
        }
    } else {
        if(ident[num_now][num_size[num_now] - 1] == 1) {
            --BR_num;
            if(BR_num < 4){
                fprintf(outfile, "%s r%d,%d\n", s, BR_num, num[num_now][--num_size[num_now]]);
            } else {
                fprintf(outfile, "load r0,%d\n%s r0,%d\n", BR_num, s, num[num_now][--num_size[num_now]]);
            }            
            ident[num_now][num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = BR_num++;
        } else if(ident[num_now][num_size[num_now] - 1] == 0) {
            --BR_num;
            int imm = num[num_now][--num_size[num_now]];
            int reg = BR_num++;
            if(imm < 1 << 16) {
                if(BR_num < 4) {
                    fprintf(outfile, "%si r%d,%d\n", s, reg, imm);
                } else {
                    fprintf(outfile, "load r0,%d\n %si r0,%d\nstore r0,%d", reg, s, imm, reg);
                }
            } else {
                if(BR_num < 4) {
                    fprintf(outfile, "%s r%d,IMM%d\n", s, reg, imm_num);
                } else {
                    fprintf(outfile, "load r0,%d\n %s r0,IMM%d\nstore r0,%d\n", reg, s, imm_num, reg);
                }
                sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, imm);
            }
            ident[num_now][num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = reg;
        } else {
            BR_num--;
            if(BR_num == 4) {
                fprintf(outfile, "load r0,%d\n%sr r%d,r0\n", BR_num, s, BR_num - 1);
            } else if(BR_num < 4){
                fprintf(outfile, "%sr r%d,r%d\n", s, BR_num - 1, BR_num);
            } else {
                fprintf(outfile, "load r0,%d\n%s r0,%d\nstore r0,%d\n", BR_num - 1, s, BR_num, BR_num - 1);
            }
            ident[num_now][--num_size[num_now] - 1] = 2;
            num[num_now][num_size[num_now] - 1] = BR_num - 1;
        }
    }
}

void expression() {
    term();
    while(tok.attr == SYMBOL && (tok.value == PLUS || tok.value == MINUS)) {
        int key = tok.value;
        getsym();
        term();
        culc(key);
    }
    if(num_now == 0 && num_size[0] == 1) {
        if(ident[0][0] == 1) {
                fprintf(outfile, "load r0,%d\n", num[0][0]);
        } else if(ident[0][0] == 0) {
            if(num[0][0] < 1 << 16) {
                fprintf(outfile, "loadi r0,%d\n", num[0][0]);
            } else {
                fprintf(outfile, "load r0,IMM%d\n", imm_num);
                sprintf(Large_im, "%s\nIMM%d: data %d", Large_im, imm_num++, num[0][0]);
            }
        } else {
            --BR_num;
            exe_BR(BR_num, "loadr r0, r%d\n", "load r0, %d\n", BR_num);
        }
        num_size[0]--;
    }
}

void term() {
    factor();
    while((tok.attr == SYMBOL && tok.value == TIMES) || (tok.attr == RWORD && tok.value == DIV)) {
        int key = tok.value;
        getsym();
        factor();
        culc(key);      
    }
}

void factor() {
    switch(tok.attr) {
        case IDENTIFIER :
            ident[num_now][num_size[num_now]] = 1;
            num[num_now][num_size[num_now]++] = serch_va_table();
            getsym();
            return;
        case NUMBER :
            ident[num_now][num_size[num_now]] = 0;
            num[num_now][num_size[num_now]++] = tok.value;
            getsym();
            return;
        case SYMBOL :
            if(tok.value == LPAREN) {
                num_now++;
                getsym();
                expression();
                if(tok.value == RPAREN) {
                    ident[num_now - 1][num_size[num_now - 1]] = 2;
                    num[num_now - 1][num_size[num_now - 1]++] = num[num_now][--num_size[num_now]];
                    num_now--;
                    // fprintf(stderr, "%d: ", num_now);
                    // for(int i = 0; i < num_size[num_now]; i++) {
                    //     fprintf(stderr, "(%d, %d)", ident[num_now][i], num[num_now][i]);
                    // }
                    // fprintf(stderr, "\n");
                    getsym();
                } else error("10");
            } else if(tok.value == MINUS) {
                getsym();
                if(tok.attr == NUMBER) {
                    ident[num_now][num_size[num_now]] = 0;
                    num[num_now][num_size[num_now]++] = -tok.value;
                    getsym();
                    return;
                }
                ident[num_now][num_size[num_now]] = 0;
                num[num_now][num_size[num_now]++] = -1;
                if(tok.attr == SYMBOL && tok.value == LPAREN) {
                    num_now++;
                    getsym();
                    expression();
                    if(tok.value == RPAREN) {
                        ident[num_now - 1][num_size[num_now - 1]] = 2;
                        num[num_now - 1][num_size[num_now - 1]++] = num[num_now][--num_size[num_now]];
                        num_now--;
                        getsym();
                    } else error("10");
                } else if(tok.attr == IDENTIFIER) {
                    ident[num_now][num_size[num_now]] = 1;
                    num[num_now][num_size[num_now]++] = serch_va_table();
                }
                culc(TIMES);
                getsym();
                return;
            }
    }
}

void statement(void) {
    if(tok.attr == IDENTIFIER) {
        int index = serch_va_table();
        getsym();
        if(tok.attr == SYMBOL && tok.value == BECOMES) {
            getsym();
            expression();
        } else
            error("ghajsihgvua");
        fprintf(outfile, "store r0,%d\n", index);
    } else if(tok.attr == RWORD && tok.value == BEGIN) {
        do {
            getsym();
            if(tok.attr == RWORD && tok.value == BEGIN) {
                statement();
                if(tok.attr != SYMBOL || tok.value != SEMICOLON) {
                    error("%sExcept at the end, a semicolon is recuired. line: %d", SYNTAX_ERROR, tok.sline);
                }
                getsym();
            }
            statement();
        } while(tok.attr == SYMBOL && tok.value == SEMICOLON);
        if(tok.attr != RWORD || tok.value != END)
            error(
                "%sAt the end of the nesting process, \"end\" is recuired. line: %d",
                SYNTAX_ERROR,
                tok.sline);
        getsym();
    } else if(tok.attr == RWORD && tok.value == WRITE) {
        do {
            getsym();
            if(tok.attr == IDENTIFIER) {
              int reg = serch_va_table();
              fprintf(outfile, "load r0,%d\n", reg); // 出力
              fprintf(outfile, "writed r0\nloadi r0,'\\n'\nwritec r0\n");
            }
            getsym();
        } while(tok.attr == SYMBOL && tok.value == COMMA);
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
            getsym();
            statement();
        }
        fprintf(outfile, "L%d:\n", label_num_);
    } else if(tok.attr == RWORD && tok.value == WHILE) {
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
    exe_BR(BR_num, "loadr r%d, r0\n", "store r0,%d\n", BR_num);
    switch(tok.value) {
        case LESSTHAN :
            getsym();
            expression();
            exe_BR(BR_num, "cmpr r%d,r0\n", "cmp r0,%d\n", BR_num);
            fprintf(outfile, "jge L%d\n", label_num_);
            break;
        case LESSEQL :
            getsym();
            expression();
            exe_BR(BR_num, "cmpr r%d,r0\n", "cmp r0,%d\n", BR_num);
            fprintf(outfile, "jgt L%d\n", label_num_);
            break;
        case GRTRTHAN :
            getsym();
            expression();
            exe_BR(BR_num, "cmpr r%d,r0\n", "cmp r0,%d\n", BR_num);
            fprintf(outfile, "jle L%d\n", label_num_);
            break;
        case GRTREQL :
            getsym();
            expression();
            exe_BR(BR_num, "cmpr r%d,r0\n", "cmp r0,%d\n", BR_num);
            fprintf(outfile, "jlt L%d\n", label_num_);
            break;
        case EQL :
            getsym();
            expression();
            exe_BR(BR_num, "cmpr r%d,r0\n", "cmp r0,%d\n", BR_num);
            fprintf(outfile, "jnz L%d\n", label_num_);
            break;
        case NOTEQL :
            getsym();
            expression();
            exe_BR(BR_num, "cmpr r%d,r0\n", "cmp r0,%d\n", BR_num);
            fprintf(outfile, "jz L%d\n", label_num_);
            break;
        default :
            break;
    }
}