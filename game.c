#include "game.h"

void exit_gracefully(int code){
  printf("Bye Bye.\n");
  exit(code);
}

int read_number(char *buffer, int *num) {
  *num = 0;
  char *c = buffer;
  while (*c >= '0' && *c <= '9') {
    *num = *num * 10 + (*c - '0');
    c++;
  }
  return c - buffer;
}


int _atoi(char *start, char *end){
  int num = 0;
  while (*start >= '0' && *start <= '9' && start<=end) {
    num = num * 10 + (*start - '0');
    start++;
  }
  return num;
}


int read_node(char *buffer, int *point, int buff_len, Node *n) {
  char *c, *end;
  c = buffer + *point;
  end = buffer + buff_len - 1;
  int skip_flag=0;
  while (1) {
    if (c > end)
      return 0;
    switch (*c) {
    case ' ':
    case '\n':
    case '\t':
    case '\r':
      c++;
      break;
    case '#':
      if (!skip_flag) while (*c++ != '\n')
			;
      break;
    case '\\':
      if (!skip_flag){
	skip_flag = 1;
        continue;
      }
      break;
    case '0' ... '9':
      c += read_number(c, &n->index);
      if (*c == '-') {
        n->text_start = c + 1;
	int action_flag = 0, skip_flag=0, loop_flag=1;
        while (loop_flag){
	  switch(*(++c)){
	  case '\\':
	    if (!skip_flag){
	      skip_flag = 1;
	      continue;
	    }
	    break;
	  case '(':
	    if (!skip_flag) action_flag=1;
	    break;
	  case '\n':
	    if (action_flag) loop_flag=0;
	    break;
	  }
	  skip_flag = 0;
	}
        n->text_end = c - 1;
        *point = c - buffer;
        return 1;
      }
    default:
      printf("ERR: Node entry incorrectly formatted.");
      while (*(++c) != '\n');
      print_between(buffer + *point, c);
      exit_gracefully(1);
    }
    skip_flag = 0;
  }
}

int read_action(char *buffer_start, int *point, char *buffer_end, Action *a) {
  char *buff_pt = buffer_start + *point;
  char end_char = '(';
  /* enum action_type action_flag; */
  while (*buff_pt == ' ' || *buff_pt == '\t' || *buff_pt == '\n')
    buff_pt++;
  switch (*buff_pt) {
  case '<':
    a->type = SCENE_ACTION_IMAGE;
    end_char = '>';
    break;
  case '[':
    a->type = SCENE_ACTION_DELAY;
    end_char = ']';
    break;
  case '{':
    a->type = SCENE_ACTION_INPUT;
    end_char = '}';
    break;
  case '(':
    a->type = SCENE_ACTION_UNDEFINED;
    end_char = ')';
    break;
  default:
    a->type = SCENE_ACTION_TEXT;
  }
  if (a->type == SCENE_ACTION_TEXT) {
    a->text_start = buff_pt;
    int skip_flag = 0;
    while (1) {
      switch (*buff_pt) {
      case '\\':
        skip_flag = 1;
        buff_pt++;
        break;
      case '<':
      case '{':
      case '[':
      case '(':
        if (skip_flag)
          skip_flag = 0;
        else {
          a->text_end = buff_pt - 1;
          *point = buff_pt - buffer_start;
          return 1;
        }
      default:
        buff_pt++;
        if (buff_pt > buffer_end) {
          a->text_end = buff_pt - 1;
          *point = buff_pt - buffer_start;
          return 1;
        }
        if (skip_flag) skip_flag = 0;
      }
    }
  } else if (a->type == SCENE_ACTION_UNDEFINED)
    return 0;
  else {
    a->text_start = buff_pt + 1;
    while (*(++buff_pt) != end_char)
      ;
    a->text_end = buff_pt++ - 1;
  }
  *point = buff_pt - buffer_start;
  return 1;
}

int read_choice(char *buffer_start, int *point, char *buffer_end, Choice *c){
  char *buff_pt = buffer_start + *point;
  if (*buff_pt != '('){
    return 0;
  }
  c->target = 0;
  while (*(++buff_pt) != ')'){
    c->target = c->target*10 + *buff_pt - '0';
  }
  c->text_start = ++buff_pt;
  if (buff_pt==buffer_end){
    c->text_end = buff_pt-1;
    *point = buff_pt - buffer_start;
    return 1;
  }
  int loop_flag=1, skip_flag=0;
  while(loop_flag){
    if (buff_pt >=buffer_end){
      c->text_end = buff_pt;
      break;
    }
    switch(*buff_pt){
    case ' ':
    case '\t':
      break;
    case '\\':
      if (!skip_flag) {
	skip_flag = 1;
	buff_pt++;
	continue;
      }
      break;
    case '#':
      if (!skip_flag) {
	c->text_end = --buff_pt;
	while( *(++buff_pt) != '\n')
	  ;
	loop_flag = 0;
      }
      break;
    case '(':
    case '\n':
      c->text_end = --buff_pt;
      loop_flag = 0;
      break;
    default:
      break;
    }
    skip_flag = 0;
    buff_pt++;
  }
  *point = buff_pt - buffer_start;
  return 1;
}

void node_to_event(Node *n, Event *e) {
  /* int len = n->text_end - n->text_start + 1; */
  int i = 0;
  /* char *c; */
  Action act;
  e->node = n->index;
  e->actions_count = 0;
  while (read_action(n->text_start, &i, n->text_end, &act)) {
    if (DEBUG) print_action(&act);
    e->actions_count += 1;
  }
  e->actions = malloc(e->actions_count * sizeof(Action));
  i = 0;
  int count;
  for (count=0; count < e->actions_count; count++){
    read_action(n->text_start, &i, n->text_end, e->actions+count);
  }
  e->choices_count = 0;
  Choice ch;
  int i2 = i;
  while (read_choice(n->text_start, &i, n->text_end, &ch)) {
    if (DEBUG) print_choice(&ch);
    e->choices_count += 1;
  }
  e->choices = malloc(e->choices_count * sizeof(Choice));
  i = i2;
  for (count=0; count < e->choices_count; count++){
    read_choice(n->text_start, &i, n->text_end, e->choices+count);
  }
}

GameState* read_file(char *filename) {
  InputContent *game_content = malloc(sizeof(InputContent));
  game_content->filename = filename;
  game_content->fp = fopen(filename, "r");
  getdelim(&game_content->filebuffer, &game_content->file_len, '\0',
           game_content->fp);
  fseek(game_content->fp, 0, SEEK_END);
  game_content->file_len = ftell(game_content->fp);
  game_content->node_len = 0;
  int point = 0;
  Node n;
  /* Event e; */
  if (DEBUG) printf("Parsing Nodes:\n");
  while (
      read_node(game_content->filebuffer, &point, game_content->file_len, &n)) {
    if (DEBUG) print_node(&n);
    game_content->node_len += 1;
  }
  point = 0;
  game_content->nodes = malloc(game_content->node_len * sizeof(Node));
  int i;
  GameState *gs = malloc(sizeof(GameState));
  gs->all_events = malloc(game_content->node_len * sizeof(Event));
  for (i = 0; i < game_content->node_len; i++) {
    read_node(game_content->filebuffer, &point, game_content->file_len,
              game_content->nodes + i);
    node_to_event(game_content->nodes + i, gs->all_events+i);
  }
  free(game_content->nodes);
  fclose(game_content->fp);
  free(game_content);
  gs->current_event = gs->all_events;
  gs->events_count = game_content->node_len;
  return gs;
}


void print_between(char *start, char *end) {
  if (end == NULL || start > end){
    printf("\111[31mNULL\111[0m");
    return;
  }
  int whitespace_flag=0;
  while (start <= end) {
    if (*start == '\\'){
      start++;
      continue;
    }
    switch (*start){
    case '\n':
    case ' ':
    case '\t':
      if (whitespace_flag) break;
      whitespace_flag = 1;
      putc(' ', stdout);
      break;
    default:
      putc(*start, stdout);
      whitespace_flag = 0;
    }
    start++;
  }
}

void print_node(Node *n) {
  printf("%d :", n->index);
  print_between(n->text_start, n->text_end);
  printf("\n");
}

void print_action(Action *a) {
  printf("ACTION: ");
  switch (a->type) {
  case SCENE_ACTION_UNDEFINED:
    printf("UNDEFINED");
    break;
  case SCENE_ACTION_TEXT:
    printf("TEXT");
    break;
  case SCENE_ACTION_IMAGE:
    printf("IMAGE");
    break;
  case SCENE_ACTION_INPUT:
    printf("INPUT");
    break;
  case SCENE_ACTION_DELAY:
    printf("DELAY");
    break;
  case SCENE_ACTION_AUDIO:
    printf("AUDIO");
    break;
  }
  printf(" value=");
  print_between(a->text_start, a->text_end);
  printf("\n");
}

void print_choice(Choice *c){
  printf("\tCHOICE: Target=%d Text=", c->target);
  print_between(c->text_start, c->text_end);
  printf("\n");
}
