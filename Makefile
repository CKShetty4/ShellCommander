CC=g++
CFLAGS=-c -Wall
LFLAGS=-Wall

all: shell_commander

shell_commander: src/main.cpp include/shell_commander.h
	$(CC) $(CFLAGS) src/main.cpp -o src/main.o
	$(CC) $(LFLAGS) src/main.o -o shell_commander

clean:
		rm -f src/main.o shell_commander