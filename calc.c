#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>

enum branch_eval {
  LEFT_EVALUATED = 1,
  RIGHT_EVALUATED = 2,
  BOTH_EVALUATED = 3
};

enum reading_mode {
  READING_LEFT_NUMBER,
  READING_RIGHT_NUMBER,
  READING_VARIABLE,
  READING_OPERATOR,
};

struct expression;

union branch {
  struct expression *exp;
  double value;
};

struct expression{
  char operator;
  enum branch_eval branch_flags;
  union branch right;
  union branch left;
  struct expression *parent;
};


double evaluate(struct expression *expr, int clean){
  if (!(expr->branch_flags & LEFT_EVALUATED)){
    expr->branch_flags |= LEFT_EVALUATED;
    expr->left.value = evaluate(expr->left.exp, clean);
  }
  if (!(expr->branch_flags & RIGHT_EVALUATED)){
    expr->branch_flags |= RIGHT_EVALUATED;
    expr->right.value = evaluate(expr->right.exp, clean);
  }
  double value;
  switch (expr->operator){
  case '+':
    value = expr->left.value + expr->right.value;
    break;
  case '-':
    value = expr->left.value - expr->right.value;
    break;
  case '*':
    value = expr->left.value * expr->right.value;
    break;
  case '/':
    value = expr->left.value / expr->right.value;
    break;
  default:
    exit(1);
  }
  if (clean) {
    free(expr);
  }
  return value;
}

int is_num(char *text) {
  while (*text != '\0') {
    switch (*text) {
    case ' ':
    case '_':
    case '\t':
    case '\n':
    case '.':
    case '0' ... '9':
      text++;
      continue;
    default:
      return 0;
    }
  }
  return 1;
}

int process_text(char *text, struct expression *exp){
  if (*text == '\0'){
    return 0;
  }

  struct expression *curr_exp = exp;
  char *c = text;
  double num = 0, *curr_num;
  int dec_flag=0, loop_flag=1;
  union branch *curr_branch = &exp->left;
  enum reading_mode rm=READING_LEFT_NUMBER;
  while(loop_flag){
    switch(*c){
    case '\0':
    case ';':
      if (rm == READING_LEFT_NUMBER) return 0;
      curr_exp->right.value = num;
      curr_exp->branch_flags |= RIGHT_EVALUATED;
      return 1;
    case ' ':
    case '_':
    case '\t':
    case '\n':
      break;
    case '0' ... '9':
      if (dec_flag>0)
	num += (*c - '0') * 1.0 / pow(10, dec_flag++);
      else
	num = num * 10 + (*c - '0');
      break;
    case '.':
      if (dec_flag){
	printf("Incorrect formatting of numbers.\n");
	exit(1);
      }
      dec_flag = 1;
      break;
    case '+':
    case '-':
    case '*':
    case '/':
      if (rm == READING_LEFT_NUMBER){
	curr_exp->left.value = num;
	curr_exp->branch_flags |= LEFT_EVALUATED;
	curr_exp->operator = *c;
	rm = READING_RIGHT_NUMBER;
	num = 0;
      }else{
	curr_exp->right.exp = malloc(sizeof(struct expression));
	curr_exp = curr_exp->right.exp;
	rm = READING_LEFT_NUMBER;
	continue;
      }
      break;
    default:
      break;
    }
    c++;
  }
  return 1;
}

void print_exp(struct expression *expr){
  if (expr==NULL) return;
  printf("(");
  if (expr->branch_flags & LEFT_EVALUATED){
    printf("%f", expr->left.value);
  }else{
    print_exp(expr->left.exp);
  }
  printf(" %c ", expr->operator);
  if (expr->branch_flags & RIGHT_EVALUATED){
    printf("%f", expr->right.value);
  }else{
    print_exp(expr->right.exp);
  }
  printf(")");
}

double eval_string(char *text){
  union branch br;
  printf("EXPR: ");
  if (is_num(text)){
    br.value = atof(text);
    printf("%f", br.value);
    return br.value;
  }else{
    br.exp = malloc(sizeof(struct expression));
    process_text(text, br.exp);
    print_exp(br.exp);
    return evaluate(br.exp, 1);
  }
}


int main(int argc, char *argv[])
{
  if (argc < 2){
    printf("%s Expression\n", argv[0]);
    return 0;
  }
  printf("\nEVAL: %f\n", eval_string(argv[1]));
  return 0;
}
