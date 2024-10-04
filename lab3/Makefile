.PHONY: all clean run 

all: fork clean

fork: fork.c
	gcc fork.c -o fork

run: fork
	./fork

clean:
	rm -rf *.o fork.exe
