.PHONY: all clean

all: threads

threads: readWriteThreads.c
	gcc readWriteThreads.c -lpthread -o threads

clean:
	rm -rf threads
