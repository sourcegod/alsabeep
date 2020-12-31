CC=gcc
CFLAGS=-lm -lasound
EXEC=alsabeep

all:
	$(CC) src/alsabeep.c -o $(EXEC) $(CFLAGS)
