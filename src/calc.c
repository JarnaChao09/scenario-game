#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>

enum branch_eval {
  NONE_EVALUATED = 0,
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

void print_exp(struct expression *expr);
double evaluate(struct expression *expr, int clean){
  if (expr==NULL){
    return 0;
  }
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

int is_num(char *text, double *value) {
  *value = 0;
  int dec_flag=0;
  while (*text != '\0') {
    switch (*text) {
    case ' ':
    case '_':
    case '\t':
    case '\n':
      break;
    case '0' ... '9':
      if (dec_flag>0)
	*value += (*text - '0') * 1.0 / pow(10, dec_flag++);
      else
	*value = *value * 10 + (*text - '0');
      break;
    case '.':
      if (dec_flag){
	printf("Incorrect formatting of numbers.\n");
	exit(1);
      }
      dec_flag = 1;
      break;
      
    default:
      return 0;
    }
    text++;
  }
  return 1;
}

struct expression *new_expression(struct expression *parent){
  struct expression *e = malloc(sizeof(struct expression));
  e->branch_flags = BOTH_EVALUATED;
  e->parent = parent;
  e->left.value = 0;
  e->right.value = 0;
  e->operator = '+';
  return e;
}

int process_text(char *text, struct expression *exp, int paren_exit){
  if (*text == '\0'){
    return 0;
  }

  struct expression *temp_exp,
    *curr_exp = exp,
    *add_exp = exp;
  char *c = text;
  double num = 0;
  /* union branch temp_br; */
  int num_flag=0, dec_flag=0, loop_flag=1, add_flag=1, nest_flag=0, paren_flag=0;
  enum reading_mode rm=READING_LEFT_NUMBER;
  if (*c == '('){
    paren_flag = 1;
    c++;
  }
  while(loop_flag){
    switch(*c){
    case '(':
      /* temp_exp = new_expression(curr_exp); */
      /* c += process_text(c, temp_exp, 1); */
      /* print_exp(temp_exp); */
      if (num_flag){
	printf("num");
      }else{
	if (rm == READING_LEFT_NUMBER){
	  curr_exp->left.exp = new_expression(curr_exp);
	  curr_exp->branch_flags &= ~LEFT_EVALUATED;
	  c += process_text(c, curr_exp->left.exp, 1);
	  rm = READING_OPERATOR;
	}
      }
      break;
    case ')':
      if (!paren_flag){
	printf("Unmatched brackets.\n");
	return 0;
      }
    case '\0':
    case ';':
      if (rm == READING_LEFT_NUMBER){
	curr_exp->left.value = num;
	curr_exp->branch_flags = BOTH_EVALUATED;
	curr_exp->right.value = 0;
	curr_exp->operator = '+';
      } else {
	curr_exp->right.value = num;
	curr_exp->branch_flags |= RIGHT_EVALUATED;
      }
      if (!paren_exit && paren_flag){
	rm = READING_OPERATOR;
	break;
      }else{
	return c-text;
      }
    case ' ':
    case '_':
    case '\t':
    case '\n':
      break;
    case '0' ... '9':
      if (rm == READING_OPERATOR){
	curr_exp->operator = '*';
	rm = READING_RIGHT_NUMBER;
      }
      if (dec_flag>0)
	num += (*c - '0') * 1.0 / pow(10, dec_flag++);
      else
	num = num * 10 + (*c - '0');
      num_flag = 1;
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
      if (rm == READING_OPERATOR){
	break;
      }else if (rm == READING_LEFT_NUMBER) {
        curr_exp->left.value = num;
	curr_exp->branch_flags |= LEFT_EVALUATED;
      }else if(add_flag){
	temp_exp = new_expression(curr_exp);
	temp_exp->left = curr_exp->left;
	temp_exp->right.value = num;
	temp_exp->operator = curr_exp->operator;
	temp_exp->branch_flags = (curr_exp->branch_flags | RIGHT_EVALUATED);

        curr_exp->left.exp = temp_exp;
	curr_exp->branch_flags &= ~LEFT_EVALUATED;
      } else {
	curr_exp->right.value = num;
	curr_exp->branch_flags |= RIGHT_EVALUATED;
	if (nest_flag){
          temp_exp = new_expression(curr_exp);
          temp_exp->left = add_exp->left;
          temp_exp->right = add_exp->right;
          temp_exp->operator= add_exp->operator;
          temp_exp->branch_flags = add_exp->branch_flags;
	  add_exp->left.exp = temp_exp;
	  add_exp->branch_flags &= ~LEFT_EVALUATED;
          nest_flag=0;
	}
	curr_exp = add_exp;
      }
      curr_exp->operator = *c;
      rm = READING_RIGHT_NUMBER;
      add_flag = 1;
      num = 0;
      num_flag = 0;
      dec_flag = 0;
      break;
    case '*':
    case '/':
      if (rm == READING_OPERATOR){
	break;
      }else if (rm == READING_LEFT_NUMBER) {
        curr_exp->left.exp = new_expression(curr_exp);
	curr_exp->branch_flags &= ~LEFT_EVALUATED;
        curr_exp->left.exp->left.value = num;
        curr_exp->left.exp->operator= * c;
	add_exp = curr_exp;
	curr_exp = curr_exp->left.exp;
        rm = READING_RIGHT_NUMBER;
      } else if (!add_flag) {
	temp_exp = new_expression(curr_exp);
	temp_exp->left = curr_exp->left;
	temp_exp->right.value = num;
	temp_exp->operator = curr_exp->operator;
	temp_exp->branch_flags = (curr_exp->branch_flags | RIGHT_EVALUATED);

        curr_exp->left.exp = temp_exp;
	curr_exp->branch_flags &= ~LEFT_EVALUATED;
	curr_exp->operator = *c;
      } else {
        curr_exp->branch_flags &= ~RIGHT_EVALUATED;
        curr_exp->right.exp = new_expression(curr_exp);
        curr_exp->right.exp->left.value = num;
        curr_exp->right.exp->operator= * c;
	add_exp = curr_exp;
	curr_exp = curr_exp->right.exp;
        rm = READING_RIGHT_NUMBER;
	nest_flag = 1;
      }
      num = 0;
      num_flag = 0;
      dec_flag = 0;
      add_flag = 0;
      break;
    default:
      break;
    }
    c++;
  }
  return -1;
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
  double value;
  if (!is_num(text, &value)){
    br.exp = new_expression(NULL);
    process_text(text, br.exp, 0);
    print_exp(br.exp);
    printf("\n");
    value = evaluate(br.exp, 1);
  }
  return value;
}


int main(int argc, char *argv[])
{
  if (argc < 2){
    printf("%s Expression\n", argv[0]);
    return 0;
  }
  printf("EVAL: %f\n", eval_string(argv[1]));
  return 0;
}
