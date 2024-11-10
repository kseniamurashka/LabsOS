.PHONY: all clean

all: archiver

archiver: main.o archiver.o
	gcc main.o archiver.o -o archiver
main.o: main.c
	gcc -c main.c
archiver.o: archiver.c
	gcc -c archiver.c
clean:
	rm -rf *.o archiver
