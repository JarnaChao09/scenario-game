#include <stdio.h>
#include <stdlib.h>
#include "game.h"


void execute_action(Action *a);

int execute_choice(Choice *c, int len);

int execute_event(Event *e);

int execute_game(GameState *gs);

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
  int i, /* j, */ choice;
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
