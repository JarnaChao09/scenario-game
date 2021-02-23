CC = gcc
CFLAGS = -Wall -g
LIBS = -lSDL2 -lSDL2_ttf -lSDL2_image
OBJ = game.o terminal.o gui.o

all: terminal gui

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) -c -o $@ $<

terminal: terminal.o game.o game.h
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

gui: gui.o game.o game.h
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	-rm *.o terminal gui

