.PHONY: all clean

all: transmitter receiver

receiver: receiver.c
	gcc receiver.c -lpthread -o receiver

transmitter: transmitter.c
	gcc transmitter.c -lpthread -o transmitter

clean:
	rm -rf *.out transmitter receiver
