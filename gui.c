#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "game.h"

#define TEXT_OFFSET 7

enum app_state {APP_NO_PAUSE, APP_EXIT, APP_PRESS_ANY_KEY, APP_PRESS_CHOICE_KEY, APP_SINGLE_CHOICE, APP_TEXT_INPUT};
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
  SDL_Texture *mainTextBG;
  SDL_Texture *mainText;
  SDL_Rect *MainText_loc;
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
  TTF_Font *fontSmall;
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

void change_background(char *image_file);
void insert_text(char *text, SDL_Rect *location, int centered, int textBox);
void draw_app();

void init_everything(char *gamefile){
  /* Load the game from file */
  /* Putting it later on this function won't work... */
  MainApp.gs = read_file(gamefile);
  MainApp.state = APP_NO_PAUSE;
  
  /* init the graphics window */
  int rendererFlags, windowFlags;
  rendererFlags = SDL_RENDERER_ACCELERATED;
  windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;
  SDL_Init(SDL_INIT_VIDEO);
  /* MainApp.WinHeight = 400; */
  /* MainApp.WinWidth = 600; */
  MainApp.window = SDL_CreateWindow("GAME", // creates a window
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, MainApp.WinWidth, MainApp.WinHeight, windowFlags);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  MainApp.renderer = SDL_CreateRenderer(MainApp.window, -1, rendererFlags);

  /* First screen: Loading screen */
  TTF_Init();
  MainApp.font = TTF_OpenFont("/usr/share/fonts/TTF/siddhanta.ttf", 24);
  MainApp.fontSmall = TTF_OpenFont("/usr/share/fonts/TTF/siddhanta.ttf", 14);
  change_background("assets/background.bmp");
  SDL_Surface * surf = SDL_LoadBMP("assets/textbox.bmp");
  MainApp.mainTextBG = SDL_CreateTextureFromSurface(MainApp.renderer, surf);
  SDL_FreeSurface(surf);
  SDL_DisplayMode dm;
  SDL_GetDesktopDisplayMode(0, &dm);
  MainApp.WinHeight = dm.h;
  MainApp.WinWidth = dm.w;
  SDL_Rect dstrect = {
    MainApp.WinWidth/2,
    MainApp.WinHeight/2
  };
  insert_text("Loading....", &dstrect, 1, 0);
  draw_app();
  MainApp.TextArea.x = dm.w / 20;
  MainApp.TextArea.y = dm.h * 8 / 10;
  MainApp.TextArea.w = dm.w * 9 / 10;
  MainApp.TextArea.h = dm.h / 10;
  MainApp.textures_len = 0;
  MainApp.textures = NULL;
  MainApp.textures_loc = NULL;
  MainApp.mainText = NULL;
  MainApp.MainText_loc = NULL;

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
  int escape_flag = 0;
  while (start <= end) {
    switch (*start){
    case '\\':
      if (escape_flag){
	break;
      }
      escape_flag = 1;
      start++;
      continue;
    case '\n':
    case ' ':
    case '\t':
      if (whitespace_flag || i==0) break;
      whitespace_flag = 1;
      *(destination + i++) = ' ';
      break;
    case 'n':
      if (escape_flag){
        *(destination + i++) = '\n';
        whitespace_flag = 1;
	start++;
	continue;
      }
    default:
      *(destination + i++) = *start;
      escape_flag = 0;
      whitespace_flag = 0;
    }
    start++;
  }
  if (whitespace_flag && i==1) *destination = '\0';
  else *(destination + i++) = '\0';
}

void insert_text(char *text, SDL_Rect *location, int centered, int textBox){
  SDL_Surface *surf;
  if (*text == '\0') {
    surf = TTF_RenderText_Blended_Wrapped(
        textBox ? MainApp.fontSmall : MainApp.font, "Continue",
        MainApp.TextColor, location->w);
  } else {
    surf = TTF_RenderText_Blended_Wrapped(textBox ? MainApp.fontSmall : MainApp.font, text, MainApp.TextColor, location->w);
  }
  SDL_Texture * texture = SDL_CreateTextureFromSurface(MainApp.renderer, surf);
  int texW = 0;
  int texH = 0;
  SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
  SDL_Rect *dstrect = malloc(sizeof(SDL_Rect));
  dstrect->x = location->x - centered * texW / 2;
  dstrect->w = texW;
  dstrect->h = texH;
  dstrect->y = location->y - centered * texH / 2;
  if (textBox) {
    MainApp.TextArea.y = MainApp.TextArea.y + MainApp.TextArea.h - texH;
    MainApp.TextArea.h = texH;
    dstrect->y = MainApp.TextArea.y;
    free(MainApp.MainText_loc);
    SDL_DestroyTexture(MainApp.mainText);
    MainApp.mainText = texture;
    MainApp.MainText_loc = dstrect;
    return;
  }
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
    free(*(MainApp.textures_loc+i));
  }
  MainApp.textures_len = 0;
  MainApp.textures = NULL;
  MainApp.textures_loc = NULL;
}

void draw_app(){
  SDL_Rect offset;
  SDL_SetRenderTarget(MainApp.renderer, NULL);
  SDL_RenderCopy(MainApp.renderer, MainApp.bgImage, NULL, NULL);
  offset = MainApp.TextArea;
  offset.x -= TEXT_OFFSET;
  offset.y -= TEXT_OFFSET;
  offset.h += 2*TEXT_OFFSET;
  offset.w += 2*TEXT_OFFSET;
  SDL_RenderCopy(MainApp.renderer, MainApp.mainTextBG, NULL, &offset);
  SDL_RenderCopy(MainApp.renderer, MainApp.mainText, NULL, MainApp.MainText_loc);
  int i;
  for (i=0; i<MainApp.textures_len; i++) {
      SDL_RenderCopy(MainApp.renderer, *(MainApp.textures + i), NULL, *(MainApp.textures_loc+i));
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
    insert_text(MainApp.processedText, &MainApp.TextArea, 0, 1);
    if (DEBUG) print_between(a->text_start, a->text_end);
    break;
  default:
    printf("Unsupported action.");
  }
}

int execute_choice(Choice *c, int len){
  char choice_text[256];
  SDL_Rect loc = {
    .x = MainApp.WinWidth/2,
    .y = MainApp.WinHeight/2,
    .w = MainApp.WinWidth,
    .h = MainApp.WinHeight
  };
  if (len == 1) {
    MainApp.state = APP_SINGLE_CHOICE;
    extract_text(c->text_start, c->text_end, choice_text);
    if (*choice_text == '\0'){
      loc.x = MainApp.TextArea.x + MainApp.TextArea.w - 100;
      loc.y = MainApp.TextArea.y + MainApp.TextArea.h + 10;
      insert_text("Continue", &loc, 0, 0);
      return 0;
    }
    insert_text(choice_text, &loc, 1, 0);
    return 0;
  }
  MainApp.state = APP_PRESS_CHOICE_KEY;
  int i;
  for (i = 0; i < len; i++) {
    loc.y = MainApp.WinHeight*(1+i*2)/10;
    loc.h = MainApp.WinHeight/5;
    sprintf(choice_text, "%c) ", i+'A');
    extract_text((c+i)->text_start, (c+i)->text_end, choice_text+3);
    insert_text(choice_text, &loc, 1, 0);
    if (DEBUG){
      printf("\t %2d) ", i + 1);
      print_between((c + i)->text_start, (c + i)->text_end);
      printf("\n");
    }
  }
  return 0;
}

void execute_event(Event *e){
  int i;
  for (i=0; i < e->actions_count; i++){
    execute_action(e->actions+i);
    draw_app();
  }
  execute_choice(e->choices, e->choices_count);
  draw_app();
}


int handle_input() {
  SDL_Event event;
  int i, next, choice;
  while(SDL_PollEvent(&event)){
    switch (event.type) {
    case SDL_QUIT:
      exit(0);
    case SDL_KEYUP:
      if (MainApp.state == APP_PRESS_ANY_KEY){
	MainApp.state = APP_NO_PAUSE;
	return 0;
      }
      if (MainApp.state == APP_SINGLE_CHOICE){
	choice = 0;
      }else if (MainApp.state == APP_PRESS_CHOICE_KEY){
	switch (event.key.keysym.sym){
	case '0' ... '9':
	  choice = event.key.keysym.sym - '0';
	  break;
	case 'A' ... 'Z':
	  choice = event.key.keysym.sym - 'A';
	  break;
	case 'a' ... 'z':
	  choice = event.key.keysym.sym - 'a';
	  break;
	default:
	  choice = -1;
	}
      }
      if (choice > -1 && choice <= MainApp.gs->current_event->choices_count) {
        next = (MainApp.gs->current_event->choices + choice)->target;
        if (next == 0) {
          MainApp.state = APP_EXIT;
          return 0;
        }
        for (i = 0; i < MainApp.gs->events_count; i++) {
          if ((MainApp.gs->all_events + i)->node == next) {
            MainApp.gs->current_event = MainApp.gs->all_events + i;
            MainApp.state = APP_NO_PAUSE;
            return 0;
          }
        }
      }
    default:
      break;
    }
  }
  return 1;
}

int execute_game() {
  do {
    execute_event(MainApp.gs->current_event);
    int loop_flag = 1;
    while (loop_flag) {
      draw_app();
      loop_flag = handle_input();
    }
  } while (MainApp.state != APP_EXIT);
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc<2) exit(1);
  init_everything(argv[1]);
  if (DEBUG) printf("Starting Game...\n"
		    "Nodes: %d\n", MainApp.gs->events_count);
  execute_game();
  destroy_everything();
  return 0;
}
