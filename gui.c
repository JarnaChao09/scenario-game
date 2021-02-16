#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>

#include "game.h"

enum app_state {APP_NO_PAUSE, APP_PRESS_ANY_KEY, APP_PRESS_CHOICE_KEY, APP_TEXT_INPUT};
/* Main struct with all the important information
 * for the game to run, has single global instance.
 */
struct app{
  /* important rendering data. */
  SDL_Renderer *renderer;
  SDL_Window *window;
  int WinHeight;
  int WinWidth;

  /* game state data. */
  GameState *gs;
  enum app_state state;
  SDL_Texture *bgImage;
  int textures_len;
  SDL_Texture **textures;
  SDL_Rect **textures_loc;
  int redraw_image;
  char *TextBufferStart;
  char *TextBufferEnd;
  char *cursor;
  char processedText[1024];

  /* graphics data for drawings. */
  SDL_Rect TextArea;
  SDL_Rect SaveButtonArea;
  SDL_Rect ExitButtonArea;
  TTF_Font *font;
  SDL_Color TextColor;
};

struct app MainApp = {
    .WinHeight = 400,
    .WinWidth = 600,
    .TextColor = {255, 255, 255},
    .TextArea = {20, 300, 560, 80},
    .bgImage = NULL,
    .TextBufferStart = NULL,
    .TextBufferEnd = NULL,
    .processedText = '\0',
    };


void init_everything(char *gamefile){
  /* Load the game from file */
  /* Putting it later on this function won't work... */
  MainApp.gs = read_file(gamefile);
  MainApp.state = APP_NO_PAUSE;
  
  /* init the graphics window */
  int rendererFlags, windowFlags;
  rendererFlags = SDL_RENDERER_ACCELERATED;
  windowFlags = 0;
  SDL_Init(SDL_INIT_VIDEO);
  /* MainApp.WinHeight = 400; */
  /* MainApp.WinWidth = 600; */
  MainApp.window = SDL_CreateWindow("SIMULATION", // creates a window
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, MainApp.WinWidth, MainApp.WinHeight, windowFlags);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  MainApp.renderer = SDL_CreateRenderer(MainApp.window, -1, rendererFlags);

  /* First screen: Loading screen */
  TTF_Init();
  MainApp.font = TTF_OpenFont("/usr/share/fonts/TTF/siddhanta.ttf", 24);
  SDL_Surface *surf;
  surf = TTF_RenderText_Solid(MainApp.font, "Loading...", MainApp.TextColor); 
  SDL_Texture * texture = SDL_CreateTextureFromSurface(MainApp.renderer, surf);
  int texW = 0;
  int texH = 0;
  SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
  SDL_Rect dstrect = {
    MainApp.WinWidth/2 - texW/2,
    MainApp.WinHeight/2 - texH/2,
    texW,
    texH
  };
  SDL_RenderCopy(MainApp.renderer, texture, NULL, &dstrect);
  SDL_SetRenderDrawColor(MainApp.renderer, 0, 255, 0, 100);
  /* SDL_RenderDrawRect(MainApp.renderer, &MainApp.TextArea); */
  SDL_RenderPresent(MainApp.renderer);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surf);
  MainApp.textures_len = 0;
  MainApp.textures = NULL;
  MainApp.textures_loc = NULL;

  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
}

void destroy_everything() {
  IMG_Quit();
  free(MainApp.gs->all_events);
  free(MainApp.gs);
  TTF_CloseFont(MainApp.font);
  TTF_Quit();
  SDL_DestroyRenderer(MainApp.renderer);
  SDL_DestroyWindow(MainApp.window);
}

void change_background(char *image_file){
  SDL_Surface * surf = SDL_LoadBMP(image_file);
  if (surf==NULL) printf("Err IMG: %s\n", IMG_GetError());
  SDL_DestroyTexture(MainApp.bgImage);
  MainApp.bgImage = SDL_CreateTextureFromSurface(MainApp.renderer, surf);
  SDL_RenderCopy(MainApp.renderer, MainApp.bgImage, NULL, NULL);
  SDL_FreeSurface(surf);
}

void extract_text(char *start, char *end, char *destination) {
  if (end == NULL || start > end){
    return;
  }
  int i = 0;
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
      *(destination + i++) = ' ';
      break;
    default:
      *(destination + i++) = *start;
      whitespace_flag = 0;
    }
    start++;
  }
  *(destination + i++) = '\0';
}

void insert_text(char *text, SDL_Rect *location){
  SDL_Surface *surf;
  surf = TTF_RenderText_Solid(MainApp.font, MainApp.processedText, MainApp.TextColor); 
  SDL_Texture * texture = SDL_CreateTextureFromSurface(MainApp.renderer, surf);
  int texW = 0;
  int texH = 0;
  SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
  SDL_Rect *dstrect = malloc(sizeof(SDL_Rect));
  dstrect->x = location->x;
  dstrect->y = location->y;
  dstrect->w = texW;
  dstrect->h = texH;
  MainApp.textures = realloc(MainApp.textures, sizeof(SDL_Texture *) * MainApp.textures_len+1);
  MainApp.textures_loc = realloc(MainApp.textures_loc, sizeof(SDL_Rect *) * MainApp.textures_len+1);
  *(MainApp.textures + MainApp.textures_len) = texture;
  *(MainApp.textures_loc + MainApp.textures_len) = dstrect;
  MainApp.textures_len += 1;
  SDL_FreeSurface(surf);
}

void clear_texts(){
  int i;
  for (i=0; i<MainApp.textures_len; i++) {
    SDL_DestroyTexture(*(MainApp.textures + i));
    free(*(MainApp.textures_loc)+i);
  }
  MainApp.textures_len = 0;
  MainApp.textures = NULL;
  MainApp.textures_loc = NULL;
}

void draw_app(){
  SDL_SetRenderTarget(MainApp.renderer, NULL);
  SDL_RenderCopy(MainApp.renderer, MainApp.bgImage, NULL, NULL);
  SDL_SetRenderDrawColor(MainApp.renderer, 0, 255, 0, 100);
  SDL_RenderDrawRect(MainApp.renderer, &MainApp.TextArea);
  int i;
  for (i=0; i<MainApp.textures_len; i++) {
    SDL_RenderCopy(MainApp.renderer, *(MainApp.textures + i), NULL, *(MainApp.textures_loc)+i);
  }
  SDL_RenderPresent(MainApp.renderer);
}


void execute_action(Action *a){
  char *c;
  int i;
  clear_texts();
  switch (a->type){
  case SCENE_ACTION_DELAY:
    SDL_Delay(_atoi(a->text_start, a->text_end));
    break;
  case SCENE_ACTION_IMAGE:
    for(c=a->text_start, i=0; c<=a->text_end; c++, i++){
      MainApp.processedText[i] = *c;
    }
    MainApp.processedText[i] = '\0';
    change_background(MainApp.processedText);
    if (DEBUG) printf("image:%s\n", MainApp.processedText);
    break;
  case SCENE_ACTION_AUDIO:
  case SCENE_ACTION_TEXT:
    extract_text(a->text_start, a->text_end, MainApp.processedText);
    insert_text(MainApp.processedText, &MainApp.TextArea);
    if (DEBUG) print_between(a->text_start, a->text_end);
    break;
  default:
    printf("Unsupported action.");
  }
  draw_app();
}

int execute_choice(Choice *c, int len){
  if (len == 1) {
    /* MainApp.state = APP_PRESS_ANY_KEY; */
    /* SDL_Rect center = {MainApp.WinWidth/2, MainApp.WinHeight} */
    return 0;
  }
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


int execute_game(GameState *gs){
  int i, next;
  next = execute_event(gs->current_event);
  for (i=0; i < gs->events_count; i++){
    if ((gs->all_events + i)->node == next){
      gs->current_event = gs->all_events + i;
      return 1;
    }
  }
  draw_app();
  printf("Node not found: %d", next);
  return 0;
}


int main(int argc, char *argv[])
{
  if (argc<2) exit(1);
  init_everything(argv[1]);
  if (DEBUG) printf("Starting Game...\n"
		    "Nodes: %d\n", MainApp.gs->events_count);
  while(execute_game(MainApp.gs));
  destroy_everything();
  return 0;
}
