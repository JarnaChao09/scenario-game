#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

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
  enum action_type type;
  struct st_action *next;
  struct st_action *prev;
} Action;

typedef struct st_event {
  Node *node;
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

int read_number(char *buffer, int *num);
void read_file(InputContent *game_content);
int read_node(char *buffer, int *point, int buff_len, Node *n);
void print_node(Node *n);
void node_to_event(Node *n, Event *e);
void print_between(char *start, char *end);
void print_action(Action *a);

int read_number(char *buffer, int *num) {
  *num = 0;
  char *c = buffer;
  while (*c >= '0' && *c <= '9') {
    *num = *num * 10 + (*c - '0');
    c++;
  }
  return c - buffer;
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
      while (*(++c) != '\n')
        ;
      print_between(buffer + *point, c);
      exit(1);
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

void print_between(char *start, char *end) {
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
  putc('\n', stdout);
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
  putc('\n', stdout);
}

void node_to_event(Node *n, Event *e) {
  int len = n->text_end - n->text_start + 1;
  int i = 0;
  char *c;
  Action act;
  e->actions_count = 0;
  while (read_action(n->text_start, &i, n->text_end, &act)) {
    print_action(&act);
    e->actions_count += 1;
  }
  e->actions = malloc(e->actions_count * sizeof(Action));
  i = 0;
  int count;
  for (count=0;count<e->actions_count;count++){
    read_action(n->text_start, &i, n->text_end, e->actions+count);
  }
}

void read_file(InputContent *game_content) {
  game_content->fp = fopen(game_content->filename, "r");
  getdelim(&game_content->filebuffer, &game_content->file_len, '\0',
           game_content->fp);
  fseek(game_content->fp, 0, SEEK_END);
  game_content->file_len = ftell(game_content->fp);
  game_content->node_len = 0;
  int point;
  Node n;
  Event e;
  printf("Parsing Nodes:\n");
  while (
      read_node(game_content->filebuffer, &point, game_content->file_len, &n)) {
    print_node(&n);
    game_content->node_len += 1;
  }
  point = 0;
  game_content->nodes = malloc(game_content->node_len * sizeof(Node));
  int i;
  for (i = 0; i < game_content->node_len; i++) {
    read_node(game_content->filebuffer, &point, game_content->file_len,
              game_content->nodes + i);
    node_to_event(game_content->nodes + i, &e);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }
  InputContent main_g;
  main_g.filename = argv[1];
  read_file(&main_g);
  /* TODO: everything */
  return 0;
}
