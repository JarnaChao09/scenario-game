#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define DEBUG 0

enum action_type {
  SCENE_ACTION_UNDEFINED,
  SCENE_ACTION_TEXT,
  SCENE_ACTION_IMAGE,
  SCENE_ACTION_INPUT,
  SCENE_ACTION_DELAY,
  SCENE_ACTION_AUDIO
};

enum event_type {
  SCENE_EVENT_SAVE,
  SCENE_EVENT_QUIT,
  /* SCENE_EVENT_ACTION, */
  /* SCENE_EVENT_CHOICE, */
  SCENE_EVENT_CONTINUE
};

typedef struct st_node {
  int index;
  char *text_start;
  char *text_end;
} Node;

typedef struct {
  char *text_start;
  char *text_end;
  int target;
} Choice;

typedef struct st_action {
  Node *node;
  char *text_start;
  char *text_end;
  int value; 			/* only useful for delay, union might be better? */
  enum action_type type;
  struct st_action *next;
  struct st_action *prev;
} Action;

typedef struct st_event {
  int node;
  enum event_type type;
  int actions_count;
  Action *actions;
  int choices_count;
  Choice *choices;
} Event;

typedef struct st_game {
  char *filename;
  FILE *fp;
  char *filebuffer;
  size_t file_len;
  Node *nodes;
  int node_len;
} InputContent;

typedef struct game_state {
  int events_count;
  Event *all_events;
  Event *current_event;
  Event **next_events;
  /* add others later */
} GameState;

int read_number(char *buffer, int *num);
GameState* read_file(char *filename);
int read_node(char *buffer, int *point, int buff_len, Node *n);
void print_node(Node *n);
void node_to_event(Node *n, Event *e);
void print_between(char *start, char *end);
void print_action(Action *a);
int read_action(char *buffer_start, int *point, char *buffer_end, Action *a);
int read_choice(char *buffer_start, int *point, char *buffer_end, Choice *c);

int read_number(char *buffer, int *num) {
  *num = 0;
  char *c = buffer;
  while (*c >= '0' && *c <= '9') {
    *num = *num * 10 + (*c - '0');
    c++;
  }
  return c - buffer;
}

void exit_gracefully(int code){
  printf("Bye Bye.\n");
  exit(code);
}

int _atoi(char *start, char *end){
  int num = 0;
  while (*start >= '0' && *start <= '9' && start<=end) {
    num = num * 10 + (*start - '0');
    start++;
  }
  return num;
}

int msleep(long msec){
    struct timespec ts;
    int res;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res);
    return res;
}

int read_node(char *buffer, int *point, int buff_len, Node *n) {
  char *c, *end;
  c = buffer + *point;
  end = buffer + buff_len - 1;
  /* int flag_loop=1; */
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
      while (*c++ != '\n')
        ;
      break;
    case '0' ... '9':
      c += read_number(c, &n->index);
      if (*c == '-') {
        n->text_start = c + 1;
        while (*(++c) != '\n')
          ;
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
  }
}

int read_action(char *buffer_start, int *point, char *buffer_end, Action *a) {
  char *buff_pt = buffer_start + *point;
  char end_char = '(';
  enum action_type action_flag;
  while (*buff_pt == ' ' || *buff_pt == '\t')
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
  int loop_flag=1;
  while(loop_flag){
    if (buff_pt >=buffer_end){
      c->text_end = buff_pt;
      break;
    }
    switch(*buff_pt){
    case ' ':
    case '\t':
      break;
    case '(':
    case '\n':
      c->text_end = --buff_pt;
      loop_flag = 0;
      break;
    default:
      break;
    }
    buff_pt++;
  }
  *point = buff_pt - buffer_start;
  return 1;
}

void print_between(char *start, char *end) {
  if (end == NULL || start>end){
    printf("\111[31mNULL\111[0m");
  }
  while (start <= end) {
    if (*start == '\\'){
      start++;
      continue;
    }
    putc(*start++, stdout);
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
  printf(" ");
  print_between(a->text_start, a->text_end);
  printf("\n");
}

void print_choice(Choice *c){
  printf("CHOICE: Target=%d Text=", c->target);
  print_between(c->text_start, c->text_end);
  printf("\n");
}

void node_to_event(Node *n, Event *e) {
  int len = n->text_end - n->text_start + 1;
  int i = 0;
  char *c;
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
  for (count=0; count < e->actions_count; count++){
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
  int point;
  Node n;
  Event e;
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

void execute_action(Action *a){
  switch (a->type){
  case SCENE_ACTION_DELAY:
    msleep(_atoi(a->text_start, a->text_end));
    break;
  case SCENE_ACTION_IMAGE:
    printf("img:");
  case SCENE_ACTION_AUDIO:
  case SCENE_ACTION_TEXT:
    print_between(a->text_start, a->text_end);
    printf("\n");
    break;
  default:
    printf("Unsupported action.");
  }
}

int execute_choice(Choice *c, int len){
  if (len == 1) return 0;
  int i, choice;
  for (i = 0; i < len; i++) {
    printf("\t %2d) ", i+1);
    print_between((c+i)->text_start, (c+i)->text_end);
    printf("\n");
  }
  scanf("%d", &choice);
  if (choice == 0) exit_gracefully(0);
  return choice-1;
}

int execute_event(Event *e){
  int i, j, choice;
  for (i=0; i < e->actions_count; i++){
    execute_action(e->actions+i);
  }
  choice = execute_choice(e->choices, e->choices_count);
  if ((e->choices + choice)->target==0) exit_gracefully(0);
  return (e->choices + choice)->target;
}

int execute_game(GameState *gs){
  int i, next;
  next = execute_event(gs->current_event);
  for (i=0; i < gs->events_count; i++){
    if ((gs->all_events + i)->node == next){
      gs->current_event = gs->all_events + i;
      return 1;
    }
  }
  printf("Node not found: %d", next);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }
  GameState *game;
  game = read_file(argv[1]);
  if (DEBUG) printf("Starting Game...\n"
		    "Nodes: %d\n", game->events_count);
  while(execute_game(game));
  return 0;
}
