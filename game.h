#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#ifndef _GAME_DEF_H_
#define _GAME_DEF_H_ 1

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

/* Read number and move the cursor to the end. */
int read_number(char *buffer, int *num);

/* read the file and prepare a game state. */
GameState *read_file(char *filename);

/* Read node from the buffer and move the cursor. */
int read_node(char *buffer, int *point, int buff_len, Node *n);

/* Convert the node to event. */
void node_to_event(Node *n, Event *e);

/* Print characters between the start and the end pointer. */
void print_between(char *start, char *end);

/* Print the node to the screen. */
void print_node(Node *n);

void print_choice(Choice *c);

/* Print the action to the screen. */
void print_action(Action *a);

/* Read the action from the buffer and move the cursor. */
int read_action(char *buffer_start, int *point, char *buffer_end, Action *a);

/* Read the choices from the file and move the cursor. */
int read_choice(char *buffer_start, int *point, char *buffer_end, Choice *c);

/* Implementation of atoi but with fixed start and end. */
int _atoi(char *start, char *end);

/* Exits the game gracefully. */
void exit_gracefully(int exit_code);


#endif
