CC = gcc
CFLAGS = -Wall -g
LIBS = -lSDL2 -lSDL2_ttf -lSDL2_image -lm
OBJ = game.o terminal.o gui.o calc.o

all: terminal gui

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -c -o $@ $<

terminal: terminal.o game.o game.h
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

gui: gui.o game.o game.h
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

calc: calc.o
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^	

clean:
	-rm *.o terminal gui

