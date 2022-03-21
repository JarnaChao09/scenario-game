CC = g++-11
CFLAGS = -Wall -g 
# -fpermissive
LIBS = -lm
# -lSDL2_ttf -lSDL2_image -lm
INCLUDES = -I /opt/hombrew/Cellar/sdl2/2.0.20/include 
# -I /opt/hombrew/Cellar/sdl2_image/2.0.5/include -I /opt/hombrew/Cellar/sdl2_ttf/2.0.18_1/include
LLIBS = -L /opt/homebrew/Cellar/sdl2/2.0.20/lib 
# -L /opt/hombrew/Cellar/sdl2_image/2.0.5/lib -L /opt/hombrew/Cellar/sdl2_ttf/2.0.18_1/lib
BINARIES = -l SDL2
# -l SDL2_image -l SDL2_ttf
OBJ = game.o terminal.o gui.o calc.o

all: terminal gui

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(LLIBS) $(BINARIES) -c -o $@ $<

terminal: terminal.o game.o game.h
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(LLIBS) $(BINARIES) -o $@ $^

gui: gui.o game.o game.h
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(LLIBS) $(BINARIES) -o $@ $^

calc: calc.o
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(LLIBS) $(BINARIES) -o $@ $^	

clean:
	-rm *.o terminal gui

