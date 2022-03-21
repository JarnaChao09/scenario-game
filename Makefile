CC = gcc
CFLAGS = -Wall -g
LIBS = -lSDL2 -lSDL2_ttf -lSDL2_image -lm
OBJ = lib/game.o lib/terminal.o lib/gui.o lib/calc.o

all: terminal gui

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -c -o $@ $<

terminal: lib/terminal.o lib/game.o game.h
	$(CC) $(CFLAGS) $(LIBS) -o bin/$@ $^

gui: lib/gui.o lib/game.o game.h
	$(CC) $(CFLAGS) $(LIBS) -o bin/$@ $^

calc: lib/calc.o
	$(CC) $(CFLAGS) $(LIBS) -o bin/$@ $^	

clean:
	-rm lib/*.o
	-rm bin/*

